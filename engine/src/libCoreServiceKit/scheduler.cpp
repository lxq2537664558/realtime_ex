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

		auto iter = this->m_mapActorBase.find(sRequestMessageInfo.nToActorID);
		if (iter != this->m_mapActorBase.end())
		{
			CActorBaseImpl* pToActorBaseImpl = iter->second;
			DebugAstEx(pToActorBaseImpl != nullptr, false);

			google::protobuf::Message* pMessage = create_protobuf_message(sRequestMessageInfo.pMessage->GetTypeName());
			if (nullptr == pMessage)
				return false;

			pMessage->CopyFrom(*sRequestMessageInfo.pMessage);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = sRequestMessageInfo.nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_REQUEST;
			sMessagePacket.pMessage = pMessage;

			pToActorBaseImpl->getChannel()->send(sMessagePacket);

			this->addWorkActorBase(pToActorBaseImpl);
			
			return true;
		}
		else
		{
			const_cast<SRequestMessageInfo&>(sRequestMessageInfo).nToActorID = sRequestMessageInfo.nToActorID;
			const_cast<SRequestMessageInfo&>(sRequestMessageInfo).nFromActorID = sRequestMessageInfo.nFromActorID;
			CActorIDConverter* pActorIDConverter = CCoreServiceAppImpl::Inst()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			uint16_t nServiceID = pActorIDConverter->convertToServiceID(sRequestMessageInfo.nToActorID);
			DebugAstEx(nServiceID != 0, false);

			return CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo);
		}
	}

	bool CScheduler::response(const SResponseMessageInfo& sResponseMessageInfo)
	{
		auto iter = this->m_mapActorBase.find(sResponseMessageInfo.nToActorID);
		if (iter != this->m_mapActorBase.end())
		{
			CActorBaseImpl* pToActorBaseImpl = iter->second;
			DebugAstEx(pToActorBaseImpl != nullptr, false);

			google::protobuf::Message* pMessage = create_protobuf_message(sResponseMessageInfo.pMessage->GetTypeName());
			if (nullptr == pMessage)
				return false;

			pMessage->CopyFrom(*sResponseMessageInfo.pMessage);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = sResponseMessageInfo.nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_RESPONSE;
			sMessagePacket.pMessage = pMessage;

			pToActorBaseImpl->getChannel()->send(sMessagePacket);

			this->addWorkActorBase(pToActorBaseImpl);
			
			return true;
		}
		else
		{
			const_cast<SResponseMessageInfo&>(sResponseMessageInfo).nToActorID = sResponseMessageInfo.nToActorID;
			CActorIDConverter* pActorIDConverter = CCoreServiceAppImpl::Inst()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			uint16_t nServiceID = pActorIDConverter->convertToServiceID(sResponseMessageInfo.nToActorID);
			DebugAstEx(nServiceID != 0, false);

			return CCoreServiceAppImpl::Inst()->getTransporter()->response(nServiceID, sResponseMessageInfo);
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