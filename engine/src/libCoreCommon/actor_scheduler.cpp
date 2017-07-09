#include "stdafx.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_service_define.h"
#include "actor_base_impl.h"

#include "libCoreCommon/base_app.h"


namespace core
{
	CActorScheduler::CActorScheduler()
		: m_nNextActorID(1)
	{
		this->m_vecBuf.resize(UINT16_MAX);
	}

	CActorScheduler::~CActorScheduler()
	{

	}

	bool CActorScheduler::init()
	{
		return true;
	}

	CActorBaseImpl* CActorScheduler::getActorBase(uint64_t nID) const
	{
		auto iter = this->m_mapActorBase.find(nID);
		if (iter == this->m_mapActorBase.end())
			return nullptr;

		return iter->second;
	}

	bool CActorScheduler::invoke(uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		auto iter = this->m_mapActorBase.find(nFromActorID);
		if (iter == this->m_mapActorBase.end())
			return false;

		CActorBaseImpl* pFromActorBaseImpl = iter->second;
		DebugAstEx(pFromActorBaseImpl != nullptr, false);

		iter = this->m_mapActorBase.find(nToActorID);
		if (iter != this->m_mapActorBase.end())
		{
			CActorBaseImpl* pToActorBaseImpl = iter->second;
			DebugAstEx(pToActorBaseImpl != nullptr, false);

			google::protobuf::Message* pNewMessage = create_protobuf_message(pMessage->GetTypeName());
			if (nullptr == pMessage)
				return false;

			pNewMessage->CopyFrom(*pMessage);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_REQUEST;
			sMessagePacket.pData = pNewMessage;

			pToActorBaseImpl->getChannel()->send(sMessagePacket);

			this->addWorkActorBase(pToActorBaseImpl);
			
			return true;
		}
		else
		{
			return CCoreApp::Inst()->getTransporter()->invoke_a(nSessionID, nFromActorID, nToActorID, pMessage);
		}
	}

	bool CActorScheduler::response(uint64_t nSessionID, uint8_t nResult, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		auto iter = this->m_mapActorBase.find(nToActorID);
		if (iter != this->m_mapActorBase.end())
		{
			CActorBaseImpl* pToActorBaseImpl = iter->second;
			DebugAstEx(pToActorBaseImpl != nullptr, false);

			google::protobuf::Message* pNewMessage = create_protobuf_message(pMessage->GetTypeName());
			if (nullptr == pMessage)
				return false;

			pNewMessage->CopyFrom(*pMessage);

			SMessagePacket sMessagePacket;
			sMessagePacket.nID = 0;
			sMessagePacket.nType = eMT_ACTOR_RESPONSE;
			sMessagePacket.pData = pNewMessage;

			pToActorBaseImpl->getChannel()->send(sMessagePacket);

			this->addWorkActorBase(pToActorBaseImpl);
			
			return true;
		}
		else
		{
			return CCoreApp::Inst()->getTransporter()->response_a(nSessionID, nResult, nToActorID, pMessage);
		}
	}

	void CActorScheduler::run()
	{
		std::list<CActorBaseImpl*> listWorkActorBase;
		listWorkActorBase.swap(this->m_listWorkActorBase);
		for (auto iter = listWorkActorBase.begin(); iter != listWorkActorBase.end(); ++iter)
		{
			(*iter)->process();
		}
	}

	CActorBaseImpl* CActorScheduler::createActorBase(CActorBase* pActorBase)
	{
		DebugAstEx(pActorBase != nullptr, nullptr);

		CActorBaseImpl* pActorBaseImpl = new CActorBaseImpl(this->m_nNextActorID++, pActorBase);

		this->m_mapActorBase[pActorBaseImpl->getID()] = pActorBaseImpl;

		return pActorBaseImpl;
	}

	void CActorScheduler::destroyActorBase(CActorBaseImpl* pActorBaseImpl)
	{
		DebugAst(pActorBaseImpl != nullptr);

		this->m_mapActorBase.erase(pActorBaseImpl->getID());

		SAFE_DELETE(pActorBaseImpl);
	}

	void CActorScheduler::addWorkActorBase(CActorBaseImpl* pBaseActorImpl)
	{
		DebugAst(pBaseActorImpl != nullptr && pBaseActorImpl->getState() != CActorBaseImpl::eABS_Pending);

		if (pBaseActorImpl->getState() == CActorBaseImpl::eABS_Working)
			return;

		this->m_listWorkActorBase.push_back(pBaseActorImpl);
	}

}