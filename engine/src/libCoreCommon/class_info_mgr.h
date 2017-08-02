#pragma once

#include <map>

#include "base_object.h"
#include "fix_memory_pool.h"

#include "libBaseCommon\singleton.h"

namespace core
{
	class CClassInfoMgr :
		public base::CSingleton<CClassInfoMgr>
	{
	private:
		struct SClassInfo
		{
			std::string		szClassName;
			uint32_t		nClassID;
			CFixMemoryPool*	pFixMemoryPool;
			std::function<CBaseObject*(void*)>
				fnCreateBaseObject;
			std::function<void(CBaseObject*)>
				fnDestroyBaseObject;
		};

	public:
		CClassInfoMgr();
		~CClassInfoMgr();

		void			registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, const std::function<CBaseObject*(void*)>& fnCreateBaseObject, const std::function<void(CBaseObject*)>& fnDestroyBaseObject);
		void			unRegisterClassInfo();

		SClassInfo*		getClassInfo(uint32_t nClassID);
		uint32_t		getClassID(const std::string& szClassName);

		CBaseObject*	createObject(const std::string& szClassName);
		CBaseObject*	createObject(uint32_t nClassID);
		void			destroyObject(CBaseObject* pBaseObject);

	private:
		std::map<uint32_t, SClassInfo>	m_mapClassInfo;
	};
}