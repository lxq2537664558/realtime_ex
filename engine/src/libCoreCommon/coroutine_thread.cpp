#include "coroutine_thread.h"
#include "coroutine_mgr.h"
#include "core_app.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/process_util.h"

namespace core
{
	CCoroutineThread::CCoroutineThread()
		: m_pCurrentCoroutine(nullptr)
		, m_pMainContext(nullptr)
	{
	}

	CCoroutineThread::~CCoroutineThread()
	{
#ifdef _WIN32
		ConvertFiberToThread();
		this->m_pMainContext = nullptr;
#else
		context* pContext = reinterpret_cast<context*>(this->m_pMainContext);
		SAFE_DELETE(pContext);
#endif
	}

	bool CCoroutineThread::init()
	{
#ifdef _WIN32
		this->m_pMainContext = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
		DebugAstEx(this->m_pMainContext != nullptr, false);
#else
		this->m_pMainContext = new context();
#endif
		this->m_pCurrentCoroutine = nullptr;

		return true;
	}

	CCoroutineImpl* CCoroutineThread::getCurrentCoroutine() const
	{
		return this->m_pCurrentCoroutine;
	}

	void CCoroutineThread::setCurrentCoroutine(CCoroutineImpl* pCoroutineImpl)
	{
		this->m_pCurrentCoroutine = pCoroutineImpl;
	}

	void CCoroutineThread::addDeadCoroutine(CCoroutineImpl* pCoroutineImpl)
	{
		DebugAst(pCoroutineImpl != nullptr);

		pCoroutineImpl->setState(eCS_DEAD);
		this->m_listDeadCoroutine.push_back(pCoroutineImpl);
	}

	void CCoroutineThread::update()
	{
		CCoroutineMgr::Inst()->update(this->m_listDeadCoroutine);
		this->m_listDeadCoroutine.clear();
	}

	void* CCoroutineThread::getMainContext() const
	{
		return this->m_pMainContext;
	}

	CCoroutineThread* getCoroutineThread()
	{
		static thread_local CCoroutineThread s_Inst;

		return &s_Inst;
	}
}