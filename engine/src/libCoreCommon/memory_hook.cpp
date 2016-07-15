#include "stdafx.h"
#include "core_common.h"

#include "libBaseCommon/base_time.h"
#include "libBaseCommon/spin_mutex.h"

#include <stdio.h>

#ifdef _WIN32
#include <share.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )
#else
#include <execinfo.h>
#endif

#define __MEMORY_HOOK__
#define _MEMORY_DETAIL_STACK_COUNT 10

namespace core
{

#pragma pack(push,1)
	struct SMemoryHookInfo
	{
		uint32_t	nSize;		// 分配的内存大小
		uint8_t		nContextCount;
		char*		pContext;	// 内存分配函数地址
		uint64_t	nAllocTime;	// 分配时间
		bool		bDetail;	// 是否是详细信息
	};

	typedef base::TLinkNode<SMemoryHookInfo> SMemoryHookInfoNode;
#pragma pack(pop)

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
		base::TLink<SMemoryHookInfoNode>	m_listMemoryHookInfo;
		bool								m_bCheck;
		bool								m_bDetail;
		base::spin_mutex					m_lock;
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
		pMemoryHookInfoNode->Value.nSize = (uint32_t)nSize;
		pMemoryHookInfoNode->Value.bDetail = this->m_bDetail;
		if (this->m_bDetail)
		{
			pMemoryHookInfoNode->Value.pContext = reinterpret_cast<char*>(malloc(_MEMORY_DETAIL_STACK_COUNT * sizeof(char*)));
			pMemoryHookInfoNode->Value.nContextCount = (uint8_t)base::getStack(3, 12, reinterpret_cast<void**>(pMemoryHookInfoNode->Value.pContext), _MEMORY_DETAIL_STACK_COUNT);
		}
		else
		{
			pMemoryHookInfoNode->Value.pContext = reinterpret_cast<char*>(pCallAddr);
			pMemoryHookInfoNode->Value.nContextCount = 1;
		}
		pMemoryHookInfoNode->pNext = nullptr;
		pMemoryHookInfoNode->pPre = nullptr;

		if (this->m_bCheck)
		{
			pMemoryHookInfoNode->Value.nAllocTime = base::getGmtTime();

			this->m_lock.lock();
			this->m_listMemoryHookInfo.pushTail(pMemoryHookInfoNode);
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

		this->m_lock.lock();
		uint32_t nTotalSize = 0;
		while (!this->m_listMemoryHookInfo.empty())
		{
			SMemoryHookInfoNode* pNode = this->m_listMemoryHookInfo.getHead();
			pNode->remove();
			nTotalSize += pNode->Value.nSize;
			if (pNode->Value.bDetail)
			{
				void** pStack = reinterpret_cast<void**>(pNode->Value.pContext);
				for (uint8_t i = 0; i < pNode->Value.nContextCount; ++i)
				{
					void* pAddr = pStack[i];
					char szBuf[1024] = { 0 };
					if (base::getFunctionInfo(pAddr, szBuf, _countof(szBuf)) <= 0)
						continue;

					char szInfo[1024] = { 0 };
					size_t nCount = base::crt::snprintf(szInfo, _countof(szInfo), "%s\r\n", szBuf);
					fwrite(szInfo, 1, nCount, pFile);
				}
				char szTime[64] = { 0 };
				base::formatGmtTime(szTime, pNode->Value.nAllocTime);
				char szInfo[1024] = { 0 };
				size_t nCount = base::crt::snprintf(szInfo, _countof(szInfo), "%s size: %d\r\n", szTime, pNode->Value.nSize);
				fwrite(szInfo, 1, nCount, pFile);
			}
			else
			{
				char szBuf[1024] = { 0 };
				if (base::getFunctionInfo(pNode->Value.pContext, szBuf, _countof(szBuf)) <= 0)
					continue;

				char szTime[64] = { 0 };
				base::formatGmtTime(szTime, pNode->Value.nAllocTime);
				char szInfo[1024] = { 0 };
				size_t nCount = base::crt::snprintf(szInfo, _countof(szInfo), "%s %s size: %d\r\n", szTime, szBuf, pNode->Value.nSize);
				fwrite(szInfo, 1, nCount, pFile);
			}
		}
		char szInfo[1024] = { 0 };
		size_t nCount = base::crt::snprintf(szInfo, _countof(szInfo), "total leak size: %d", nTotalSize);
		fwrite(szInfo, 1, nCount, pFile);

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