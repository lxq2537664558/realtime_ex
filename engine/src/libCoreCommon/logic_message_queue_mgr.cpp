#include "logic_message_queue_mgr.h"

#include "libBaseCommon/profiling.h"
#include "libBaseCommon/debug_helper.h"

namespace core
{

	CLogicMessageQueueMgr::CLogicMessageQueueMgr()
	{

	}

	CLogicMessageQueueMgr::~CLogicMessageQueueMgr()
	{

	}

	CLogicMessageQueue* CLogicMessageQueueMgr::getMessageQueue()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		while (this->m_listMessageQueue.empty())
		{
			this->m_cond.wait(guard);
		}
		
		CLogicMessageQueue* pMessageQueue = this->m_listMessageQueue.front();
		this->m_listMessageQueue.pop_front();
		
		return pMessageQueue;
	}

	void CLogicMessageQueueMgr::putMessageQueue(CLogicMessageQueue* pMessageQueue)
	{
		DebugAst(pMessageQueue != nullptr);

		std::unique_lock<std::mutex> guard(this->m_lock);

		this->m_listMessageQueue.push_back(pMessageQueue);
		
		// ��notify��unlock��˳��û���⣬���Ҳ�������ȴ��߳����������ò������ٴ�˯�ߵ����⣬�������һ���ȴ�ת�ƵĹ��̣�ֱ�Ӱ��̵߳ȴ������ȴ��б���
		// �����unlock��notify��������Ƚ϶��α�������⣬��Ȼ�߼���ȷ��Ҳ��û�����

		// ���ﲻ����Ҫ����Ŀ��Ʊ���������notify��������Ϊ���û���߳��ڵȴ����������ɶҲ������������������Լ��ӱ�ʶһ��
		this->m_cond.notify_one();
	}
}