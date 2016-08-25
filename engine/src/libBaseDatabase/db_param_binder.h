#pragma once
#include "db_binder.h"

namespace base
{
	class CDbParamBinder :
		public CDbBinder,
		public IDbParamBinder
	{
	public:
		explicit CDbParamBinder(CDbStatement* pDbStatement);
		virtual ~CDbParamBinder();

		virtual void setInt64(uint32_t nIndex, int64_t& nValue);
		virtual void setUInt64(uint32_t nIndex, uint64_t& nValue);
		virtual void setInt32(uint32_t nIndex, int32_t& nValue);
		virtual void setUInt32(uint32_t nIndex, uint32_t& nValue);
		virtual void setInt16(uint32_t nIndex, int16_t& nValue);
		virtual void setUInt16(uint32_t nIndex, uint16_t& nValue);
		virtual void setInt8(uint32_t nIndex, int8_t& nValue);
		virtual void setUInt8(uint32_t nIndex, uint8_t& nValue);
		virtual void setFloat(uint32_t nIndex, float& fValue);
		virtual void setDouble(uint32_t nIndex, double& fValue);
		virtual void setBlob(uint32_t nIndex, void* pBuf, uint32_t nBufSize);
		virtual void setText(uint32_t nIndex, void* pBuf, uint32_t nBufSize);

		virtual void bind();
	};
}