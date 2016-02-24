#pragma once

#include "base_common.h"

#include <math.h>

namespace base
{
	template<class T>
	class TVector2
	{
	public:
		T x;
		T y;

	public:
		TVector2(T x, T y)
		{
			this->x = x;
			this->y = y;
		}

		TVector2()
		{
			this->x = this->y = 0;
		}

		TVector2(const TVector2& rhs)
		{
			this->x = rhs.x;
			this->y = rhs.y;
		}

		template<class U>
		TVector2(const TVector2<U>& rhs)
		{
			this->x = (T)rhs.x;
			this->y = (T)rhs.y;
		}

		template<class U>
		const TVector2& operator = (const TVector2<U>& rhs)
		{
			this->x = (T)rhs.x;
			this->y = (T)rhs.y;

			return *this;
		}

		void	normalise()
		{
			T len = (T)sqrt(float(this->x*this->x + this->y*this->y));
			if (0 == len)
			{
				return;
			}
			this->x = this->x / len;
			this->y = this->y / len;
		}

		T		length() const
		{
			return (T)sqrt(float(this->x*this->x + this->y*this->y));
		}

		bool operator == (const TVector2& rhs) const
		{
			return this->x == rhs.x && this->y == rhs.y;
		}

		bool operator != (const TVector2& rhs) const
		{
			return this->x != rhs.x || this->y != rhs.y;
		}

		TVector2 operator + (const TVector2& rhs) const
		{
			return TVector2(this->x + rhs.x, this->y + rhs.y);
		}

		TVector2 operator - (const TVector2& rhs) const
		{
			return TVector2(this->x - rhs.x, this->y - rhs.y);
		}

		template<class U>
		TVector2 operator * (U Arg) const
		{
			return TVector2(this->x*Arg, this->y*Arg);
		}

		T operator * (const TVector2& rhs) const
		{
			return this->x*rhs.x + this->y*rhs.y;
		}
	};

	typedef TVector2<int32_t>		CVector2I;
	typedef TVector2<float>		CVector2f;
}