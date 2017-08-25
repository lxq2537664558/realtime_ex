#include "stdafx.h"
#include "base64.h"
#include <iostream>
#include <string>

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(uint8_t c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

namespace base
{
	namespace base64
	{
		int32_t encode(const char* szData, uint32_t nLen, char* szBuf, uint32_t nBufSize)
		{
			if (nullptr == szBuf || szData == nullptr)
				return -1;

			std::string ret;
			int32_t i = 0;
			int32_t j = 0;
			unsigned char char_array_3[3];
			unsigned char char_array_4[4];

			while (nLen--)
			{
				char_array_3[i++] = *(szData++);
				if (i == 3)
				{
					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
					char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (i = 0; i < 4; i++)
					{
						ret += base64_chars[char_array_4[i]];
					}
					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 3; j++)
				{
					char_array_3[j] = '\0';
				}

				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (j = 0; j < i + 1; j++)
				{
					ret += base64_chars[char_array_4[j]];
				}

				while ((i++ < 3))
				{
					ret += '=';
				}
			}

			if (ret.size() > nBufSize || ret.empty())
				return -1;

			memcpy(szBuf, &ret[0], ret.size());

			return (int32_t)ret.size();
		}

		int32_t decode(const char* szData, uint32_t nLen, char* szBuf, uint32_t nBufSize)
		{
			if (nullptr == szBuf || szData == nullptr)
				return -1;

			std::string ret;

			int32_t i = 0;
			int32_t j = 0;
			int32_t pos = 0;
			unsigned char char_array_4[4], char_array_3[3];

			while (nLen-- && (szData[pos] != '=') && is_base64(szData[pos]))
			{
				char_array_4[i++] = szData[pos];
				pos++;
				if (i == 4)
				{
					for (i = 0; i < 4; ++i)
					{
						char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);
					}

					char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
					char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
					char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

					for (i = 0; i < 3; ++i)
					{
						ret += char_array_3[i];
					}

					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 4; j++)
				{
					char_array_4[j] = 0;
				}

				for (j = 0; j < 4; j++)
				{
					char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);
				}

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (j = 0; j < i - 1; j++)
				{
					ret += char_array_3[j];
				}
			}

			if (ret.size() > nBufSize || ret.empty())
				return -1;

			memcpy(szBuf, &ret[0], ret.size());

			return (int32_t)ret.size();
		}
	}
}