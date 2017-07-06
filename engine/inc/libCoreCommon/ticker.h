#pragma once

#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/noninheritable.h"

#include <atomic>
#include <functional>

namespace core
{
	// ��ʱ������֧��ֱ�ӿ�������Ϊֱ�ӿ���ʱ����ж�ʱ��Ϊ�������Ϊ�Ǵ���ȥ�ػ��ǿ���һ���յĶ�ʱ���أ�
	// ���ﶨʱ������Ŀ���ֱֻ����move�ķ�ʽ��һ���ƶ���ȥ��ԭ���Ķ�ʱ���Ͳ����иո�ע��Ķ�ʱ�������ˣ����綨ʱ���Ѿ�ע���ˣ�
	// ��Ȼ��Ҳ������ָ�����ʽ����CTicker���󣬵�������һ����Ҫ���Լ���������ڴ棬�������ɾ������ʱ���������ܵģ��ܿ��ܾͱ�����
	// ���ｨ����ָ�����ʽ����CTicker������std::unique_ptr����
	class CTicker :
		public virtual base::noninheritable<CTicker>,
		public base::noncopyable
	{
		friend class CTickerRunnable;
		
	public:
		CTicker();
		~CTicker();

		CTicker(CTicker&& rhs);
		CTicker& operator = (CTicker&& rhs);

		int64_t		getIntervalTime() const;
		bool		isRegister() const;
		uint64_t	getContext() const;
		void		setCallback(const std::function<void(uint64_t)>& callback);

	private:
		void*							m_pCoreContext;
		int64_t							m_nIntervalTime;	// ��ʱ�����еļ��ʱ��
		uint64_t						m_nContext;
		std::function<void(uint64_t)>	m_callback;
	};
}