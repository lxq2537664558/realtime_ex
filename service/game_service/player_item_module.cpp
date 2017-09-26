#include "libBaseCommon/string_util.h"
#include "libBaseCommon/time_util.h"

#include "player_item_module.h"
#include "player.h"

#include "../common/common.h"
#include "item_static_config.h"

#include "db_proto_src/player_item.pb.h"
#include "client_proto_src/s2c_player_item_notify.pb.h"
#include "client_proto_src/s2c_add_player_item_notify.pb.h"
#include "client_proto_src/s2c_del_player_item_notify.pb.h"

CPlayerItemModule::CPlayerItemModule(CPlayer* pPlayer)
	: CPlayerModule(pPlayer)
{

}

CPlayerItemModule::~CPlayerItemModule()
{

}

void CPlayerItemModule::onInit()
{

}

void CPlayerItemModule::onDestroy()
{

}

void CPlayerItemModule::onLoadData()
{
	this->m_pPlayer->getDbServiceInvokeHolder()->async_select(this->getPlayerID(), "player_item", [this](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
	{
		if (nErrorCode == base::db::eDBRC_EmptyRecordset)
			return;

		const proto::db::player_item* pPlayerItem = dynamic_cast<const proto::db::player_item*>(pMessage);
		if (nullptr == pPlayerItem)
		{
			this->m_pPlayer->setModuleLoadDataError(this->getModuleType());

			PrintInfo("CPlayerItemModule::onLoadData error player_id: {}", this->getPlayerID());
			return;
		}

		for (int32_t i = 0; i < pPlayerItem->data().data_size(); ++i)
		{
			const proto::db::player_item_data& sItem = pPlayerItem->data().data(i);
			this->m_mapItem[sItem.id()] = sItem.count();
		}
	});
}

void CPlayerItemModule::onBackup()
{
	proto::db::player_item db_msg;
	db_msg.set_player_id(this->getPlayerID());
	for (auto iter = this->m_mapItem.begin(); iter != this->m_mapItem.end(); ++iter)
	{
		proto::db::player_item_data* pItem = db_msg.mutable_data()->add_data();
		DebugAst(pItem != nullptr);

		pItem->set_id(iter->first);
		pItem->set_count(iter->second);
	}
	
	this->m_pPlayer->getDbServiceInvokeHolder()->update(&db_msg);
}

void CPlayerItemModule::onPlayerLogin()
{
	s2c_player_item_notify notify_msg;
	for (auto iter = this->m_mapItem.begin(); iter != this->m_mapItem.end(); ++iter)
	{
		auto* pItem = notify_msg.add_data();
		DebugAst(pItem != nullptr);

		pItem->set_id(iter->first);
		pItem->set_count(iter->second);
	}

	this->m_pPlayer->sendClientMessage(&notify_msg);
}

void CPlayerItemModule::onPlayerLogout()
{
	
}

void CPlayerItemModule::onModuleEvent(const IPlayerModuleEvent* pEvent)
{

}

void CPlayerItemModule::addItem(uint32_t nID, uint32_t nCount, uint32_t nActionType /*= 0*/, const void* pContext /*= nullptr*/)
{
	const SItemConfigInfo* pItemConfigInfo = CItemStaticConfig::Inst()->getItemConfigInfo(nID);
	DebugAst(pItemConfigInfo != nullptr);

	uint32_t& nTotalCount = this->m_mapItem[nID];
	nTotalCount += nCount;

	s2c_add_player_item_notify notify_msg;
	notify_msg.set_id(nID);
	notify_msg.set_delta(nCount);
	notify_msg.set_count(nTotalCount);

	this->m_pPlayer->sendClientMessage(&notify_msg);

	CPlayerAddItemEvent event;
	event.nItemID = nID;
	event.nCount = nCount;
	this->m_pPlayer->raiseEvent(&event);
}

void CPlayerItemModule::delItem(uint32_t nID, uint32_t nCount, uint32_t nActionType /*= 0*/, const void* pContext /*= nullptr*/)
{
	auto iter = this->m_mapItem.find(nID);
	if (iter == this->m_mapItem.end())
		return;

	uint32_t nTotalCount = iter->second;
	if (nTotalCount <= nCount)
	{
		nTotalCount = 0;
		this->m_mapItem.erase(nID);
	}
	else
	{
		nTotalCount -= nCount;
		this->m_mapItem[nID] = nTotalCount;
	}

	s2c_del_player_item_notify notify_msg;
	notify_msg.set_id(nID);
	notify_msg.set_delta(nCount);
	notify_msg.set_count(nTotalCount);

	this->m_pPlayer->sendClientMessage(&notify_msg);

	CPlayerDelItemEvent event;
	event.nItemID = nID;
	event.nCount = nCount;
	this->m_pPlayer->raiseEvent(&event);
}

uint32_t CPlayerItemModule::getItemCount(uint32_t nID) const
{
	auto iter = this->m_mapItem.find(nID);
	if (iter == this->m_mapItem.end())
		return 0;

	return iter->second;
}