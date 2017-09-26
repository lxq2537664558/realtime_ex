#pragma once

#include "noncopyable.h"
#include "noninheritable.h"

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace base
{
	class CThreadBase;
	class __BASE_COMMON_API__ IRunnable
	{
		friend class CThreadBase;

	public:
		IRunnable() : m_pThreadBase(nullptr) { }
		virtual ~IRunnable() { }

		void			quit();
		void			join();
		void			pause();
		void			resume();

		std::thread::id	getID() const;

	protected:
		virtual bool	onInit() { return true; }
		virtual bool	onProcess() = 0;
		virtual void	onDestroy() { }

	private:
		CThreadBase*	m_pThreadBase;
	};

	enum EThreadState
	{
		eTS_None,
		eTS_Normal,
		eTS_Pause1,
		eTS_Pause2,
		eTS_Quit,
	};

	struct SThreadBaseInfo;
	class __BASE_COMMON_API__ CThreadBase :
		public noncopyable,
		public noninheritable<CThreadBase>
	{
	public:
		void		quit();
		void		join();
		void		pause();
		void		resume();

		std::thread::id	
					getID() const;
		IRunnable*	getRunnable() const;

		void		release();

		static std::thread::id	
					getCurrentID();

		static CThreadBase*	
					createNew(IRunnable* pRunnable);

	private:
		CThreadBase();
		~CThreadBase();

		bool		init(IRunnable* pRunnable);

	protected:
		IRunnable*					m_pRunnable;
		SThreadBaseInfo*			m_pThreadBaseInfo;
	};
}