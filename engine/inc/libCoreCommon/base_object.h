#pragma once

#include <functional>

#include "libBaseCommon/base_common.h"
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"

namespace core
{
	class CBaseObject;
	typedef std::function<CBaseObject*(void*)>	funCreateBaseObject;
	typedef std::function<void(CBaseObject*)>	funDestroyBaseObject;

	class CFixMemoryPool;
	struct SClassInfo
	{
		std::string				szClassName;
		uint32_t				nClassID;
		funCreateBaseObject		fnCreateBaseObject;
		funDestroyBaseObject	fnDestroyBaseObject;
		CFixMemoryPool*			pFixMemoryPool;
	};

	/**
	@brief: 逻辑基础类，是所有框架对象的根
	*/
	class __CORE_COMMON_API__ CBaseObject :
		public base::noncopyable
	{
	public:
		virtual ~CBaseObject() { }

		virtual const char*	getClassName() const = 0;
		virtual uint32_t	getClassID() const = 0;
		
		virtual void		release() = 0;

		static void			registerClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, const funCreateBaseObject& fnCreateBaseObject, const funDestroyBaseObject& fnDestroyBaseObject);
		
		static SClassInfo*	getClassInfo(uint32_t nClassID);
		static uint32_t		getClassID(const std::string& szClassName);
		
		static CBaseObject*	createObject(const std::string& szClassName);
		static CBaseObject*	createObject(uint32_t nClassID);
		static void			destroyObject(CBaseObject* pBaseObject);
	};
}

#define _GET_CLASS_NAME(Class) #Class

#define _GET_CLASS_ID(Class) base::hash(_GET_CLASS_NAME(Class))

#define DECLARE_OBJECT(Class)\
public:\
	static const char*	s_szClassName;\
	static uint32_t		s_nClassID;\
	\
	friend core::CBaseObject*	Create##Class(void* pBuf);\
	friend void					Destroy##Class(core::CBaseObject* pObject);\
	virtual const char*			getClassName() const;\
	virtual uint32_t			getClassID() const;\
	static void					registerClassInfo();

#define DEFINE_OBJECT(Class, nBatchCount)\
	const char* Class::s_szClassName = _GET_CLASS_NAME(Class);\
	uint32_t Class::s_nClassID = _GET_CLASS_ID(Class);\
	core::CBaseObject*	create##Class(void* pBuf)\
	{\
		return new(pBuf)Class();\
	}\
	void destroy##Class(core::CBaseObject* pObject)\
	{\
		Class* pClass = static_cast<Class*>(pObject);\
		pClass->~Class();\
	}\
	const char* Class::getClassName() const\
	{\
		return s_szClassName;\
	}\
	uint32_t Class::getClassID() const\
	{\
		return s_nClassID;\
	}\
	void Class::registerClassInfo()\
	{\
		auto fnCreateBaseObject = std::bind(&create##Class, std::placeholders::_1);\
		auto fnDestroyBaseObject = std::bind(&destroy##Class, std::placeholders::_1);\
		core::CBaseObject::registerClassInfo(s_szClassName, sizeof(Class), nBatchCount, fnCreateBaseObject, fnDestroyBaseObject);\
	}
