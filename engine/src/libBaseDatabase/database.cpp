#include "database.h"
#include "db_protobuf.h"
#include "db_thread_mgr.h"

#include <vector>
#include <string>
#include <map>

#ifdef _WIN32
#include <direct.h>
#else
#include <fcntl.h>
#include <sys/resource.h>
#endif

#ifdef _WIN32
#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "proto_src/flush_command.pb.h"
#include "proto_src/select_command.pb.h"
#include "proto_src/delete_command.pb.h"
#include "proto_src/query_command.pb.h"
#include "proto_src/call_command.pb.h"
#include "proto_src/nop_command.pb.h"
#include "proto_src/update_command.pb.h"
#include "proto_src/insert_command.pb.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/base_common.h"
#include "libBaseCommon/rw_lock.h"
#include "libBaseCommon/defer.h"

namespace
{
	std::map<uint32_t, base::CDbThreadMgr*>	s_mapDbThreadMgr;
	base::CRWLock s_lock;

	const std::string szDbOption = "db_option.proto";

	base::CDbThreadMgr* getDbThreadMgr(uint32_t nID)
	{
		s_lock.lock(base::CRWLock::eLT_Read);
		defer([&]()
		{
			s_lock.unlock(base::CRWLock::eLT_Read);
		});

		auto iter = s_mapDbThreadMgr.find(nID);
		if (iter == s_mapDbThreadMgr.end())
			return nullptr;

		return iter->second;
	}
}

namespace base
{
	namespace db
	{
		uint32_t create(const std::string& szHost, uint16_t nPort, const std::string& szDb, const std::string& szUser, const std::string& szPassword, const std::string& szCharset, const SDbOptions& sDbOptions)
		{
			std::vector<std::string> vecProto;

#ifdef _WIN32
			WIN32_FIND_DATAA FindFileData;

			std::string szProtoFile = sDbOptions.szProtoDir;
			szProtoFile = base::string_util::trim(szProtoFile);
			if (!szProtoFile.empty() && (szProtoFile[szProtoFile.size() - 1] == '/' || szProtoFile[szProtoFile.size() - 1] == '\\'))
				szProtoFile.resize(szProtoFile.size() - 1);
			szProtoFile += "/*.proto";
			
			HANDLE hFind = ::FindFirstFileA(szProtoFile.c_str(), &FindFileData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					std::string szName = FindFileData.cFileName;
					vecProto.push_back(szName);
				} while (::FindNextFileA(hFind, &FindFileData) != 0);
				::FindClose(hFind);
			}
#else
			DIR* pDir = nullptr;
			if ((pDir = opendir(sDbOptions.szProtoDir.c_str())) == nullptr)
				return 0;

			struct dirent* pFile = nullptr;
			while ((pFile = readdir(pDir)) != nullptr)
			{
				if (pFile->d_type != DT_REG)
					continue;

				std::string szName = pFile->d_name;
				size_t pos = szName.rfind(".proto");
				if (pos == std::string::npos || pos + base::function_util::strnlen(".proto", _TRUNCATE) != szName.size())
					continue;

				vecProto.push_back(szName);
			}
			closedir(pDir);
#endif
			
			CDbThreadMgr* pDbThreadMgr = new CDbThreadMgr();
			if (!pDbThreadMgr->init(szHost, nPort, szDb, szUser, szPassword, szCharset, sDbOptions))
			{
				SAFE_DELETE(pDbThreadMgr);
				return 0;
			}

			s_lock.lock(CRWLock::eLT_Write);
			defer([&]()
			{
				s_lock.unlock(CRWLock::eLT_Write);
			});

			// 初始化proto需要保护
			if (!importProtobuf(sDbOptions.szProtoDir, vecProto))
			{
				SAFE_DELETE(pDbThreadMgr);
				return 0;
			}

			static uint32_t nID = 1;
			s_mapDbThreadMgr[nID] = pDbThreadMgr;

			return nID++;
		}

		void release(uint32_t nID)
		{
			CDbThreadMgr* pDbThreadMgr = nullptr;
			{
				s_lock.lock(CRWLock::eLT_Write);
				defer([&]()
				{
					s_lock.unlock(CRWLock::eLT_Write);
				});

				auto iter = s_mapDbThreadMgr.find(nID);
				DebugAst(iter != s_mapDbThreadMgr.end());
				pDbThreadMgr = iter->second;
				s_mapDbThreadMgr.erase(iter);
				DebugAst(pDbThreadMgr != nullptr);
			}
			pDbThreadMgr->exit();
			pDbThreadMgr->update();

			SAFE_DELETE(pDbThreadMgr);
		}

