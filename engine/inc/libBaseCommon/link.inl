#include "debug_helper.h"

namespace base
{
	template<class T>
	TLinkNode<T>::TLinkNode()
	{
		this->pNext = nullptr;
		this->pPre = nullptr;
	}

	template<class T>
	TLinkNode<T>::~TLinkNode()
	{
		this->remove();
	}

	template<class T>
	void TLinkNode<T>::remove()
	{
		if (this->pNext != nullptr && this->pPre != nullptr)
		{
			this->pPre->pNext = this->pNext;
			this->pNext->pPre = this->pPre;
		}
		this->pPre = nullptr;
		this->pNext = nullptr;
	}

	template<class T>
	bool TLinkNode<T>::isLink() const
	{
		return this->pNext != nullptr && this->pPre != nullptr;
	}

	template<class T>
	TLink<T>::TLink()
	{
		this->m_head.pNext = &this->m_tail;
		this->m_tail.pPre = &this->m_head;

		this->m_head.pPre = nullptr;
		this->m_tail.pNext = nullptr;
	}

	template<class T>
	TLink<T>::~TLink()
	{
		// nothing
	}

	template<class T>
	bool TLink<T>::pushTail(T *pNode)
	{
		DebugAstEx(pNode != nullptr, false);

		if (pNode->isLink())
			return false;

		pNode->pNext = &this->m_tail;
		pNode->pPre = this->m_tail.pPre;
		this->m_tail.pPre->pNext = pNode;
		this->m_tail.pPre = pNode;

		return true;
	}

	template<class T>
	T* TLink<T>::getTail()
	{
		if (this->empty())
			return nullptr;

		return this->m_tail.pPre;
	}

	template<class T>
	bool TLink<T>::pushHead(T *pNode)
	{
		DebugAstEx(pNode != nullptr, false);

		if (pNode->isLink())
			return false;

		pNode->pPre = &this->m_head;
		pNode->pNext = this->m_head.pNext;
		this->m_head.pNext->pPre = pNode;
		this->m_head.pNext = pNode;

		return true;
	}

	template<class T>
	T* TLink<T>::getHead()
	{
		if (this->empty())
			return nullptr;

		return this->m_head.pNext;
	}

	template<class T>
	bool TLink<T>::insert(T* pWhere, T* pNode)
	{
		DebugAstEx(pNode != nullptr && pWhere != nullptr, false);

		if (pNode->isLink() || !pWhere->isLink())
			return false;

		pNode->pNext = pWhere->pNext;
		pNode->pPre = pWhere;
		pWhere->pNext->pPre = pNode;
		pWhere->pNext = pNode;

		return true;
	}

	template<class T>
	bool TLink<T>::empty() const
	{
		return this->m_head.pNext == &this->m_tail;
	}

	template<class T>
	void TLink<T>::splice(TLink<T>& rhs)
	{
		rhs.m_head = this->m_head;
		rhs.m_tail = this->m_tail;

		this->m_head.pNext = &this->m_tail;
		this->m_tail.pPre = &this->m_head;

		this->m_head.pPre = nullptr;
		this->m_tail.pNext = nullptr;
	}
}