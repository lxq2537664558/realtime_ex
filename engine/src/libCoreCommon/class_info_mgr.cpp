#include "stdafx.h"
#include "class_info_mgr.h"
#include "fix_memory_pool.h"

namespace core
{
	CClassInfoMgr::CClassInfoMgr()
	{

	}

	CClassInfoMgr::~CClassInfoMgr()
	{

	}

	void CClassInfoMgr::registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, const std::function<CBaseObject*(void*)>& fnCreateBaseObject, const std::function<void(CBaseObject*)>& fnDestroyBaseObject)
	{
		uint32_t nClassID = this->getClassID(szClassName);
		DebugAst(this->m_mapClassInfo.find(nClassID) == this->m_mapClassInfo.end());

		SClassInfo& classInfo = this->m_mapClassInfo[nClassID];
		classInfo.szClassName = szClassName;
		classInfo.nClassID = nClassID;
		classInfo.fnCreateBaseObject = fnCreateBaseObject;
		classInfo.fnDestroyBaseObject = fnDestroyBaseObject;
		classInfo.pFixMemoryPool = new CFixMemoryPool(nObjectSize, nBatchCount);
	}

	void CClassInfoMgr::unRegisterClassInfo()
	{
		for (auto iter = this->m_mapClassInfo.begin(); iter != this->m_mapClassInfo.end(); ++iter)
		{
			SClassInfo& classInfo = iter->second;
			SAFE_DELETE(classInfo.pFixMemoryPool);
		}
	}

	CClassInfoMgr::SClassInfo* CClassInfoMgr::getClassInfo(uint32_t nClassID)
	{
		auto iter = this->m_mapClassInfo.find(nClassID);
		if (iter == this->m_mapClassInfo.end())
			return nullptr;

		return &iter->second;
	}

	uint32_t CClassInfoMgr::getClassID(const std::string& szClassName)
	{
		return base::function_util::hash(szClassName.c_str());
	}

	void CClassInfoMgr::destroyObject(CBaseObject* pBaseObject)
	{
		DebugAst(pBaseObject != nullptr);

		SClassInfo* pClassInfo = this->getClassInfo(pBaseObject->getClassID());
		DebugAst(pClassInfo != nullptr);
		pClassInfo->fnDestroyBaseObject(pBaseObject);

		char* pBuf = (char*)pBaseObject;
		pClassInfo->pFixMemoryPool->deallocate(pBuf);
	}

	CBaseObject* CClassInfoMgr::createObject(const std::string& szClassName)
	{
		return this->createObject(this->getClassID(szClassName));
	}

	CBaseObject* CClassInfoMgr::createObject(uint32_t nClassID)
	{
		SClassInfo* pClassInfo = this->getClassInfo(nClassID);
		DebugAstEx(pClassInfo != nullptr, nullptr);

		void* pBuf = pClassInfo->pFixMemoryPool->allocate();
		CBaseObject* pBaseObject = static_cast<CBaseObject*>(pClassInfo->fnCreateBaseObject(pBuf));

		return pBaseObject;
	}
}