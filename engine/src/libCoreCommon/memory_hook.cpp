#include "stdafx.h"
#include "core_common.h"

#include "libBaseCommon\base_time.h"
#include "libBaseCommon\spin_mutex.h"

#include <dbghelp.h>
#include <stdio.h>
#include <share.h>

#ifdef _WIN32
#pragma comment( lib, "dbghelp.lib" )
#endif

#define __MEMORY_HOOK__
#define _MEMORY_DETAIL_STACK_COUNT 10

namespace core
{
	struct SMemoryHookInfo
	{
		size_t		nSize;		// 分配的内存大小
		char*		pContext;	// 内存分配函数地址
		uint64_t	nAllocTime;	// 分配时间
		bool		bDetail;	// 是否是详细信息
	};

	typedef base::STinyListNode<SMemoryHookInfo> SMemoryHookInfoNode;

	class CMemoryHookMgr
	{
	public:
		CMemoryHookMgr();
		~CMemoryHookMgr();

		static CMemoryHookMgr* Inst();

		void*	allocate(size_t nSize, void* pCallAddr);
		void	deallocate(void* pData);
		void	beginLeakChecker(bool bDetail);
		void	endLeakChecker(const char* szName);

	private:
		base::CTinyList<SMemoryHookInfoNode>	m_listMemoryHookInfo;
		bool									m_bCheck;
		bool									m_bDetail;
		base::spin_mutex						m_lock;
	};

	CMemoryHookMgr::CMemoryHookMgr()
		: m_bCheck(false)
		, m_bDetail(false)
	{
	}

	CMemoryHookMgr::~CMemoryHookMgr()
	{

	}

	CMemoryHookMgr* CMemoryHookMgr::Inst()
	{
		static CMemoryHookMgr s_Inst;

		return &s_Inst;
	}

	void* CMemoryHookMgr::allocate(size_t nSize, void* pCallAddr)
	{
		void* pData = malloc(nSize + sizeof(SMemoryHookInfoNode));
		SMemoryHookInfoNode* pMemoryHookInfoNode = reinterpret_cast<SMemoryHookInfoNode*>(pData);
		pMemoryHookInfoNode->Value.nSize = nSize;
		if (this->m_bDetail)
		{
			pMemoryHookInfoNode->Value.pContext = reinterpret_cast<char*>(malloc(_MEMORY_DETAIL_STACK_COUNT * sizeof(void*)));
			base::getStack(1, 10, reinterpret_cast<void**>(pMemoryHookInfoNode->Value.pContext), _MEMORY_DETAIL_STACK_COUNT);
		}
		else
		{
			pMemoryHookInfoNode->Value.pContext = reinterpret_cast<char*>(pCallAddr);
		}
		pMemoryHookInfoNode->pNext = nullptr;
		pMemoryHookInfoNode->pPre = nullptr;

		if (this->m_bCheck)
		{
			pMemoryHookInfoNode->Value.nAllocTime = base::getGmtTime();

			this->m_lock.lock();
			this->m_listMemoryHookInfo.pushBack(pMemoryHookInfoNode);
			this->m_lock.unlock();
		}
		else
		{
			pMemoryHookInfoNode->Value.nAllocTime = 0;
		}

		return pMemoryHookInfoNode + 1;
	}

	void CMemoryHookMgr::deallocate(void* pData)
	{
		if (nullptr == pData)
			return;

		SMemoryHookInfoNode* pMemoryHookInfoNode = reinterpret_cast<SMemoryHookInfoNode*>(pData)-1;
		if (pMemoryHookInfoNode->Value.bDetail)
			free(pMemoryHookInfoNode->Value.pContext);

		if (pMemoryHookInfoNode->Value.nAllocTime != 0)
		{
			this->m_lock.lock();
			pMemoryHookInfoNode->remove();
			this->m_lock.unlock();
		}

		free(pMemoryHookInfoNode);
	}

