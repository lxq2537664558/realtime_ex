#include "http_response.h"

#include "libBaseCommon/base_function.h"

namespace core
{
	CHttpResponse::CHttpResponse()
		: m_nStatusCode(0)
		, m_bKeepAlive(true)
	{

	}

	CHttpResponse::~CHttpResponse()
	{

	}

	void CHttpResponse::serialize(std::string& szData) const
	{
		char szBuf[64] = { 0 };
		snprintf(szBuf, sizeof(szBuf), "HTTP/1.1 %d ", this->m_nStatusCode);
		szData = szBuf;
		szData += this->m_szStatusMessage;
		szData += "\r\n";

		for (auto iter = this->m_mapHeader.begin(); iter != m_mapHeader.end(); ++iter)
		{
			szData += iter->first;
			szData += ": ";
			szData += iter->second;
			szData += "\r\n";
		}

		if (this->m_bKeepAlive)
		{
			szData += "Connection: Keep-Alive\r\n";
		}
		else
		{
			szData += "Connection: close\r\n";
		}

		if (!this->m_szBody.empty())
		{
			base::crt::snprintf(szBuf, sizeof(szBuf), "Content-Length: %d\r\n", this->m_szBody.size());
			szData += szBuf;
		}

		szData += "\r\n";
		szData += this->m_szBody;
	}

	void CHttpResponse::setContentType(const std::string& szContentType)
	{
		this->addHeader("Content-Type", szContentType);
	}

	void CHttpResponse::setStatusCode(int32_t nStatusCode)
	{
		this->m_nStatusCode = nStatusCode;
	}

	void CHttpResponse::setStatusMessage(const std::string& szMessage)
	{
		this->m_szStatusMessage = szMessage;
	}

	void CHttpResponse::setKeepAlive(bool bEnable)
	{
		this->m_bKeepAlive = bEnable;
	}

	bool CHttpResponse::isKeepAlive() const
	{
		return this->m_bKeepAlive;
	}

	void CHttpResponse::addHeader(const std::string& szKey, const std::string& szValue)
	{
		this->m_mapHeader[szKey] = szValue;
	}

	void CHttpResponse::write(const char* szData)
	{
		this->m_szBody.append(szData);
	}

	void CHttpResponse::write(const char* szData, uint32_t nSize)
	{
		this->m_szBody.append(szData, nSize);
	}
}