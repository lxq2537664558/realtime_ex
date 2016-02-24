#include "debug_helper.h"

namespace base
{
	template<class T>
	STinyListNode<T>::STinyListNode()
	{
		this->pNext = nullptr;
		this->pPre = nullptr;
		this->Value = T();
	}

	template<class T>
	STinyListNode<T>::~STinyListNode()
	{
		this->remove();
	}

	template<class T>
	void STinyListNode<T>::remove()
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
	bool STinyListNode<T>::isHang() const
	{
		return this->pNext != nullptr && this->pPre != nullptr;
	}

	template<class NodeType>
	CTinyList<NodeType>::CTinyList()
	{
		this->m_head.pNext = &this->m_tail;
		this->m_tail.pPre = &this->m_head;

		this->m_head.pPre = nullptr;
		this->m_tail.pNext = nullptr;
	}

	template<class NodeType>
	CTinyList<NodeType>::~CTinyList()
	{
		// nothing
	}

	template<class NodeType>
	bool CTinyList<NodeType>::pushBack(NodeType *pNode)
	{
		DebugAstEx(pNode != nullptr, false);

		if (pNode->isHang())
			return false;

		pNode->pNext = &this->m_tail;
		pNode->pPre = this->m_tail.pPre;
		this->m_tail.pPre->pNext = pNode;
		this->m_tail.pPre = pNode;

		return true;
	}

	template<class NodeType>
	NodeType* CTinyList<NodeType>::getBack()
	{
		if (this->isEmpty())
			return nullptr;

		return this->m_tail.pPre;
	}

	template<class NodeType>
	bool CTinyList<NodeType>::pushFront(NodeType *pNode)
	{
		DebugAstEx(pNode != nullptr, false);

		if (pNode->isHang())
			return false;

		pNode->pPre = &this->m_head;
		pNode->pNext = this->m_head.pNext;
		this->m_head.pNext->pPre = pNode;
		this->m_head.pNext = pNode;

		return true;
	}

	template<class NodeType>
	NodeType* CTinyList<NodeType>::getFront()
	{
		if (this->isEmpty())
			return nullptr;

		return this->m_head.pNext;
	}

	template<class NodeType>
	bool CTinyList<NodeType>::isEmpty() const
	{
		return this->m_head.pNext == &this->m_tail;
	}

	template<class NodeType>
	void CTinyList<NodeType>::splice(CTinyList<NodeType>& rhs)
	{
		rhs.m_head = this->m_head;
		rhs.m_tail = this->m_tail;

		this->m_head.pNext = &this->m_tail;
		this->m_tail.pPre = &this->m_head;

		this->m_head.pPre = nullptr;
		this->m_tail.pNext = nullptr;
	}
}