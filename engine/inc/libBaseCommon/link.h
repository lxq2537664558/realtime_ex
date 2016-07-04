#pragma once
#include "base_common.h"

namespace base
{
	template<class T>
	struct TLinkNode
	{
		TLinkNode* pPre;
		TLinkNode* pNext;
		T          Value;

		TLinkNode();
		~TLinkNode();

		void remove();
		bool isLink() const;
	};

	template<class T>
	class TLink
	{
	public:
		TLink();
		~TLink();

		bool	pushTail(T* pNode);
		T*		getTail();

		bool	pushHead(T* pNode);
		T*		getHead();

		bool	insert(T* pWhere, T* pNode);

		bool	empty() const;

		void	splice(TLink<T>& rhs);

	private:
		T m_head;
		T m_tail;
	};

}

#include "link.inl"