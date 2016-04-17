#pragma once
#include "base_common.h"

namespace base
{
	template<class T>
	struct STinyListNode
	{
		STinyListNode* pPre;
		STinyListNode* pNext;
		T              Value;

		STinyListNode();
		~STinyListNode();

		void remove();
		bool isHang() const;
	};

	template<class NodeType>
	class CTinyList
	{
	public:
		CTinyList();
		~CTinyList();

		bool		pushBack(NodeType* pNode);
		NodeType*	getBack();

		bool		pushFront(NodeType* pNode);
		NodeType*	getFront();

		bool		isEmpty() const;

		void		splice(CTinyList<NodeType>& rhs);

	private:
		NodeType m_head;
		NodeType m_tail;
	};

}

#include "tiny_list.inl"