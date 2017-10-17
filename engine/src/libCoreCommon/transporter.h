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

		bool					invoke(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nSessionID, const void* pMessage, uint8_t nMessageSerializerType);
		bool					response(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nSessionID, uint32_t nResult, const void* pMessage, uint8_t nMessageSerializerType);
		
		bool					gate_forward(uint64_t nSessionID, uint32_t nFromServiceID, uint32_t nToServiceID, const message_header* pData);

		bool					send(CCoreService* pCoreService, uint64_t nSessionID, uint32_t nToServiceID, const void* pMessage);

		bool					broadcast(CCoreService* pCoreService, const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const void* pMessage);

	private:
		std::vector<char>	m_szBuf;
	};
}