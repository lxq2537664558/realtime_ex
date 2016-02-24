#include "stdafx.h"
#include "thread_base.h"
#include "debug_helper.h"
#include "exception_handler.h"

#ifdef _WIN32
#include <process.h>
#else
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace base
{
#ifdef _WIN32
	uint32_t CThreadBase::threadProc(void* pContext)
#else
	void* CThreadBase::threadProc(void* pContext)
#endif
	{
		CThreadBase* pThreadBase = static_cast<CThreadBase*>(pContext);
		if (nullptr == pThreadBase)
			return 0;
		if (nullptr == pThreadBase->m_pRunnable)
			return 0;

		pThreadBase->m_nThreadID = CThreadBase::getCurrentID();

		initThreadExceptionHander();

		if (!pThreadBase->m_pRunnable->onInit())
		{
			pThreadBase->m_pRunnable->onDestroy();
			return 0;
		}
		while (!pThreadBase->m_bQuit)
		{
			if (!pThreadBase->m_pRunnable->onProcess())
				break;
		}
		pThreadBase->m_pRunnable->onDestroy();

		return 0;
	}

	CThreadBase::CThreadBase()
		: m_bQuit(false)
		, m_nThreadID(__INVALID_ID)
		, m_pRunnable(nullptr)
#ifdef _WIN32
		, m_hThread(nullptr)
#else
		, this->m_hThread(0)
#endif
	{
	}

	CThreadBase::~CThreadBase()
	{
		this->quit();
		this->join();
	}

	bool CThreadBase::init(IRunnable* pRunnable)
	{
		DebugAstEx(pRunnable != nullptr, false);

		if (this->m_pRunnable != nullptr)
			return false;

		this->m_bQuit = false;
		this->m_pRunnable = pRunnable;
		this->m_nThreadID = __INVALID_ID;

#ifdef _WIN32
		this->m_hThread = nullptr;
#else
		this->m_hThread = 0;
#endif

#ifdef _WIN32
		this->m_hThread = (HANDLE)_beginthreadex(nullptr, 0, &threadProc, this, 0, &this->m_nThreadID);
		if (this->m_hThread == NULL || this->m_hThread == INVALID_HANDLE_VALUE)
			return false;
#else
		// 这个m_hThread其实是个地址，里面包含了真正的线程id，可以用syscall(__NR_gettid)获取
		if (pthread_create(&this->m_hThread, nullptr, &threadProc, this) != 0)
			return false;
#endif

		return true;
	}

	void CThreadBase::quit()
	{
		this->m_bQuit = true;
	}

	uint32_t CThreadBase::isQuit() const
	{
		return this->m_bQuit;
	}

	void CThreadBase::join()
	{
#ifdef _WIN32
		if (this->m_hThread == nullptr)
			return;
#else
		if (this->m_hThread == 0)
			return;
#endif

#ifdef _WIN32
		::WaitForSingleObject(this->m_hThread, INFINITE);
#else
		pthread_join(this->m_hThread, nullptr);
#endif
	}

	uint32_t CThreadBase::getID() const
	{
		return this->m_nThreadID;
	}

	IRunnable* CThreadBase::getRunnable() const
	{
		return this->m_pRunnable;
	}

	uint32_t CThreadBase::getCurrentID()
	{
#ifdef _WIN32
		return ::GetCurrentThreadId();
#else
		return (uint32_t)syscall(__NR_gettid);
#endif
	}

	void CThreadBase::release()
	{
		delete this;
	}

	CThreadBase* CThreadBase::createNew(IRunnable* pRunnable)
	{
		DebugAstEx(pRunnable != nullptr, nullptr);

		CThreadBase* pThread = new CThreadBase();
		if (!pThread->init(pRunnable))
		{
			SAFE_DELETE(pThread);
			return nullptr;
		}

		return pThread;
	}
}