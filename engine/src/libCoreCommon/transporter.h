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

		bool					init();

		bool					invoke(CServiceBaseImpl* pServiceBaseImpl, EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromID, uint64_t nToID, const google::protobuf::Message* pMessage);
		bool					response(CServiceBaseImpl* pServiceBaseImpl, EMessageTargetType eType, uint64_t nSessionID, uint8_t nResult, uint64_t nToID, const google::protobuf::Message* pMessage);
		
		bool					invoke_a(CServiceBaseImpl* pServiceBaseImpl, EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToID, const google::protobuf::Message* pMessage);
		bool					response_a(CServiceBaseImpl* pServiceBaseImpl, EMessageTargetType eType, uint64_t nSessionID, uint8_t nResult, uint64_t nToID, const google::protobuf::Message* pMessage);

		bool					forward(CServiceBaseImpl* pServiceBaseImpl, EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromID, uint64_t nToID, const google::protobuf::Message* pMessage);
		
		bool					send(uint64_t nSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage);

		bool					broadcast(const std::vector<uint64_t>& vecSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage);

		SPendingResponseInfo*	getPendingResponseInfo(uint64_t nSessionID, bool bErase);
		SPendingResponseInfo*	addPendingResponseInfo(uint64_t nSessionID, uint64_t nToID, const std::string& szMessageName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);

		uint64_t				genSessionID();

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		
	private:
		uint64_t									m_nNextSessionID;
		std::map<uint64_t, SPendingResponseInfo*>	m_mapPendingResponseInfo;
		std::vector<char>							m_szBuf;
	};
}