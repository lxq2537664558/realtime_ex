#pragma once
#include <stdint.h>
#include <string>

class CBaseStaticConfig
{
public:
	CBaseStaticConfig();
	virtual ~CBaseStaticConfig();

	virtual bool		    init()					= 0;	// 初始化
	virtual uint32_t	    getConfigType() const	= 0;	// 获取配置类型
    virtual std::string     getConfigName()			= 0;    // 获取配置名称
	virtual void			release()				= 0;
};
