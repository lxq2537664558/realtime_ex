#pragma once

#include <vector>
#include <functional>
#include <algorithm>

namespace base
{
	template<class _Kty,
	class _Ty,
	class _Pr = std::less<_Kty>,
	class _Alloc = std::allocator<std::pair<const _Kty, _Ty> > >
	class vector_map :
		public std::vector<std::pair<_Kty, _Ty>, _Alloc>
	{
	public:
		typedef vector_map<_Kty, _Ty, _Pr, _Alloc>									my_type;
		typedef _Kty																key_type;
		typedef _Ty																	mapped_type;
		typedef _Pr																	key_compare;
		typedef std::pair<_Kty, _Ty>												value_type;
		typedef typename std::vector<std::pair<_Kty, _Ty>, _Alloc>::iterator		iterator;
		typedef typename std::vector<std::pair<_Kty, _Ty>, _Alloc>::const_iterator	const_iterator;

	public:
		const_iterator find(const_iterator itBegin, const_iterator itEnd, const key_type& key) const
		{
			const_iterator iter = std::lower_bound(itBegin, itEnd, key, compare(key_compare()));
			if (iter != itEnd && !compare(key_compare())(*iter, key) && !compare(key_compare())(key, *iter))
				return iter;

			return itEnd;
		}

		const_iterator find(const key_type& key) const
		{
			return this->find(this->begin(), this->end(), key);
		}

		void sort()
		{
			std::sort(this->begin(), this->end(), compare(key_compare()));
		}

	private:
		struct compare
		{
			key_compare& m_comp;

			compare(key_compare& comp) : m_comp(comp) {}

			bool operator() (const value_type& lhs, const value_type& rhs)
			{
				return this->m_comp(lhs.first, rhs.first);
			}

			bool operator() (const value_type& lhs, typename const value_type::first_type& k)
			{
				return this->m_comp(lhs.first, k);
			}

			bool operator() (typename const value_type::first_type& k, const value_type& rhs)
			{
				return this->m_comp(k, rhs.first);
			}
		};
	};
}