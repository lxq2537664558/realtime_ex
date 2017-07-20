#include "stdafx.h"
#include "base_object.h"
#include "class_info_mgr.h"

namespace core
{
	void CBaseObject::registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, funCreateBaseObject pfCreateBaseObject, funDestroyBaseObject pfDestroyBaseObject)
	{
		CClassInfoMgr::Inst()->registerClassInfo(szClassName, nObjectSize, nBatchCount, pfCreateBaseObject, pfDestroyBaseObject);
	}

	SClassInfo* CBaseObject::getClassInfo(uint32_t nClassID)
	{
		return CClassInfoMgr::Inst()->getClassInfo(nClassID);
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