#include "stdafx.h"
#include "thread_base.h"
#include "debug_helper.h"
#include "exception_handler.h"

namespace base
{
	void IRunnable::quit()
	{
		if (this->m_pThreadBase == nullptr)
			return;

		this->m_pThreadBase->quit();
	}

	void IRunnable::join()
	{
		if (this->m_pThreadBase == nullptr)
			return;

		this->m_pThreadBase->join();
	}

	void IRunnable::pause()
	{
		if (this->m_pThreadBase == nullptr)
			return;

		this->m_pThreadBase->pause();
	}

	void IRunnable::resume()
	{
		if (this->m_pThreadBase == nullptr)
			return;

		this->m_pThreadBase->resume();
	}

	std::thread::id	 IRunnable::getID() const
	{
		if (this->m_pThreadBase == nullptr)
			return std::thread::id();

		return this->m_pThreadBase->getID();
	}

	struct SThreadBaseInfo
	{
		std::atomic<uint32_t>	m_nState;
		std::mutex				m_lock;
		std::condition_variable	m_cond;
		std::thread				m_thread;
	};

	CThreadBase::CThreadBase()
		: m_pRunnable(nullptr)
	{
		this->m_pThreadBaseInfo = new SThreadBaseInfo();
	}

	CThreadBase::~CThreadBase()
	{
		if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) != eTS_None)
		{
			this->quit();
			this->join();
		}
		
		SAFE_DELETE(this->m_pThreadBaseInfo);
	}

	bool CThreadBase::init(IRunnable* pRunnable)
	{
		DebugAstEx(pRunnable != nullptr, false);

		if (this->m_pRunnable != nullptr)
			return false;

		this->m_pRunnable = pRunnable;
		this->m_pRunnable->m_pThreadBase = this;
		
		this->m_pThreadBaseInfo->m_thread = std::thread([this]()
		{
			initThreadExceptionHander();
			
			this->m_pThreadBaseInfo->m_nState.store(eTS_Normal, std::memory_order_release);

			if (!this->m_pRunnable->onInit())
			{
				this->m_pRunnable->onDestroy();
				this->m_pThreadBaseInfo->m_nState.store(eTS_None, std::memory_order_release);
				return;
			}

			while (true)
			{
				if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) == eTS_Quit)
				{
					this->m_pThreadBaseInfo->m_cond.notify_one();
					break;
				}

				if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) == eTS_Pause1)
				{
					this->m_pThreadBaseInfo->m_lock.lock();

					if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) == eTS_Pause1)
						this->m_pThreadBaseInfo->m_nState.store(eTS_Pause2, std::memory_order_release);

					this->m_pThreadBaseInfo->m_cond.notify_one();
					this->m_pThreadBaseInfo->m_lock.unlock();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				else if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) == eTS_Pause2)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}

				if (!this->m_pRunnable->onProcess())
					break;
			}
			this->m_pRunnable->onDestroy();
			this->m_pThreadBaseInfo->m_nState.store(eTS_None, std::memory_order_release);
		});

		return true;
	}

	void CThreadBase::quit()
	{
		if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) == eTS_None)
			return;

		this->m_pThreadBaseInfo->m_nState.store(eTS_Quit, std::memory_order_release);
	}

	void CThreadBase::join()
	{
		this->m_pThreadBaseInfo->m_thread.join();
	}

	void CThreadBase::pause()
	{
		std::unique_lock<std::mutex> guard(this->m_pThreadBaseInfo->m_lock);
		if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) != eTS_Normal)
			return;

		this->m_pThreadBaseInfo->m_nState.store(eTS_Pause1, std::memory_order_release);

		this->m_pThreadBaseInfo->m_cond.wait(guard);
	}

	void CThreadBase::resume()
	{
		if (this->m_pThreadBaseInfo->m_nState.load(std::memory_order_acquire) != eTS_Pause2)
			return;

		this->m_pThreadBaseInfo->m_nState.store(eTS_Normal, std::memory_order_release);
	}

	std::thread::id CThreadBase::getID() const
	{
		return this->m_pThreadBaseInfo->m_thread.get_id();
	}

	IRunnable* CThreadBase::getRunnable() const
	{
		return this->m_pRunnable;
	}

	std::thread::id CThreadBase::getCurrentID()
	{
		return std::this_thread::get_id();
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