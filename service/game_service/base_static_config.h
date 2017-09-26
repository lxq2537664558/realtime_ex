#pragma once
#include <stdint.h>
#include <string>

class CBaseStaticConfig
{
public:
	CBaseStaticConfig();
	virtual ~CBaseStaticConfig();

	virtual bool		    init()					= 0;	// ��ʼ��
	virtual uint32_t	    getConfigType() const	= 0;	// ��ȡ��������
    virtual std::string     getConfigName()			= 0;    // ��ȡ��������
	virtual void			release()				= 0;
};
