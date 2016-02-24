#pragma once
#include "db_binder.h"

namespace base
{
	class CDbResultBinder :
		public CDbBinder,
		public IDbResultBinder
	{
	public:
		CDbResultBinder(CDbStatement* pDbStatement);
		~CDbResultBinder();

		virtual void bindInt64(uint32_t nIndex, int64_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindUInt64(uint32_t nIndex, uint64_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindInt32(uint32_t nIndex, int32_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindUInt32(uint32_t nIndex, uint32_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindInt16(uint32_t nIndex, int16_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindUInt16(uint32_t nIndex, uint16_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindInt8(uint32_t nIndex, int8_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindUInt8(uint32_t nIndex, uint8_t& nValue, bool& bIsNull, bool& bError);
		virtual void bindFloat(uint32_t nIndex, float& fValue, bool& bIsNull, bool& bError);
		virtual void bindDouble(uint32_t nIndex, double& fValue, bool& bIsNull, bool& bError);
		virtual void bindBlob(uint32_t nIndex, void* pBuf, uint32_t nBufSize, _ulong& nResultSize, bool& bIsNull, bool& bError);
		virtual void bindText(uint32_t nIndex, void* pBuf, uint32_t nBufSize, _ulong& nResultSize, bool& bIsNull, bool& bError);

		virtual void bind();
	};
}