#pragma once
#include "libCoreCommon/actor_id_converter.h"

class CMyActorIDConverter :
	public core::CActorIDConverter
{
public:
	CMyActorIDConverter() { }
	virtual ~CMyActorIDConverter() { }

	virtual uint32_t convertToServiceID(uint64_t nActorID);
};