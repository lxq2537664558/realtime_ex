#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/base_connection.h"

#include "service_base.h"
#include "core_common_define.h"

#include <map>
#include <list>

namespace core
{
	class CTransporter :
		public base::noncopyable
	{
	public:
		CTransporter();
		~CTransporter();

		bool					invoke(CCoreService* pCoreService, uint64_t nSessionID, uint64_t nFromActorID, uint32_t nToServiceID, uint64_t nToActorID, const google::protobuf::Message* pMessage);
		bool					response(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nToActorID, uint64_t nSessionID, uint8_t nResult, const google::protobuf::Message* pMessage);
		
		bool					gate_forward(uint64_t nSessionID, uint32_t nFromServiceID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);

		bool					send(CCoreService* pCoreService, uint64_t nSessionID, uint32_t nToServiceID, const google::protobuf::Message* pMessage);

		bool					broadcast(CCoreService* pCoreService, const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const google::protobuf::Message* pMessage);

		SPendingResponseInfo*	getPendingResponseInfo(uint64_t nSessionID);
		SPendingResponseInfo*	addPendingResponseInfo(uint64_t nSessionID, uint64_t nCoroutineID, const std::string& szMessageName, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, uint64_t nHolderID);
		void					delPendingResponseInfo(uint64_t nHolderID);

		uint64_t				genSessionID();

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		
	private:
		uint64_t									m_nNextSessionID;
		std::map<uint64_t, SPendingResponseInfo*>	m_mapPendingResponseInfo;
		std::map<uint64_t, std::list<uint64_t>>		m_mapHolderSessionIDList;
		std::vector<char>							m_szBuf;
	};
}