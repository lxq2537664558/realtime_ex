#include "stdafx.h"
#include "scheduler.h"
#include "core_service_app_impl.h"
#include "message_registry.h"
#include "message_dispatcher.h"
#include "actor.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/coroutine.h"

static bool actor_message_forward(uint16_t nFromServiceID, uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
	{
		DebugAstEx(nSize > sizeof(core::request_cookice), true);

		const core::request_cookice* pCookice = reinterpret_cast<const core::request_cookice*>(pData);

		// °þµôcookice
		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);

		if (pCookice->nToActorID != 0)
		{
			core::CActorBase* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getActorBase(pCookice->nToActorID);
			if (NULL == pActorBase)
				return true;
			
			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = pCookice->nFromActorID;
			sMessagePacket.nType = eMT_REQUEST;
			sMessagePacket.nDataSize = nSize;
			sMessagePacket.pData = const_cast<void*>(pData);
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);

			return false;
		}
	}
	else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
	{
		const core::response_cookice* pCookice = reinterpret_cast<const core::response_cookice*>(pData);
		// °þµôcookice
		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);

		if (pCookice->nActorID != 0)
		{
			core::CActorBase* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getActorBase(pCookice->nActorID);
			if (NULL == pActorBase)
				return true;

			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = 0;
			sMessagePacket.nType = eMT_RESPONSE;
			sMessagePacket.nDataSize = nSize;
			sMessagePacket.pData = const_cast<void*>(pData);
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);

			return false;
		}
	}
	else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
	{
		const core::gate_forward_cookice* pCookice = reinterpret_cast<const core::gate_forward_cookice*>(pData);
		// °þµôcookice
		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);

		if (pCookice->nActorID != 0)
		{
			core::CActorBase* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getActorBase(pCookice->nActorID);
			if (NULL == pActorBase)
				return true;

			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = nFromServiceID;
			sMessagePacket.nType = eMT_GATE_FORWARD;
			sMessagePacket.nDataSize = nSize;
			sMessagePacket.pData = const_cast<void*>(pData);
			pActorBase->getChannel()->send(sMessagePacket);
			
			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);

			return false;
		}
	}

	return true;
}

namespace core
{
	CScheduler::CScheduler()
		: m_nNextActorID(1)
	{

	}

	CScheduler::~CScheduler()
	{

	}

