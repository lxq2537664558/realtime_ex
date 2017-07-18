#include "stdafx.h"
#include "base_object.h"
#include "fix_memory_pool.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/logger.h"

namespace core
{
	std::map<uint32_t, SClassInfo>* CBaseObject::s_mapClassInfo;

	CBaseObject::CBaseObject()
		: m_bDel(false)
	{
	}

	CBaseObject::~CBaseObject()
	{

	}

	void CBaseObject::registerClassInfo()
	{
		if (s_mapClassInfo == nullptr)
			s_mapClassInfo = new std::map<uint32_t, SClassInfo>();
	}

	void CBaseObject::del()
	{
		this->m_bDel = true;
	}

	void CBaseObject::registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, funCreateBaseObject pfCreateBaseObject, funDestroyBaseObject pfDestroyBaseObject)
	{
		uint32_t nClassID = CBaseObject::getClassID(szClassName);
		DebugAst(s_mapClassInfo->find(nClassID) == s_mapClassInfo->end());
		SClassInfo& classInfo = (*s_mapClassInfo)[nClassID];
		classInfo.szClassName = szClassName;
		classInfo.nClassID = nClassID;
		classInfo.pfCreateBaseObject = pfCreateBaseObject;
		classInfo.pfDestroyBaseObject = pfDestroyBaseObject;
		classInfo.pFixMemoryPool = new CFixMemoryPool(nObjectSize, nBatchCount);
	}

	void CBaseObject::unRegisterClassInfo()
	{
		for (auto iter = s_mapClassInfo->begin(); iter != s_mapClassInfo->end(); ++iter)
		{
			SClassInfo& classInfo = iter->second;
			SAFE_DELETE(classInfo.pFixMemoryPool);
		}

		SAFE_DELETE(s_mapClassInfo);
	}

	SClassInfo* CBaseObject::getClassInfo(uint32_t nClassID)
	{
		auto iter = s_mapClassInfo->find(nClassID);
		if (iter == s_mapClassInfo->end())
			return nullptr;

		return &iter->second;
	}

	uint32_t CBaseObject::getClassID(const std::string& szClassName)
	{
		return base::hash(szClassName.c_str());
	}

	void CBaseObject::destroyObject(CBaseObject* pBaseObject)
	{
		DebugAst(pBaseObject != nullptr);

		SClassInfo* pClassInfo = CBaseObject::getClassInfo(pBaseObject->getClassID());
		DebugAst(pClassInfo != nullptr);
		pClassInfo->pfDestroyBaseObject(pBaseObject);

		char* pBuf = (char*)pBaseObject;
		pClassInfo->pFixMemoryPool->deallocate(pBuf);
	}

	CBaseObject* CBaseObject::createObject(const std::string& szClassName)
	{
		return CBaseObject::createObject(CBaseObject::getClassID(szClassName));
	}

	CBaseObject* CBaseObject::createObject(uint32_t nClassID)
	{
		SClassInfo* pClassInfo = CBaseObject::getClassInfo(nClassID);
		DebugAstEx(pClassInfo != nullptr, nullptr);

		void* pBuf = pClassInfo->pFixMemoryPool->allocate();
		CBaseObject* pBaseObject = static_cast<CBaseObject*>(pClassInfo->pfCreateBaseObject(pBuf));

		return pBaseObject;
	}
}