#pragma once

#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/noninheritable.h"

#include <atomic>
#include <functional>

namespace core
{
	// ��ʱ������֧��ֱ�ӿ�������Ϊֱ�ӿ���ʱ����ж�ʱ��Ϊ�������Ϊ�Ǵ���ȥ�ػ��ǿ���һ���յĶ�ʱ���أ�
	// ���ﶨʱ������Ŀ���ֻ��ͨ����move�ķ�ʽ
	// ��Ȼ��Ҳ������ָ�����ʽ����CTicker���󣬵�������һ����Ҫ���Լ���������ڴ棬�������ɾ������ʱ���������ܵģ��ܿ��ܾͱ�����
	// ���ｨ����ָ�����ʽ����CTicker������std::unique_ptr����
	class CTicker :
		public virtual base::noninheritable<CTicker>,
		public base::noncopyable
	{
		friend class CTickerRunnable;
		
	public:
		enum ETickerType
		{
			eTT_None	= 0,
			eTT_Net		= 1,
			eTT_Service	= 2,
			eTT_Actor	= 3,
		};

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

		uint8_t		getType() const;
		uint16_t	getServiceID() const;
		uint64_t	getActorID() const;

	private:
		uint8_t							m_nType;
		uint16_t						m_nServiceID;
		uint64_t						m_nActorID;
		void*							m_pCoreContext;		// �������ֻ�����߼��߳�ȥ��
		int64_t							m_nIntervalTime;	// ��ʱ�����еļ��ʱ��
		uint64_t						m_nContext;
		std::function<void(uint64_t)>	m_callback;
	};
}