	bool CScheduler::init()
	{
		CMessageDispatcher::Inst()->setForwardCallback(std::bind(&actor_message_forward, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		return true;
	}

	CActorBase* CScheduler::getActorBase(uint64_t nID) const
	{
		auto iter = this->m_mapActorBase.find(nID);
		if (iter == this->m_mapActorBase.end())
			return nullptr;

		return iter->second;
	}

	bool CScheduler::invoke(const SRequestMessageInfo& sRequestMessageInfo)
	{
		auto iter = this->m_mapActorBase.find(sRequestMessageInfo.nFromActorID);
		if (iter == this->m_mapActorBase.end())
			return false;

		CActorBase* pFromActorBase = iter->second;
		DebugAstEx(pFromActorBase != nullptr, false);

		uint16_t nServiceID = CActor::getServiceID(sRequestMessageInfo.nToActorID);
		if (nServiceID == 0)
		{
			auto iter = this->m_mapActorBase.find(sRequestMessageInfo.nToActorID);
			if (iter == this->m_mapActorBase.end())
				return false;

			CActorBase* pToActorBase = iter->second;
			DebugAstEx(pToActorBase != nullptr, false);

			char* pData = new char[sizeof(request_cookice) + sRequestMessageInfo.pData->nMessageSize];
			// Ìî³äcookice
			request_cookice* pCookice = reinterpret_cast<request_cookice*>(pData);
			pCookice->nSessionID = sRequestMessageInfo.nSessionID;
			pCookice->nTraceID = 0;
			pCookice->nFromActorID = sRequestMessageInfo.nFromActorID;
			pCookice->nToActorID = sRequestMessageInfo.nToActorID;

			memcpy(pCookice + 1, sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = sRequestMessageInfo.nFromActorID;
			sMessagePacket.nType = eMT_REQUEST;
			sMessagePacket.nDataSize = sRequestMessageInfo.pData->nMessageSize + sizeof(request_cookice);
			sMessagePacket.pData = pData;

			pToActorBase->getChannel()->send(sMessagePacket);

			this->m_mapWorkActorBase[pToActorBase->getID()] = pToActorBase;

			return true;
		}
		else
		{
			const_cast<SRequestMessageInfo&>(sRequestMessageInfo).nToActorID = CActor::getLocalActorID(sRequestMessageInfo.nToActorID);
			const_cast<SRequestMessageInfo&>(sRequestMessageInfo).nFromActorID = CActor::getRemoteActorID(CCoreServiceAppImpl::Inst()->getServiceBaseInfo().nID, sRequestMessageInfo.nFromActorID);
			return CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo);
		}
	}

	bool CScheduler::response(const SResponseMessageInfo& sResponseMessageInfo)
	{
		uint16_t nServiceID = CActor::getServiceID(sResponseMessageInfo.nToActorID);
		if (nServiceID == 0)
		{
			auto iter = this->m_mapActorBase.find(sResponseMessageInfo.nToActorID);
			if (iter == this->m_mapActorBase.end())
				return false;

			CActorBase* pToActorBase = iter->second;
			DebugAstEx(pToActorBase != nullptr, false);

			char* pData = new char[sizeof(response_cookice) + sResponseMessageInfo.pData->nMessageSize];

			response_cookice* pCookice = reinterpret_cast<response_cookice*>(pData);
			pCookice->nTraceID = 0;
			pCookice->nSessionID = sResponseMessageInfo.nSessionID;
			pCookice->nResult = sResponseMessageInfo.nResult;

			memcpy(pCookice + 1, sResponseMessageInfo.pData, sResponseMessageInfo.pData->nMessageSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = sResponseMessageInfo.nFromActorID;
			sMessagePacket.nType = eMT_RESPONSE;
			sMessagePacket.nDataSize = sResponseMessageInfo.pData->nMessageSize + sizeof(response_cookice);
			sMessagePacket.pData = pData;

			pToActorBase->getChannel()->send(sMessagePacket);

			this->m_mapWorkActorBase[pToActorBase->getID()] = pToActorBase;

			return true;
		}
		else
		{
			const_cast<SResponseMessageInfo&>(sResponseMessageInfo).nToActorID = CActor::getLocalActorID(sResponseMessageInfo.nToActorID);
			return CCoreServiceAppImpl::Inst()->getTransporter()->response(nServiceID, sResponseMessageInfo);
		}
	}

	void CScheduler::run()
	{
		std::map<uint64_t, CActorBase*> mapWorkActorBase;
		mapWorkActorBase.swap(this->m_mapWorkActorBase);
		for (auto iter = mapWorkActorBase.begin(); iter != mapWorkActorBase.end(); ++iter)
		{
			iter->second->run();
		}
	}

	CActorBase* CScheduler::createActorBase(CActor* pActor)
	{
		DebugAstEx(pActor != nullptr, nullptr);

		CActorBase* pActorBase = new CActorBase(this->m_nNextActorID++, pActor);

		this->m_mapActorBase[pActorBase->getID()] = pActorBase;

		return pActorBase;
	}

	void CScheduler::destroyActorBase(CActorBase* pActorBase)
	{
		DebugAst(pActorBase != nullptr);

		this->m_mapActorBase.erase(pActorBase->getID());

		SAFE_DELETE(pActorBase);
	}

	void CScheduler::addWorkActorBase(CActorBase* pActorBase)
	{
		DebugAst(pActorBase != nullptr);

		this->m_mapWorkActorBase[pActorBase->getID()] = pActorBase;
	}

}