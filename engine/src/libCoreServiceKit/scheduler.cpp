#include "stdafx.h"
#include "scheduler.h"
#include "core_service_app_impl.h"
#include "core_service_app.h"
#include "message_dispatcher.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_service_define.h"
#include "actor_base_impl.h"

#include "libCoreCommon/base_app.h"


namespace core
{
	CScheduler::CScheduler()
		: m_nNextActorID(1)
	{
		this->m_vecBuf.resize(UINT16_MAX);
	}

	CScheduler::~CScheduler()
	{

	}

	bool CScheduler::init()
	{
		return true;
	}

	CActorBaseImpl* CScheduler::getBaseActor(uint64_t nID) const
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

		CActorBaseImpl* pFromActorBaseImpl = iter->second;
		DebugAstEx(pFromActorBaseImpl != nullptr, false);

		uint16_t nServiceID = CActorBase::getServiceID(sRequestMessageInfo.nToActorID);
		if (nServiceID == 0)
		{
			auto iter = this->m_mapActorBase.find(sRequestMessageInfo.nToActorID);
			if (iter == this->m_mapActorBase.end())
				return false;

			CActorBaseImpl* pToActorBaseImpl = iter->second;
			DebugAstEx(pToActorBaseImpl != nullptr, false);

			CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(0);
			DebugAstEx(pSerializeAdapter != nullptr, false);

			message_header* pData = pSerializeAdapter->serialize(sRequestMessageInfo.pData, &this->m_vecBuf[0], (uint16_t)this->m_vecBuf.size());
			if (pData == nullptr)
				return false;

			char* pBuf = new char[sizeof(request_cookice) + pData->nMessageSize];
			// Ìî³äcookice
			actor_request_cookice* pCookice = reinterpret_cast<actor_request_cookice*>(pBuf);
			pCookice->nSessionID = sRequestMessageInfo.nSessionID;
			pCookice->nFromActorID = sRequestMessageInfo.nFromActorID;
			pCookice->nToActorID = sRequestMessageInfo.nToActorID;

			memcpy(pCookice + 1, pData, pData->nMessageSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = sRequestMessageInfo.nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_REQUEST;
			sMessagePacket.nDataSize = pData->nMessageSize + sizeof(request_cookice);
			sMessagePacket.pData = pBuf;

			pToActorBaseImpl->getChannel()->send(sMessagePacket);

			this->addWorkActorBase(pToActorBaseImpl);
			
			return true;
		}
		else
		{
			const_cast<SRequestMessageInfo&>(sRequestMessageInfo).nToActorID = CActorBase::getLocalActorID(sRequestMessageInfo.nToActorID);
			const_cast<SRequestMessageInfo&>(sRequestMessageInfo).nFromActorID = CActorBase::makeRemoteActorID(CCoreServiceAppImpl::Inst()->getNodeBaseInfo().nID, sRequestMessageInfo.nFromActorID);
			return CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo);
		}
	}

	bool CScheduler::response(const SResponseMessageInfo& sResponseMessageInfo)
	{
		uint16_t nNodeID = CActorBase::getServiceID(sResponseMessageInfo.nToActorID);
		if (nNodeID == 0)
		{
			auto iter = this->m_mapActorBase.find(sResponseMessageInfo.nToActorID);
			if (iter == this->m_mapActorBase.end())
				return false;

			CActorBaseImpl* pToActorBaseImpl = iter->second;
			DebugAstEx(pToActorBaseImpl != nullptr, false);

			CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(0);
			DebugAstEx(pSerializeAdapter != nullptr, false);

			message_header* pData = pSerializeAdapter->serialize(sResponseMessageInfo.pData, &this->m_vecBuf[0], (uint16_t)this->m_vecBuf.size());
			if (pData == nullptr)
				return false;

			char* pBuf = new char[sizeof(response_cookice) + pData->nMessageSize];

			actor_response_cookice* pCookice = reinterpret_cast<actor_response_cookice*>(pBuf);
			pCookice->nToActorID = sResponseMessageInfo.nToActorID;
			pCookice->nSessionID = sResponseMessageInfo.nSessionID;
			pCookice->nResult = sResponseMessageInfo.nResult;

			memcpy(pCookice + 1, pData, pData->nMessageSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = sResponseMessageInfo.nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_RESPONSE;
			sMessagePacket.nDataSize = pData->nMessageSize + sizeof(response_cookice);
			sMessagePacket.pData = pBuf;

			pToActorBaseImpl->getChannel()->send(sMessagePacket);

			this->addWorkActorBase(pToActorBaseImpl);
			
			return true;
		}
		else
		{
			const_cast<SResponseMessageInfo&>(sResponseMessageInfo).nToActorID = CActorBase::getLocalActorID(sResponseMessageInfo.nToActorID);
			return CCoreServiceAppImpl::Inst()->getTransporter()->response(nNodeID, sResponseMessageInfo);
		}
	}

	void CScheduler::run()
	{
		std::list<CActorBaseImpl*> listWorkActorBase;
		listWorkActorBase.swap(this->m_listWorkActorBase);
		for (auto iter = listWorkActorBase.begin(); iter != listWorkActorBase.end(); ++iter)
		{
			(*iter)->process();
		}

		if (!this->m_listWorkActorBase.empty())
			CBaseApp::Inst()->busy();
	}

	CActorBaseImpl* CScheduler::createActorBase(CActorBase* pActorBase)
	{
		DebugAstEx(pActorBase != nullptr, nullptr);

		CActorBaseImpl* pActorBaseImpl = new CActorBaseImpl(this->m_nNextActorID++, pActorBase);

		this->m_mapActorBase[pActorBaseImpl->getID()] = pActorBaseImpl;

		return pActorBaseImpl;
	}

	void CScheduler::destroyActorBase(CActorBaseImpl* pActorBaseImpl)
	{
		DebugAst(pActorBaseImpl != nullptr);

		this->m_mapActorBase.erase(pActorBaseImpl->getID());

		SAFE_DELETE(pActorBaseImpl);
	}

	void CScheduler::addWorkActorBase(CActorBaseImpl* pBaseActorImpl)
	{
		DebugAst(pBaseActorImpl != nullptr && pBaseActorImpl->getState() != CActorBaseImpl::eABS_Pending);

		if (pBaseActorImpl->getState() == CActorBaseImpl::eABS_Working)
			return;

		this->m_listWorkActorBase.push_back(pBaseActorImpl);
	}

}