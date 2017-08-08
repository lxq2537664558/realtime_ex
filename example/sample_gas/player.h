#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/actor_base.h"
#include "libCoreCommon/ticker.h"

using namespace std;
using namespace core;
using namespace base;

class CPlayer :
	public CActorBase
{
public:
	CPlayer();
	virtual ~CPlayer();

	void				setName(const std::string& szName);
	const std::string&	getName() const;

private:
	virtual void		onInit(const std::string& szContext);
	virtual void		onDestroy();

	virtual void		release();

private:
	std::string m_szName;
};