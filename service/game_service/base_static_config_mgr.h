#pragma once

#include <map>

#include "base_static_config.h"

#include "libBaseCommon/noncopyable.h"

class CBaseStaticConfigMgr :
	public base::noncopyable
{
public:
    CBaseStaticConfigMgr();
    virtual ~CBaseStaticConfigMgr();

public:
	bool				init();									// 加载资源配置
	
	CBaseStaticConfig*	getBaseStaticConfig(uint32_t nType) const;	// 获取指定的资源配置

private:
	void				bindConfig();
	
protected:
	std::map<uint32_t, CBaseStaticConfig*>	m_mapBaseStaticConfig;	// 游戏配置映射表
};