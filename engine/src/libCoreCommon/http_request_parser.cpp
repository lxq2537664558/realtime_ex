#include "http_request_parser.h"

#include "libBaseCommon/function_util.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/string_util.h"

#include <algorithm>

namespace
{
	const char* szCRLF = "\r\n";
}

namespace core
{
	CHttpRequestParser::CHttpRequestParser()
		: m_eParseState(eHPS_RequestLine)
		, m_nParseDataCount(0)
	{

	}

	CHttpRequestParser::~CHttpRequestParser()
	{

	}

	CHttpRequest& CHttpRequestParser::getRequest()
	{
		return this->m_request;
	}

	bool CHttpRequestParser::parseRequestLine(const char* szData, uint32_t nDataSize)
	{
		/*
		格式 GET /xxx?aa=1&yy=6 HTTP/1.1
		*/

		const char* szEnd = szData + nDataSize;

		const char* szMethodBegin = szData;
		const char* szMethodEnd = std::find(szMethodBegin, szEnd, ' ');
		if (szMethodEnd == szEnd)
			return false;

		if (!this->m_request.setMethod(szMethodBegin))
			return false;

		const char* szURIBegin = szMethodEnd + 1;
		const char* szURIEnd = std::find(szURIBegin, szEnd, ' ');
		if (szURIEnd == szEnd)
			return false;

		const char* szQueryBegin = std::find(szURIBegin, szURIEnd, '?');
		if (szQueryBegin != szURIEnd)
		{
			this->m_request.setRequestURI(szURIBegin, (uint32_t)(szQueryBegin - szURIBegin));
			this->m_request.setQuery(szQueryBegin + 1, (uint32_t)(szURIEnd - szQueryBegin + 1));
		}
		else
		{
			this->m_request.setRequestURI(szURIBegin, (uint32_t)(szURIEnd - szURIBegin));
		}

		const char* szVersion = szURIEnd + 1;
		if (szEnd - szVersion != 8)
			return false;

		if (_strnicmp(szVersion, "HTTP/1.", 7) != 0)
			return false;

		if (*(szEnd - 1) == '1')
		{
			this->m_request.setVersion(eHV_Http11);
		}
		else if (*(szEnd - 1) == '0')
		{
			this->m_request.setVersion(eHV_Http10);
		}
		else
		{
			return false;
		}

		return true;
	}

	bool CHttpRequestParser::parseRequest(const char* szData, uint32_t nDataSize)
	{
		/*
		请求行 GET URI METHOD\r\n
		请求头1 KEY:VALUE\r\n
		请求头2 KEY:VALUE\r\n
		.
		.
		.
		\r\n
		请求体（长度由请求头中的Content-Length决定）
		*/

		while (true)
		{
			if (this->m_eParseState == eHPS_RequestLine)
			{
				const char* szEnd = szData + nDataSize;
				const char* szLineBegin = szData;
				const char* szLineEnd = std::search(szLineBegin, szEnd, szCRLF, szCRLF + 2);
				// 请求行不完整，等待完整数据
				if (szLineEnd == szEnd)
					break;

				if (!this->parseRequestLine(szLineBegin, (uint32_t)(szLineEnd - szLineBegin)))
					return false;

				this->m_nParseDataCount = (uint32_t)(szLineEnd - szLineBegin + 2);
				this->m_eParseState = eHPS_Headers;
			}
			else if (this->m_eParseState == eHPS_Headers)
			{
				const char* szHeaderBegin = szData + this->m_nParseDataCount;
				const char* szEnd = szData + nDataSize;
				const char* szHeaderEnd = std::search(szHeaderBegin, szEnd, szCRLF, szCRLF + 2);
				if (szHeaderEnd == szEnd)
					break;

				const char* szNameEnd = std::find(szHeaderBegin, szHeaderEnd, ':');
				if (szNameEnd != szHeaderEnd)
				{
					this->m_request.addHeader(szHeaderBegin, szNameEnd, szNameEnd + 1, szHeaderEnd);
				}
				else
				{
					// 头结束
					if (szHeaderEnd != szHeaderBegin)
					{
						return false;
					}

					this->m_eParseState = eHPS_Body;
				}
				this->m_nParseDataCount += (uint32_t)(szHeaderEnd - szHeaderBegin + 2);
			}
			else if (this->m_eParseState == eHPS_Body)
			{
				if (this->m_request.getMethod() == eHMT_Get)
				{
					this->m_eParseState = eHPS_Finish;
					break;
				}

				const char* szContentLength = this->m_request.getHeader("Content-Length");
				if (szContentLength == nullptr)
				{
					this->m_eParseState = eHPS_Finish;
					break;
				}

				uint32_t nContentLength = 0;
				if (!base::string_util::convert_to_value(szContentLength, nContentLength))
					return false;

				if (nContentLength + this->m_nParseDataCount > nDataSize)
					break;

				const char* szBegin = szData + this->m_nParseDataCount;
				const char* szEnd = szBegin + nContentLength;
				this->m_request.setBody(szBegin, (uint32_t)(szEnd - szBegin));
				this->m_nParseDataCount += nContentLength;

				this->m_eParseState = eHPS_Finish;
				break;
			}
		}

		return true;
	}

	EHttpParseState CHttpRequestParser::getParseState() const
	{
		return this->m_eParseState;
	}

	uint32_t CHttpRequestParser::getParseDataCount() const
	{
		return this->m_nParseDataCount;
	}

	void CHttpRequestParser::reset()
	{
		this->m_eParseState = eHPS_RequestLine;
		this->m_nParseDataCount = 0;
		this->m_request.reset();
	}
}