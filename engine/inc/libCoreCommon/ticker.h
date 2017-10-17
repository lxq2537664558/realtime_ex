#pragma once

#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/noninheritable.h"

#include "core_common.h"

#include <functional>


namespace core
{
	class CTickerRunnable;
	class CLogicRunnable;
	class CNetRunnable;
	
	// ��ʱ������֧��ֱ�ӿ�������Ϊֱ�ӿ���ʱ����ж�ʱ��Ϊ�������Ϊ�Ǵ���ȥ�ػ��ǿ���һ���յĶ�ʱ���أ�
	// ���ﶨʱ������Ŀ���ֻ��ͨ����move�ķ�ʽ
	// ��Ȼ��Ҳ������ָ�����ʽ����CTicker���󣬵�������һ����Ҫ���Լ���������ڴ棬�������ɾ������ʱ���������ܵģ��ܿ��ܾͱ�����
	// ���ｨ����ָ�����ʽ����CTicker������std::unique_ptr����
	class __CORE_COMMON_API__ CTicker :
		public virtual base::noninheritable<CTicker>,
		public base::noncopyable
	{
		friend class CTickerRunnable;
		friend class CLogicRunnable;
		friend class CNetRunnable;
		
	public:
		CTicker();
		~CTicker();

		CTicker(CTicker&& rhs);
		CTicker& operator = (CTicker&& rhs);

		int64_t		getIntervalTime() const;
		bool		isRegister() const;
		uint64_t	getContext() const;
		void		setCallback(const std::function<void(uint64_t)>& callback);
		std::function<void(uint64_t)>&
					getCallback();

	private:
		void*							m_pCoreContext;		// �������ֻ�����߼��߳�ȥ��
		int64_t							m_nIntervalTime;	// ��ʱ�����еļ��ʱ��
		uint64_t						m_nContext;
		std::function<void(uint64_t)>*	m_callback;
	};
}