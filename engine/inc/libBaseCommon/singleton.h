#pragma once
#include "noncopyable.h"
#include "debug_helper.h"

namespace base
{
	template<class T>
	class CSingleton :
		public noncopyable
	{
	public:
		virtual ~CSingleton() { }
		static T*		Inst();

		virtual void	release();

	private:
		static T* s_pInst;
	};

	template<class T>
	T* CSingleton<T>::s_pInst = nullptr;

	template<class T>
	T* CSingleton<T>::Inst()
	{
		if (s_pInst == nullptr)
			s_pInst = new T();

		return s_pInst;
	}

	template<class T>
	void CSingleton<T>::release()
	{
		SAFE_DELETE(s_pInst);
	}
}