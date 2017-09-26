#include "db_thread_mgr.h"
#include "libBaseCommon/debug_helper.h"

namespace base
{
	CDbThreadMgr::CDbThreadMgr()
	{
	}

	CDbThreadMgr::~CDbThreadMgr()
	{
		this->exit();
	}

	bool CDbThreadMgr::init(const std::string& szHost, uint16_t nPort, const std::string& szDb, const std::string& szUser, const std::string& szPassword, const std::string& szCharacterset, uint32_t nDbThreadCount, const db::SCacheConfigInfo& sCacheConfigInfo)
	{
		DebugAstEx(nDbThreadCount > 0, false);

		this->m_sDbConnectionInfo.szHost = szHost;
		this->m_sDbConnectionInfo.nPort = nPort;
		this->m_sDbConnectionInfo.szDb = szDb;
		this->m_sDbConnectionInfo.szUser = szUser;
		this->m_sDbConnectionInfo.szPassword = szPassword;
		this->m_sDbConnectionInfo.szCharacterset = szCharacterset;

		this->m_vecDbThread.resize(nDbThreadCount);
		for (uint32_t i = 0; i < nDbThreadCount; ++i)
		{
			this->m_vecDbThread[i] = new CDbThread();
			if (!this->m_vecDbThread[i]->init(this, sCacheConfigInfo))
				return false;
		}

		return true;
	}

	void CDbThreadMgr::exit()
	{
		for (uint32_t i = 0; i < this->m_vecDbThread.size(); ++i)
		{
			if (this->m_vecDbThread[i] == nullptr)
				continue;

			this->m_vecDbThread[i]->quit();
			this->m_vecDbThread[i]->join();
		}

		// 再执行一次，确保残留的db操作完成
		for (uint32_t i = 0; i < this->m_vecDbThread.size(); ++i)
		{
			if (this->m_vecDbThread[i] == nullptr)
				continue;

			if (!this->m_vecDbThread[i]->isConnectDb())
				continue;

			this->m_vecDbThread[i]->onProcess();
			SAFE_DELETE(this->m_vecDbThread[i]);
		}

		this->m_vecDbThread.clear();
	}

	void CDbThreadMgr::query(uint32_t nThreadIndex, const SDbCommand& sDbCommand)
	{
		DebugAst(!this->m_vecDbThread.empty());

		if (sDbCommand.nType != db::eDBCT_Flush)
		{
			nThreadIndex = nThreadIndex % this->m_vecDbThread.size();

			this->m_vecDbThread[nThreadIndex]->query(sDbCommand);
		}
		else
		{
			if (nThreadIndex == 0)
			{
				for (size_t i = 0; i < this->m_vecDbThread.size(); ++i)
				{
					this->m_vecDbThread[i]->query(sDbCommand);
				}
			}
			else
			{
				nThreadIndex = nThreadIndex % this->m_vecDbThread.size();

				this->m_vecDbThread[nThreadIndex]->query(sDbCommand);
			}
		}
	}

	uint32_t CDbThreadMgr::getThreadCount() const
	{
		return (uint32_t)this->m_vecDbThread.size();
	}

	const SDbConnectionInfo& CDbThreadMgr::getDbConnectionInfo() const
	{
		return this->m_sDbConnectionInfo;
	}

	void CDbThreadMgr::addResultInfo(const SDbResultInfo& sResultInfo)
	{
		std::unique_lock<std::mutex> lock(this->m_tResultLock);
		this->m_listResultInfo.push_back(sResultInfo);
	}

	void CDbThreadMgr::update()
	{
		std::unique_lock<std::mutex> lock(this->m_tResultLock);
		if (this->m_listResultInfo.empty())
			return;

		std::list<SDbResultInfo> listResultInfo;
		listResultInfo.splice(listResultInfo.end(), this->m_listResultInfo);
		for (auto iter = listResultInfo.begin(); iter != listResultInfo.end(); ++iter)
		{
			SDbResultInfo& sDbResultInfo = *iter;
			if (sDbResultInfo.callback != nullptr)
				sDbResultInfo.callback(sDbResultInfo.pMessage.get(), sDbResultInfo.nErrorCode);
		}
	}

	uint32_t CDbThreadMgr::getQPS(uint32_t nThreadIndex)
	{
		if (nThreadIndex >= this->m_vecDbThread.size())
			return 0;

		return this->m_vecDbThread[nThreadIndex]->getQPS();
	}

	uint32_t CDbThreadMgr::getQueueSize(uint32_t nThreadIndex)
	{
		if (nThreadIndex >= this->m_vecDbThread.size())
			return 0;

		return this->m_vecDbThread[nThreadIndex]->getQueueSize();
	}

	void CDbThreadMgr::setMaxCacheSize(uint64_t nSize)
	{
		nSize = this->m_vecDbThread.size() / nSize;

		for (size_t i = 0; i < this->m_vecDbThread.size(); ++i)
		{
			this->m_vecDbThread[i]->setMaxCacheSize(nSize);
		}
	}
}