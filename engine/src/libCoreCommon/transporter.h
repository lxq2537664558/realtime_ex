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

		bool					invoke(CCoreService* pCoreService, uint64_t nSessionID, uint64_t nFromActorID, uint32_t nToServiceID, uint64_t nToActorID, const void* pMessage);
		bool					response(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nToActorID, uint64_t nSessionID, uint8_t nResult, const void* pMessage);
		
		bool					gate_forward(uint64_t nSessionID, uint32_t nFromServiceID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);

		bool					send(CCoreService* pCoreService, uint64_t nSessionID, uint32_t nToServiceID, const void* pMessage);

		bool					broadcast(CCoreService* pCoreService, const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const void* pMessage);

	private:
		std::vector<char>	m_szBuf;
	};
}