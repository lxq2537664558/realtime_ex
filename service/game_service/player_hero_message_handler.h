#pragma once
#include "libCoreCommon/service_base.h"

#include "client_proto_src/c2s_active_hero_request.pb.h"
#include "client_proto_src/c2s_active_hero_patch_request.pb.h"

using namespace core;

class CPlayerHeroMessageHandler 
{
public:
	CPlayerHeroMessageHandler(CServiceBase* pServiceBase);
	~CPlayerHeroMessageHandler();

private:
	void c2s_active_hero_request_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const c2s_active_hero_request* pRequest);
	void c2s_active_hero_patch_request_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const c2s_active_hero_patch_request* pRequest);
};