	void CMemoryHookMgr::beginLeakChecker(bool bDetail)
	{
		this->m_bCheck = true;
		this->m_bDetail = bDetail;
	}

	void CMemoryHookMgr::endLeakChecker(const char* szName)
	{
		if (nullptr == szName)
			return;

#ifdef _WIN32
		FILE* pFile = _fsopen(szName, "w", _SH_DENYNO);
#else
		FILE* pFile = fopen(szName, "w");
#endif
		if (pFile == nullptr)
			return;

		auto saveInfo = [](void* pAddr, size_t nSize, uint64_t nAllocTime, FILE* pFile)->void
		{
#if defined(WIN32)
			char szSymbolBuf[sizeof(PIMAGEHLP_SYMBOL) + 1024];
			PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)szSymbolBuf;
			pSymbol->SizeOfStruct = sizeof(szSymbolBuf);
			pSymbol->MaxNameLength = 1024;
			DWORD64 dwSymDisplacement = 0;

			if (SymGetSymFromAddr64(GetCurrentProcess(), (DWORD64)pAddr, 0, pSymbol))
			{
				char szTime[64] = { 0 };
				base::formatGmtTime(szTime, nAllocTime);
				char szInfo[1024] = { 0 };
				size_t nCount = base::crt::snprintf(szInfo, _countof(szInfo), "%s %s [0x%x] size: %d\r\n", szTime, pSymbol->Name, pSymbol->Address, nSize);
				fwrite(szInfo, 1, nCount, pFile);
			}
#else
			char** pSymbol = NULL;
			pSymbol = (char**)backtrace_symbols(&pAddr, 1);
			if (pSymbol != NULL && pSymbol[0] != NULL)
			{
				char szTime[64] = { 0 };
				base::formatGmtTime(szTime, nAllocTime);
				char szInfo[1024] = { 0 };
				size_t nCount = base::crt::snprintf(szInfo, _countof(szInfo), "%s %s [0x%x] size: %d\r\n", szTime, pSymbol[0], pAddr, nSize);
				fwrite(szInfo, 1, nCount, pFile);
			}

			if (pSymbol != NULL)
				free(pSymbol);
#endif
		};

		this->m_lock.lock();
		while (!this->m_listMemoryHookInfo.isEmpty())
		{
			SMemoryHookInfoNode* pNode = this->m_listMemoryHookInfo.getFront();
			pNode->remove();

			if (pNode->Value.bDetail)
			{
				for (size_t i = 0; i < _MEMORY_DETAIL_STACK_COUNT; ++i)
				{
					void* pAddr = reinterpret_cast<void*>(pNode->Value.pContext[i]);
					saveInfo(pAddr, pNode->Value.nSize, pNode->Value.nAllocTime, pFile);
				}
			}
			else
			{
				saveInfo(pNode->Value.pContext, pNode->Value.nSize, pNode->Value.nAllocTime, pFile);
			}
		}
		this->m_lock.unlock();
		fclose(pFile);

		this->m_bCheck = false;
		this->m_bDetail = false;
	}

	void beginMemoryLeakChecker(bool bDetail)
	{
#ifdef __MEMORY_HOOK__
		core::CMemoryHookMgr::Inst()->beginLeakChecker(bDetail);
#endif
	}

	void endMemoryLeakChecker(const char* szName)
	{
#ifdef __MEMORY_HOOK__
		core::CMemoryHookMgr::Inst()->endLeakChecker(szName);
#endif
	}
}

#ifdef __MEMORY_HOOK__

void* operator new (size_t size)
{
	return core::CMemoryHookMgr::Inst()->allocate(size, ((void**)&size)[-1]);
}

void* operator new[](size_t size)
{
	return core::CMemoryHookMgr::Inst()->allocate(size, ((void**)&size)[-1]);
}

void operator delete (void* pData)
{
	core::CMemoryHookMgr::Inst()->deallocate(pData);
}

void operator delete[](void* pData)
{
	core::CMemoryHookMgr::Inst()->deallocate(pData);
}

#endif