		void query(uint32_t nID, const google::protobuf::Message* pRequest, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
		{
			DebugAst(pRequest != nullptr);

			CDbThreadMgr* pDbThreadMgr = getDbThreadMgr(nID);
			DebugAst(pDbThreadMgr != nullptr);

			std::string szMessageName = pRequest->GetTypeName();

			uint32_t nType = 0;
			uint32_t nThreadIndex = 0;

			google::protobuf::Message* pMessage = nullptr;
			if (szMessageName == "proto.db.update_command")
			{
				const proto::db::update_command* pCommand = dynamic_cast<const proto::db::update_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Update;

				pMessage = pDbThreadMgr->createMessage(pCommand->message_name());
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", pCommand->message_name());
					return;
				}

				if (!pMessage->ParseFromString(pCommand->message_content()))
				{
					PrintWarning("parse message error {}", pCommand->message_name());
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}

				uint64_t nValue = 0;
				if (!getPrimaryValue(pMessage, nValue))
				{
					PrintWarning("get primary value error {}", pCommand->message_name());
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}

				nThreadIndex = (uint32_t)nValue;
			}
			else if (szMessageName == "proto.db.select_command")
			{
				const proto::db::select_command* pCommand = dynamic_cast<const proto::db::select_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Select;
				nThreadIndex = (uint32_t)pCommand->id();

				pMessage = pDbThreadMgr->createMessage(szMessageName);
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", szMessageName);
					return;
				}

				std::string szData = pRequest->SerializeAsString();
				if (!pMessage->ParseFromString(szData))
				{
					PrintWarning("ParseFromString message error {}", szMessageName);
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}
			}
			else if (szMessageName == "proto.db.query_command")
			{
				const proto::db::query_command* pCommand = dynamic_cast<const proto::db::query_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Query;
				nThreadIndex = (uint32_t)pCommand->channel_id();

				pMessage = pDbThreadMgr->createMessage(szMessageName);
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", szMessageName);
					return;
				}

				std::string szData = pRequest->SerializeAsString();
				if (!pMessage->ParseFromString(szData))
				{
					PrintWarning("ParseFromString message error {}", szMessageName);
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}
			}
			else if (szMessageName == "proto.db.insert_command")
			{
				const proto::db::insert_command* pCommand = dynamic_cast<const proto::db::insert_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Insert;

				pMessage = pDbThreadMgr->createMessage(pCommand->message_name());
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", pCommand->message_name());
					return;
				}

				if (!pMessage->ParseFromString(pCommand->message_content()))
				{
					PrintWarning("parse message error {}", pCommand->message_name());
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}

				uint64_t nValue = 0;
				if (!getPrimaryValue(pMessage, nValue))
				{
					PrintWarning("get primary value error {}", pCommand->message_name());
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}

				nThreadIndex = (uint32_t)nValue;
			}
			else if (szMessageName == "proto.db.flush_command")
			{
				const proto::db::flush_command* pCommand = dynamic_cast<const proto::db::flush_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				SDbCommand sDbCommand;
				sDbCommand.callback = callback;
				sDbCommand.nType = eDBCT_Flush;
				sDbCommand.pMessage = nullptr;

				for (uint32_t i = 0; i < pDbThreadMgr->getThreadCount(); ++i)
				{
					pDbThreadMgr->query(i, sDbCommand);
				}

				return;
			}
			else if (szMessageName == "proto.db.call_command")
			{
				const proto::db::call_command* pCommand = dynamic_cast<const proto::db::call_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Call;
				nThreadIndex = pCommand->channel_id();

				pMessage = pDbThreadMgr->createMessage(szMessageName);
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", szMessageName);
					return;
				}

				std::string szData = pRequest->SerializeAsString();
				if (!pMessage->ParseFromString(szData))
				{
					PrintWarning("ParseFromString message error {}", szMessageName);
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}
			}
			else if (szMessageName == "proto.db.delete_command")
			{
				const proto::db::delete_command* pCommand = dynamic_cast<const proto::db::delete_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Delete;
				nThreadIndex = (uint32_t)pCommand->id();

				pMessage = pDbThreadMgr->createMessage(szMessageName);
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", szMessageName);
					return;
				}

				std::string szData = pRequest->SerializeAsString();
				if (!pMessage->ParseFromString(szData))
				{
					PrintWarning("ParseFromString message error {}", szMessageName);
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}
			}
			else if (szMessageName == "proto.db.nop_command")
			{
				const proto::db::nop_command* pCommand = dynamic_cast<const proto::db::nop_command*>(pRequest);
				DebugAst(pCommand != nullptr);

				nType = eDBCT_Nop;
				nThreadIndex = pCommand->channel_id();

				pMessage = pDbThreadMgr->createMessage(szMessageName);
				if (pMessage == nullptr)
				{
					PrintWarning("create message error {}", szMessageName);
					return;
				}

				std::string szData = pRequest->SerializeAsString();
				if (!pMessage->ParseFromString(szData))
				{
					PrintWarning("ParseFromString message error {}", szMessageName);
					pDbThreadMgr->destroyMessage(pMessage);
					return;
				}
			}
			else
			{
				DebugAst(!"error type");
			}

			SDbCommand sDbCommand;
			sDbCommand.callback = callback;
			sDbCommand.nType = nType;
			sDbCommand.pMessage = pMessage;

			pDbThreadMgr->query(nThreadIndex, sDbCommand);
		}

		void update(uint32_t nID)
		{
			CDbThreadMgr* pDbThreadMgr = getDbThreadMgr(nID);
			DebugAst(pDbThreadMgr != nullptr);

			pDbThreadMgr->update();
		}

		uint32_t getQueueSize(uint32_t nID, uint32_t nThreadIndex)
		{
			CDbThreadMgr* pDbThreadMgr = getDbThreadMgr(nID);
			DebugAstEx(pDbThreadMgr != nullptr, 0);

			return pDbThreadMgr->getQueueSize(nThreadIndex);
		}

		uint32_t getThreadCount(uint32_t nID)
		{
			CDbThreadMgr* pDbThreadMgr = getDbThreadMgr(nID);
			DebugAstEx(pDbThreadMgr != nullptr, 0);

			return pDbThreadMgr->getThreadCount();
		}
	}
}