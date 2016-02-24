#pragma once

#include <map>
#include <functional>

#include "libBaseCommon/base_common.h"
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

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
		funCreateBaseObject		pfCreateBaseObject;
		funDestroyBaseObject	pfDestroyBaseObject;
		CFixMemoryPool*			pFixMemoryPool;
	};

	/**
	@brief: 逻辑基础类，是所有框架对象的根
	*/
	class CBaseObject :
		public base::noncopyable
	{
	public:
		CBaseObject();
		virtual ~CBaseObject();

		/**
		@brief: 判断对象是否已经被删除了
		*/
		bool				isDel() const { return this->m_bDel; }
		virtual const char*	getClassName() const = 0;
		virtual uint32_t	getClassID() const = 0;
		/**
		@brief: 删除对象（只是打上删除标记）
		*/
		virtual void		del();


		static void			registClassInfo(const std::string& szClassName, uint32_t nObjectSize, uint32_t nBatchCount, funCreateBaseObject pfCreateBaseObject, funDestroyBaseObject pfDestroyBaseObject);
		static void			unRegistClassInfo();

		static SClassInfo*	getClassInfo(uint32_t nClassID);
		static uint32_t		getClassID(const std::string& szClassName);
		static void			destroyObject(CBaseObject* pBaseObject);
		static CBaseObject*	createObject(const std::string& szClassName);
		static CBaseObject*	createObject(uint32_t nClassID);

	private:
		static std::map<uint32_t, SClassInfo>	s_mapClassInfo;

	private:
		bool m_bDel;	// 这个Del标记主要是用来标记某一个基础对象是否需要被删除，这么做为了删除对象在一个地方统一进行，在当前流程中只是打一个标记，这样可以避免很多野指针问题，但是在编写类似获取对象的函数时需要把这个bDel标记考虑进去
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
	static void					registClassInfo();

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
	void Class::registClassInfo()\
	{\
		auto pfCreateBaseObject = std::bind(&create##Class, std::placeholders::_1);\
		auto pfDestroyBaseObject = std::bind(&destroy##Class, std::placeholders::_1);\
		core::CBaseObject::registClassInfo(s_szClassName, sizeof(Class), nBatchCount, pfCreateBaseObject, pfDestroyBaseObject);\
	}
