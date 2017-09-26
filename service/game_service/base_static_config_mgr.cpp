#include "base_static_config_mgr.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

#include "shell_common.h"
#include "hero_static_config.h"
#include "item_static_config.h"
#include "hero_patch_static_config.h"
#include "lv_static_config.h"

CBaseStaticConfigMgr::CBaseStaticConfigMgr()
{

}

CBaseStaticConfigMgr::~CBaseStaticConfigMgr()
{
	for (auto iter = this->m_mapBaseStaticConfig.begin(); iter != this->m_mapBaseStaticConfig.end(); ++iter)
	{
		CBaseStaticConfig* pBaseStaticConfig = iter->second;
		pBaseStaticConfig->release();
	}
}

bool CBaseStaticConfigMgr::init()
{
	this->m_mapBaseStaticConfig.clear();

	this->bindConfig();

	for ( auto iter = this->m_mapBaseStaticConfig.begin(); iter != this->m_mapBaseStaticConfig.end(); ++iter)
	{
		CBaseStaticConfig* pBaseStaticConfig = iter->second;
		DebugAstEx(pBaseStaticConfig != nullptr, false);

		if (!pBaseStaticConfig->init())
        {
			PrintWarning("load static config error type: {} name: {}", pBaseStaticConfig->getConfigType(), pBaseStaticConfig->getConfigName());
#if _DEBUG
			continue;
#else
			return false;
#endif
        }
	}

	return true;
}

CBaseStaticConfig* CBaseStaticConfigMgr::getBaseStaticConfig(uint32_t nType) const
{
	auto iter = m_mapBaseStaticConfig.find(nType);
	if (iter == m_mapBaseStaticConfig.end())
		return nullptr;

	return iter->second;
}

void CBaseStaticConfigMgr::bindConfig()
{
	this->m_mapBaseStaticConfig[eSCT_Lv] = CLvStaticConfig::Inst();
	this->m_mapBaseStaticConfig[eSCT_Item] = CItemStaticConfig::Inst();
	this->m_mapBaseStaticConfig[eSCT_Hero] = CHeroStaticConfig::Inst();
	this->m_mapBaseStaticConfig[eSCT_HeroPatch] = CHeroPatchStaticConfig::Inst();
}
