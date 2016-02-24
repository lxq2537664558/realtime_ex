#pragma once

#include "core_common.h"
#include "base_object.h"

namespace core
{

	class CBaseConnectionMgr;
	class CCoreConnection;
	class CBaseConnection :
		public CBaseObject
	{
		friend class CBaseConnectionMgr;
		friend class CCoreConnection;

	public:
		CBaseConnection();
		virtual ~CBaseConnection();

		void					send(uint16_t nMsgType, const void* pData, uint16_t nSize);
		void					send(uint16_t nMsgType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);

		void					shutdown(bool bForce, const std::string& szMsg);

		uint64_t				getID() const;

		virtual void			onConnect(const std::string& szContext) { }
		virtual void			onDisconnect() { }
		virtual void			onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize) { }

		const SNetAddr&			getLocalAddr() const;
		const SNetAddr&			getRemoteAddr() const;

	private:
		CCoreConnection*	m_pCoreConnection;
	};
}