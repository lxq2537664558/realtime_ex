#pragma once

#include "noncopyable.h"
#include "noninheritable.h"

#include <functional>

namespace base
{
	class CTickerMgrImpl;
	
	// ��ʱ������֧��ֱ�ӿ�������Ϊֱ�ӿ���ʱ����ж�ʱ��Ϊ�������Ϊ�Ǵ���ȥ�ػ��ǿ���һ���յĶ�ʱ���أ�
	// ���ﶨʱ������Ŀ���ֻ��ͨ����move�ķ�ʽ��ʽ�Ŀ���
	// ��Ȼ��Ҳ������ָ�����ʽ����CTicker���󣬵�������һ����Ҫ���Լ���������ڴ棬�������ɾ������ʱ���������ܵģ��ܿ��ܾͱ�����
	// ���ｨ����ָ�����ʽ����CTicker������std::unique_ptr����
	class __BASE_COMMON_API__ CTicker :
		public virtual noninheritable<CTicker>,
		public noncopyable
	{
		friend class CTickerMgrImpl;
		
	public:
		CTicker();
		~CTicker();

		CTicker(CTicker&& rhs);
		CTicker& operator = (CTicker&& rhs);

		int64_t		getIntervalTime() const;
		bool		isRegister() const;
		uint64_t	getContext() const;
		void		setCallback(const std::function<void(uint64_t)>& callback, bool bCoroutine = false);
		std::function<void(uint64_t)>&
					getCallback();
		bool		isCoroutine() const;

	private:
		void*							m_pCoreContext;		// �������ֻ�����߼��߳�ȥ��
		int64_t							m_nIntervalTime;	// ��ʱ�����еļ��ʱ��
		uint64_t						m_nContext;
		std::function<void(uint64_t)>*	m_callback;
		bool							m_bCoroutine;
	};

	class __BASE_COMMON_API__ CTickerMgr :
		public virtual noninheritable<CTickerMgr>,
		public noncopyable
	{
	public:
		CTickerMgr(int64_t nTime, const std::function<void(CTicker*)>& callback);
		~CTickerMgr();

		int64_t	getLogicTime() const;
		void	update(int64_t nTime);

		bool	registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void	unregisterTicker(CTicker* pTicker);

	private:
		CTickerMgrImpl*	m_pTickerMgrImpl;
	};
}