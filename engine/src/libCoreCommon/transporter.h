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
		CTransporter(CCoreService* pCoreService);
		~CTransporter();

		bool	invoke(uint32_t nToServiceID, uint64_t nSessionID, const void* pMessage, const SInvokeOption* pInvokeOption);
		bool	response(uint32_t nToServiceID, uint64_t nSessionID, uint32_t nResult, const void* pMessage, uint8_t nMessageSerializerType);

		bool	gate_forward(uint64_t nSessionID, uint32_t nToServiceID, const message_header* pData);

		bool	send(uint64_t nSessionID, uint32_t nToServiceID, const void* pMessage);

		bool	broadcast(const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const void* pMessage);

	private:
		std::vector<char>	m_szBuf;
		CCoreService*		m_pCoreService;
	};
}