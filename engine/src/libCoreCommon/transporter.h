#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/base_connection.h"

#include "service_base.h"
#include "core_common_define.h"

#include <map>

namespace core
{
	class CTransporter :
		public base::noncopyable
	{
	public:
		CTransporter();
		~CTransporter();

		bool					invoke(CCoreService* pCoreService, uint64_t nSessionID, EMessageTargetType eFromType, uint64_t nFromID, EMessageTargetType eToType, uint64_t nToID, const google::protobuf::Message* pMessage);
		bool					response(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nToActorID, uint64_t nSessionID, uint8_t nResult, const google::protobuf::Message* pMessage);
		
		bool					invoke_a(CCoreService* pCoreService, uint64_t nSessionID, uint64_t nFromActorID, EMessageTargetType eToType, uint64_t nToID, const google::protobuf::Message* pMessage);
		
		bool					forward(CCoreService* pCoreService, EMessageTargetType eType, uint64_t nID, const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		bool					gate_forward(uint64_t nSessionID, uint64_t nSocketID, uint32_t nFromServiceID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);

		bool					send(CCoreService* pCoreService, uint64_t nSessionID, uint64_t nSocketID, uint32_t nToServiceID, const google::protobuf::Message* pMessage);

		bool					broadcast(CCoreService* pCoreService, const std::vector<std::pair<uint64_t, uint64_t>>& vecSessionID, uint32_t nToServiceID, const google::protobuf::Message* pMessage);

		SPendingResponseInfo*	getPendingResponseInfo(uint64_t nSessionID, bool bErase);
		SPendingResponseInfo*	addPendingResponseInfo(uint64_t nSessionID, uint64_t nToID, const std::string& szMessageName, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);

		uint64_t				genSessionID();

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		
	private:
		uint64_t									m_nNextSessionID;
		std::map<uint64_t, SPendingResponseInfo*>	m_mapPendingResponseInfo;
		std::vector<char>							m_szBuf;
	};
}