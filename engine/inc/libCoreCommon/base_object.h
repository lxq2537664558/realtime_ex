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
	@brief: �߼������࣬�����п�ܶ���ĸ�
	*/
	class CBaseObject :
		public base::noncopyable
	{
	public:
		CBaseObject();
		virtual ~CBaseObject();

		/**
		@brief: �ж϶����Ƿ��Ѿ���ɾ����
		*/
		bool				isDel() const { return this->m_bDel; }
		virtual const char*	getClassName() const = 0;
		virtual uint32_t	getClassID() const = 0;
		/**
		@brief: ɾ������ֻ�Ǵ���ɾ����ǣ�
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
		bool m_bDel;	// ���Del�����Ҫ���������ĳһ�����������Ƿ���Ҫ��ɾ������ô��Ϊ��ɾ��������һ���ط�ͳһ���У��ڵ�ǰ������ֻ�Ǵ�һ����ǣ��������Ա���ܶ�Ұָ�����⣬�����ڱ�д���ƻ�ȡ����ĺ���ʱ��Ҫ�����bDel��ǿ��ǽ�ȥ
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
	}\
	Class::registClassInfo()
