#include "serialize_adapter.h"

namespace core
{
	class CNativeSerializeAdapter :
		public CSerializeAdapter
	{
	public:
		CNativeSerializeAdapter();
		virtual ~CNativeSerializeAdapter();

		virtual message_header*		serialize(const void* pData, void* pBuf, uint16_t nBufSize);
		virtual CMessagePtr<char>	deserialize(const message_header* pData);
	};
}