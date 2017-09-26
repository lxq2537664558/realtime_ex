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
	bool				init();									// ������Դ����
	
	CBaseStaticConfig*	getBaseStaticConfig(uint32_t nType) const;	// ��ȡָ������Դ����

private:
	void				bindConfig();
	
protected:
	std::map<uint32_t, CBaseStaticConfig*>	m_mapBaseStaticConfig;	// ��Ϸ����ӳ���
};