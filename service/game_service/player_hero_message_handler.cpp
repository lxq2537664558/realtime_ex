#include "player_hero_message_handler.h"
#include "player.h"
#include "game_service.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/register_message_util.h"

#include "client_proto_src/c2s_active_hero_response.pb.h"
#include "client_proto_src/c2s_active_hero_patch_response.pb.h"
#include "hero_static_config.h"
#include "hero_patch_static_config.h"
#include "../common/error_code.h"

CPlayerHeroMessageHandler::CPlayerHeroMessageHandler(CServiceBase* pServiceBase)
{
	register_forward_message_handler(pServiceBase, this, &CPlayerHeroMessageHandler::c2s_active_hero_request_handler);
	register_forward_message_handler(pServiceBase, this, &CPlayerHeroMessageHandler::c2s_active_hero_patch_request_handler);
}

CPlayerHeroMessageHandler::~CPlayerHeroMessageHandler()
{

}

void CPlayerHeroMessageHandler::c2s_active_hero_request_handler(CServiceBase* pServiceBase, SClientSessionInfo sClientSessionInfo, const c2s_active_hero_request* pRequest)
{
	CGameService* pGameService = dynamic_cast<CGameService*>(pServiceBase);
	DebugAst(pGameService != nullptr);

	CPlayer* pPlayer = pGameService->getPlayerMgr()->getPlayer(sClientSessionInfo.nSessionID);
	if (pPlayer == nullptr)
		return;

	c2s_active_hero_response response_msg;
	response_msg.set_id(pRequest->id());

	const SHeroConfigInfo* pHeroConfigInfo = CHeroStaticConfig::Inst()->getHeroConfigInfo(pRequest->id());
	// Ó¢ÐÛid´íÎó
	if (pHeroConfigInfo == nullptr)
	{
		response_msg.set_result(eEC_Hero_NotFoundHeroConfig);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// ÅÐ¶ÏÊÇ·ñÒÑ¾­¼¤»îÁË
	if (pPlayer->getHeroModule()->isHeroActive(pHeroConfigInfo->nID))
	{
		response_msg.set_result(eEC_Hero_HeroDoubleActive);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// µÈ¼¶ÅÐ¶Ï
	if (pPlayer->getAttributeModule()->getLv() < pHeroConfigInfo->nActiveReqLv)
	{
		response_msg.set_result(eEC_Hero_ReqLvError);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// Ç°ÖÃËéÆ¬¼¤»îÅÐ¶Ï
	if (pHeroConfigInfo->nActiveReqPatchID != 0 && !pPlayer->getHeroModule()->isPatchActive(pHeroConfigInfo->nActiveReqPatchID))
	{
		response_msg.set_result(eEC_Hero_ReqPatchError);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// ÅÐ¶Ï×ÊÔ´ÏûºÄ
	for (size_t i = 0; i < pHeroConfigInfo->vecActiveCost.size(); ++i)
	{
		const SAttributeValue& sAttributeValue = pHeroConfigInfo->vecActiveCost[i];
		if (pPlayer->getAttributeModule()->getAttribute(sAttributeValue.nType) < sAttributeValue.nValue)
		{
			response_msg.set_result(eEC_Hero_ReqResourceError);
			pPlayer->sendClientMessage(&response_msg);
			return;
		}
	}

	// ÏûºÄ×ÊÔ´
	for (size_t i = 0; i < pHeroConfigInfo->vecActiveCost.size(); ++i)
	{
		const SAttributeValue& sAttributeValue = pHeroConfigInfo->vecActiveCost[i];
		int32_t nValue = sAttributeValue.nValue;
		pPlayer->getAttributeModule()->addAttribute(sAttributeValue.nType, -nValue);
	}

	pPlayer->getHeroModule()->activeHero(pHeroConfigInfo->nID);

	response_msg.set_result(eEC_Sucess);
	pPlayer->sendClientMessage(&response_msg);
	return;
}

void CPlayerHeroMessageHandler::c2s_active_hero_patch_request_handler(CServiceBase* pServiceBase, SClientSessionInfo sClientSessionInfo, const c2s_active_hero_patch_request* pRequest)
{
	CGameService* pGameService = dynamic_cast<CGameService*>(pServiceBase);
	DebugAst(pGameService != nullptr);

	CPlayer* pPlayer = pGameService->getPlayerMgr()->getPlayer(sClientSessionInfo.nSessionID);
	if (pPlayer == nullptr)
		return;

	c2s_active_hero_patch_response response_msg;
	response_msg.set_id(pRequest->id());

	const SHeroPatchConfigInfo* pHeroPatchConfigInfo = CHeroPatchStaticConfig::Inst()->getHeroPatchConfigInfo(pRequest->id());
	// Ó¢ÐÛËéÆ¬id´íÎó
	if (pHeroPatchConfigInfo == nullptr)
	{
		response_msg.set_result(eEC_Hero_NotFoundHeroPatchConfig);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// ÅÐ¶ÏÊÇ·ñÒÑ¾­¼¤»îÁË
	if (pPlayer->getHeroModule()->isPatchActive(pHeroPatchConfigInfo->nID))
	{
		response_msg.set_result(eEC_Hero_HeroPatchDoubleActive);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// µÈ¼¶²»¹»
	if (pPlayer->getAttributeModule()->getLv() < pHeroPatchConfigInfo->nActiveReqLv)
	{
		response_msg.set_result(eEC_Hero_ReqLvError);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// Ç°ÖÃËéÆ¬¼¤»îÅÐ¶Ï
	if (pHeroPatchConfigInfo->nActiveReqPatchID != 0 && !pPlayer->getHeroModule()->isPatchActive(pHeroPatchConfigInfo->nActiveReqPatchID))
	{
		response_msg.set_result(eEC_Hero_ReqPatchError);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// Ç°ÖÃÓ¢ÐÛ¼¤»îÅÐ¶Ï
	if (pHeroPatchConfigInfo->nActiveReqHeroID != 0 && !pPlayer->getHeroModule()->isHeroActive(pHeroPatchConfigInfo->nActiveReqHeroID))
	{
		response_msg.set_result(eEC_Hero_ReqHeroError);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}

	// ÅÐ¶ÏËéÆ¬ÊÇ·ñ´æÔÚ
	if (pHeroPatchConfigInfo->nActiveReqItemID != 0 && pPlayer->getItemModule()->getItemCount(pHeroPatchConfigInfo->nActiveReqItemID) == 0)
	{
		response_msg.set_result(eEC_Hero_ReqItemError);
		pPlayer->sendClientMessage(&response_msg);
		return;
	}
	
	// ¿ÛËéÆ¬
	if (pHeroPatchConfigInfo->nActiveReqItemID != 0)
		pPlayer->getItemModule()->delItem(pHeroPatchConfigInfo->nActiveReqItemID, 1);

	pPlayer->getHeroModule()->activePatch(pHeroPatchConfigInfo->nID);

	response_msg.set_result(eEC_Sucess);
	pPlayer->sendClientMessage(&response_msg);
}