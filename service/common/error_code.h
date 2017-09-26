#pragma once

enum EErrorCode
{
	eEC_Sucess	= 0,

	// 登陆相关
	eEC_Login_QueryGlobalDbError	= 100,
	eEC_Login_GetSuitableUCIDError = 101,
	eEC_Login_InsertGameDbError = 102,
	eEC_Login_InsertGbloalDbError = 103,
	eEC_Login_InvokeUCError	= 104,
	eEC_Login_InvokeDispatchError = 105,
	eEC_Login_InvokeGasError = 106,
	eEC_Login_CreatePlayerError = 107,

	// GM
	eEC_GM_NameError = 200,
	eEC_GM_ArgError = 201,

	// 英雄系统
	eEC_Hero_NotFoundHeroConfig = 300,
	eEC_Hero_HeroDoubleActive = 301,
	eEC_Hero_ReqLvError = 302,
	eEC_Hero_ReqPatchError = 303,
	eEC_Hero_ReqResourceError = 304,
	eEC_Hero_NotFoundHeroPatchConfig = 305,
	eEC_Hero_HeroPatchDoubleActive = 306,
	eEC_Hero_ReqHeroError = 307,
	eEC_Hero_ReqItemError = 308,
	// 道具系统
};