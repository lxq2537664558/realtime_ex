#include "db_thread.h"
#include "db_thread_mgr.h"
#include "db_command_handler.h"
#include "db_protobuf.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

#include "proto_src/flush_command.pb.h"
#include "proto_src/select_command.pb.h"
#include "proto_src/delete_command.pb.h"

#include "libBaseCommon/time_util.h"

namespace base
{

	CDbThread::CDbThread()
		: m_pDbThreadMgr(nullptr)
		, m_quit(0)
		, m_nQPS(0)
	{
	}

	CDbThread::~CDbThread()
	{
	}

	bool CDbThread::connectDb(bool bInit)
	{
		do
		{
			if (!this->m_dbConnection.connect(
				this->m_pDbThreadMgr->getDbConnectionInfo().szHost,
				this->m_pDbThreadMgr->getDbConnectionInfo().nPort,
				this->m_pDbThreadMgr->getDbConnectionInfo().szUser,
				this->m_pDbThreadMgr->getDbConnectionInfo().szPassword,
				this->m_pDbThreadMgr->getDbConnectionInfo().szDb,
				this->m_pDbThreadMgr->getDbConnectionInfo().szCharacterset))
			{
				if (bInit)
					return false;

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			break;
		} while (1);

		this->m_dbCommandHandlerProxy.onConnect(&this->m_dbConnection);
		return true;
	}

	void CDbThread::join()
	{
		this->m_quit = 1;
		this->m_thread.join();
	}

	bool CDbThread::init(CDbThreadMgr* pDbThreadMgr, uint64_t nMaxCacheSize, uint32_t nWritebackTime)
	{
		DebugAstEx(pDbThreadMgr != nullptr, false);
		this->m_pDbThreadMgr = pDbThreadMgr;
		if (!this->m_dbCommandHandlerProxy.init())
			return false;

		if (!this->m_dbCacheMgr.init(this, nMaxCacheSize, nWritebackTime))
			return false;

		if (!this->connectDb(true))
			return false;

		this->m_thread = std::thread([this]()
		{
			while (true)
			{
				this->onProcess();
				if (this->m_quit != 0)
				{
					std::unique_lock<std::mutex> lock(this->m_tCommandLock);
					if (this->m_listCommand.empty())
						break;
				}
			}

			this->flushCache(0, true);

			this->m_dbConnection.close();
			this->m_dbCommandHandlerProxy.onDisconnect();
		});

		return true;
	}

	void base::CDbThread::onDestroy()
	{

	}

	void CDbThread::onProcess()
	{
		if (!this->m_dbConnection.isConnect() || !this->m_dbConnection.ping())
		{
			this->m_dbConnection.close();
			this->m_dbCommandHandlerProxy.onDisconnect();
			this->connectDb(false);
		}

		this->m_dbCacheMgr.update();

		std::list<SDbCommand> listCommand;
		{
			std::unique_lock<std::mutex> lock(this->m_tCommandLock);
			while (this->m_listCommand.empty())
			{
				if (std::cv_status::timeout == this->m_condition.wait_for(lock, std::chrono::seconds(1)))
					return;
			}

			listCommand.splice(listCommand.end(), this->m_listCommand);
		}

		for (auto iter = listCommand.begin(); iter != listCommand.end(); ++iter)
		{
			SDbCommand sDbCommand = *iter;
			google::protobuf::Message* pRequestMessage = sDbCommand.pMessage;
			defer([&]()
			{
				SAFE_DELETE(pRequestMessage);
			});

			if (sDbCommand.nType == db::eDBCT_Flush)
			{
				proto::db::flush_command* pFlushCommand = dynamic_cast<proto::db::flush_command*>(pRequestMessage);
				if (pFlushCommand == nullptr)
					continue;

				this->flushCache(pFlushCommand->id(), pFlushCommand->type() == db::eFCT_Del);
				continue;
			}
			
			std::shared_ptr<google::protobuf::Message> pResponseMessage;
			uint32_t nErrorCode = db::eDBRC_OK;
			if (!this->onPreCache(sDbCommand.nType, pRequestMessage, pResponseMessage))
			{
				nErrorCode = this->m_dbCommandHandlerProxy.onDbCommand(sDbCommand.nType, pRequestMessage, &pResponseMessage);
				if (nErrorCode == db::eDBRC_LostConnection)
				{
					std::unique_lock<std::mutex> lock(this->m_tCommandLock);
					this->m_listCommand.splice(this->m_listCommand.begin(), listCommand, iter, listCommand.end());
					break;
				}
			}
			this->onPostCache(sDbCommand.nType, pRequestMessage, pResponseMessage);

			if (sDbCommand.callback == nullptr)
				continue;

			SDbResultInfo sDbResultInfo;
			
			sDbResultInfo.pMessage = pResponseMessage;
			sDbResultInfo.nErrorCode = nErrorCode;
			sDbResultInfo.callback = sDbCommand.callback;

			this->m_pDbThreadMgr->addResultInfo(sDbResultInfo);
		}
	}

	bool CDbThread::onPreCache(uint32_t nType, const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>& pResponseMessage)
	{
		if (this->m_dbCacheMgr.getMaxCacheSize() <= 0)
			return false;

		switch (nType)
		{
		case db::eDBCT_Select:
			{
				const proto::db::select_command* pCommand = dynamic_cast<const proto::db::select_command*>(pRequestMessage);
				DebugAstEx(pCommand != nullptr, false);

				std::string szDataName = getMessageNameByTableName(pCommand->table_name());
				google::protobuf::Message* pMessage = this->m_dbCacheMgr.getData(pCommand->id(), szDataName);
				if (pMessage != nullptr)
				{
					pResponseMessage = std::shared_ptr<google::protobuf::Message>(pMessage);
					return true;
				}
			}
			break;

		case db::eDBCT_Update:
			{
				uint64_t nID = 0;
				if (!getPrimaryValue(pRequestMessage, nID))
					return false;

				if (this->m_dbCacheMgr.setData(nID, pRequestMessage))
					return true;
			}
			break;

		case db::eDBCT_Insert:
			{
				uint64_t nID = 0;
				if (!getPrimaryValue(pRequestMessage, nID))
					return false;

				this->m_dbCacheMgr.addData(nID, pRequestMessage);
			}
			break;

		case db::eDBCT_Delete:
			{
				const proto::db::delete_command* pCommand = dynamic_cast<const proto::db::delete_command*>(pRequestMessage);
				DebugAstEx(pCommand != nullptr, false);

				this->m_dbCacheMgr.delData(pCommand->id(), getMessageNameByTableName(pCommand->table_name()));
			}
			break;
		}

		return false;
	}

	void CDbThread::onPostCache(uint32_t nType, const google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>& pResponseMessage)
	{
		if (this->m_dbCacheMgr.getMaxCacheSize() <= 0)
			return;

		if (nType == db::eDBCT_Select)
		{
			DebugAst(pResponseMessage != nullptr);

			const proto::db::select_command* pCommand = dynamic_cast<const proto::db::select_command*>(pRequestMessage);
			DebugAst(pCommand != nullptr);

			this->m_dbCacheMgr.setData(pCommand->id(), pResponseMessage.get());
		}
	}

	void CDbThread::query(const SDbCommand& sDbCommand)
	{
		std::unique_lock<std::mutex> lock(this->m_tCommandLock);
		this->m_listCommand.push_back(sDbCommand);

		this->m_condition.notify_one();
	}

	uint32_t CDbThread::getQueueSize()
	{
		std::unique_lock<std::mutex> lock(this->m_tCommandLock);
		return (uint32_t)this->m_listCommand.size();
	}

	uint32_t CDbThread::getQPS()
	{
		return this->m_nQPS;
	}

	CDbCommandHandlerProxy& CDbThread::getDbCommandHandlerProxy()
	{
		return this->m_dbCommandHandlerProxy;
	}

	void CDbThread::setMaxCacheSize(uint64_t nSize)
	{
		this->m_dbCacheMgr.setMaxCacheSize(nSize);
	}

	void CDbThread::flushCache(uint64_t nKey, bool bDel)
	{
		this->m_dbCacheMgr.flushCache(nKey, bDel);
	}
}