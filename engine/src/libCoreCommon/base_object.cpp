#include "stdafx.h"
#include "base_object.h"
#include "class_info_mgr.h"

namespace core
{
	void CBaseObject::registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, const std::function<CBaseObject*(void*)>& fnCreateBaseObject, const std::function<void(CBaseObject*)>& fnDestroyBaseObject)
	{
		CClassInfoMgr::Inst()->registerClassInfo(szClassName, nObjectSize, nBatchCount, fnCreateBaseObject, fnDestroyBaseObject);
	}

	uint32_t CBaseObject::getClassID(const std::string& szClassName)
	{
		return CClassInfoMgr::Inst()->getClassID(szClassName);
	}

	void CBaseObject::destroyObject(CBaseObject* pBaseObject)
	{
		CClassInfoMgr::Inst()->destroyObject(pBaseObject);
	}

	CBaseObject* CBaseObject::createObject(const std::string& szClassName)
	{
		return CClassInfoMgr::Inst()->createObject(szClassName);
	}

	CBaseObject* CBaseObject::createObject(uint32_t nClassID)
	{
		return CClassInfoMgr::Inst()->createObject(nClassID);
	}
}