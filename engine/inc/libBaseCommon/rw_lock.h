#pragma once
#include "base_common.h"
#include <mutex>
#include <condition_variable>

namespace base
{
	class CRWLock
	{
	public:
		enum ELockType
		{
			eLT_Read,
			eLT_Write,
		};

		CRWLock()
		{
			this->m_nReadCount = 0;
			this->m_nWriteCount = 0;
			this->m_bWrite = false;
		}

		~CRWLock()
		{

		}

		void lock(ELockType eType)
		{
			if (eType == eLT_Write)
			{
				std::unique_lock<std::mutex> lock(this->m_mutex);
				++this->m_nWriteCount;
				this->m_condWrite.wait(lock, [=]()->bool
				{
					return this->m_nReadCount == 0 && !this->m_bWrite;
				});
				this->m_bWrite = true;
			}
			else
			{
				std::unique_lock<std::mutex> lock(this->m_mutex);
				this->m_condRead.wait(lock, [=]()->bool
				{
					return this->m_nWriteCount == 0;
				});
				++this->m_nReadCount;
			}
		}

		void unlock(ELockType eType)
		{
			if (eType == eLT_Write)
			{
				std::unique_lock<std::mutex> lock(this->m_mutex);
				if (--this->m_nWriteCount == 0)
					this->m_condRead.notify_all();
				else
					this->m_condWrite.notify_one();
				
				this->m_bWrite = false;
			}
			else
			{
				std::unique_lock<std::mutex> lock(this->m_mutex);
				if (--this->m_nReadCount == 0 && this->m_nWriteCount > 0)
				{
					this->m_condWrite.notify_one();
				}
			}
		}

	private:
		volatile size_t			m_nReadCount;
		volatile size_t			m_nWriteCount;
		volatile bool			m_bWrite;
		std::mutex				m_mutex;
		std::condition_variable m_condWrite;
		std::condition_variable m_condRead;
	};
}