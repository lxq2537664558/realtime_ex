#include "stdafx.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"
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

	bool CActorScheduler::invoke(EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		auto iter = this->m_mapActorBase.find(nFromActorID);
		if (iter == this->m_mapActorBase.end())
			return false;

		CActorBaseImpl* pFromActorBaseImpl = iter->second;
		DebugAstEx(pFromActorBaseImpl != nullptr, false);

		if (eType == eMTT_Actor)
		{
			iter = this->m_mapActorBase.find(nToID);
			if (iter != this->m_mapActorBase.end())
			{
				CActorBaseImpl* pToActorBaseImpl = iter->second;
				DebugAstEx(pToActorBaseImpl != nullptr, false);

				google::protobuf::Message* pNewMessage = create_protobuf_message(pMessage->GetTypeName());
				if (nullptr == pMessage)
					return false;

				pNewMessage->CopyFrom(*pMessage);

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = nFromActorID;
				sActorMessagePacket.nSessionID = nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pNewMessage;

				pToActorBaseImpl->getChannel()->send(sActorMessagePacket);

				this->addWorkActorBase(pToActorBaseImpl);

				return true;
			}
			else
			{
				return CCoreApp::Inst()->getTransporter()->invoke(eMTT_Actor, nSessionID, nFromActorID, nToID, pMessage);
			}
		}
		else
		{
			return CCoreApp::Inst()->getTransporter()->invoke(eMTT_Service, nSessionID, nFromActorID, nToID, pMessage);
		}
	}

	bool CActorScheduler::response(EMessageTargetType eType, uint64_t nSessionID, uint8_t nResult, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		if (eType == eMTT_Actor)
		{
			auto iter = this->m_mapActorBase.find(nToID);
			if (iter != this->m_mapActorBase.end())
			{
				CActorBaseImpl* pToActorBaseImpl = iter->second;
				DebugAstEx(pToActorBaseImpl != nullptr, false);

				google::protobuf::Message* pNewMessage = create_protobuf_message(pMessage->GetTypeName());
				if (nullptr == pMessage)
					return false;

				pNewMessage->CopyFrom(*pMessage);

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = nResult;
				sActorMessagePacket.nSessionID = nSessionID;
				sActorMessagePacket.nType = eMT_RESPONSE;
				sActorMessagePacket.pMessage = pNewMessage;

				pToActorBaseImpl->getChannel()->send(sActorMessagePacket);

				this->addWorkActorBase(pToActorBaseImpl);

				return true;
			}
			else
			{
				return CCoreApp::Inst()->getTransporter()->response(eMTT_Actor, nSessionID, nResult, nToID, pMessage);
			}
		}
		else
		{
			return CCoreApp::Inst()->getTransporter()->response(eMTT_Service, nSessionID, nResult, nToID, pMessage);
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