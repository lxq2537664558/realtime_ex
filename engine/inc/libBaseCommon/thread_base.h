#pragma once

#include "noncopyable.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace base
{
	class CThreadBase;
	class __BASE_COMMON_API__ IRunnable
	{
	public:
		virtual ~IRunnable() { }

		virtual bool	onInit() { return true; }
		virtual bool	onProcess() = 0;
		virtual void	onDestroy() { }
	};

	class __BASE_COMMON_API__ CThreadBase :
		public noncopyable
	{
	public:
		void				quit();
		uint32_t			isQuit() const;
		void				join();
		uint32_t			getID() const;
		IRunnable*			getRunnable() const;

		void				release();

		static uint32_t		getCurrentID();

		static CThreadBase*	createNew(IRunnable* pRunnable);

	private:
		CThreadBase();
		~CThreadBase();

#ifdef _WIN32
		static uint32_t	__stdcall	threadProc(void* pContext);
#else
		static void*				threadProc(void* pContext);
#endif

		bool				init(IRunnable* pRunnable);

	protected:
		volatile uint32_t	m_bQuit;
		IRunnable*			m_pRunnable;
		uint32_t			m_nThreadID;

#ifdef _WIN32
		HANDLE				m_hThread;
#else
		pthread_t			m_hThread;
#endif
	};
}