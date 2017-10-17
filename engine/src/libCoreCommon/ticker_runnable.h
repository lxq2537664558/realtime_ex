#pragma once

#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/spin_lock.h"
#include "libBaseCommon/link.h"
#include "libBaseCommon/singleton.h"

#include "ticker.h"
#include "message_queue.h"

#include <memory>
#include <vector>

namespace core
{
	enum ETickerState
	{
		eRegister,
		eUnRegister,
	};

	class CTickerRunnable;
	class CNetRunnable;
	class CLogicRunnable;
	class CTicker;
	class CCoreTickerInfo
	{
		friend class CTickerRunnable;
		friend class CNetRunnable;
		friend class CLogicRunnable;
		friend class CTicker;

	public:
		CCoreTickerInfo();
		~CCoreTickerInfo();

		CCoreTickerInfo(const CCoreTickerInfo& rhs) = delete;
		const CCoreTickerInfo& operator = (const CCoreTickerInfo& rhs) = delete;

		void	addRef();
		void	release();
		int32_t	getRef() const;

	private:
		CTicker*				m_pTicker;			// ��������ķ��ʸ��޸Ķ����ڷ���ʱ�����߳��У�����û�о�������
		void*					m_pMemory;
		int64_t					m_nNextTime;		// ��һ�ζ�ʱ������ʱ��
		int64_t					m_nIntervalTime;	// ��ʱ�����еļ��ʱ��
		CMessageQueue*			m_pMessageQueue;
		int32_t					m_nState;			// ��������ķ��ʸ��޸Ķ����ڷ���ʱ�����߳��У�����û�о�������
		std::atomic<int32_t>	m_nRef;
	};

	typedef base::TLinkNode<CCoreTickerInfo> CCoreTickerNode;

	class CTickerRunnable :
		public base::IRunnable,
		public base::CSingleton<CTickerRunnable>
	{
	private:
		enum
		{
			__TIME_NEAR_BITS = 16,
			__TIME_CASCADE_BITS = 8,
			__TIME_CASCADE_COUNT = 3,
			__TIME_NEAR_SIZE = 1 << __TIME_NEAR_BITS,
			__TIME_NEAR_MASK = __TIME_NEAR_SIZE - 1,
			__TIME_CASCADE_SIZE = 1 << __TIME_CASCADE_BITS,
			__TIME_CASCADE_MASK = __TIME_CASCADE_SIZE - 1,
		};

	public:
		CTickerRunnable();
		virtual ~CTickerRunnable();

		bool			init();

		bool			registerTicker(CMessageQueue* pMessageQueue, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void			unregisterTicker(CTicker* pTicker);
		
	private:
		virtual bool	onInit();
		virtual bool	onProcess();
		virtual void	onDestroy();

		void			update(int64_t nTime);

		void			insertTicker(CCoreTickerNode* pCoreTickerNode);
		void			cascadeTicker();
		void			onTicker(CCoreTickerNode* pCoreTickerNode);

	private:
		base::CThreadBase*				m_pThreadBase;
		base::TLink<CCoreTickerNode>	m_listNearTicker[__TIME_NEAR_SIZE];								// ������е���ʱ��̶�
		base::TLink<CCoreTickerNode>	m_listCascadeTicker[__TIME_CASCADE_COUNT][__TIME_CASCADE_SIZE];	// ����ʱ��̶�
		base::TLink<CCoreTickerNode>	m_listFarTicker;												// ��Զ�Ķ�ʱ������
		std::vector<CCoreTickerNode*>	m_vecRegisterTicker;											// ˫���н�����ע�ᶨʱ��
		base::spin_lock					m_lockRegister;
		std::vector<CCoreTickerNode*>	m_vecUnRegisterTicker;											// ˫���н����ķ�ע�ᶨʱ��
		base::spin_lock					m_lockUnRegister;

		std::vector<CCoreTickerNode*>	m_vecTempTickerNode;
		int64_t							m_nLogicTime;													// ��ǰ�̶�ʱ��
	};
}