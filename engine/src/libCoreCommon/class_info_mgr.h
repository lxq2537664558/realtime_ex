#pragma once

#include "base_object.h"

#include "libBaseCommon\singleton.h"

#include <map>

namespace core
{
	class CClassInfoMgr :
		public base::CSingleton<CClassInfoMgr>
	{
	public:
		CClassInfoMgr();
		~CClassInfoMgr();

		void			registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, funCreateBaseObject pfCreateBaseObject, funDestroyBaseObject pfDestroyBaseObject);
		void			unRegisterClassInfo();

		SClassInfo*		getClassInfo(uint32_t nClassID);
		uint32_t		getClassID(const std::string& szClassName);
		void			destroyObject(CBaseObject* pBaseObject);
		CBaseObject*	createObject(const std::string& szClassName);
		CBaseObject*	createObject(uint32_t nClassID);

	private:
		std::map<uint32_t, SClassInfo>	m_mapClassInfo;
	};
}