#include "http_request.h"

#include "libBaseCommon/base_common.h"
#include "libBaseCommon/debug_helper.h"

#include <algorithm>

namespace core
{
	CHttpRequest::CHttpRequest()
		: m_eMethod(eHMT_Unknown)
		, m_eVersion(eHV_Unknown)
	{

	}

	CHttpRequest::~CHttpRequest()
	{

	}

	bool CHttpRequest::setMethod(const char* szMethod)
	{
		DebugAstEx(szMethod != nullptr, false);

		if (_strnicmp(szMethod, "GET", 3) == 0)
		{
			this->m_eMethod = eHMT_Get;
		}
		else if (_strnicmp(szMethod, "POST", 4) == 0)
		{
			this->m_eMethod = eHMT_Post;
		}
		else if (_strnicmp(szMethod, "PUT", 3) == 0)
		{
			this->m_eMethod = eHMT_Put;
		}
		else if (_strnicmp(szMethod, "DELETE", 6) == 0)
		{
			this->m_eMethod = eHMT_Delete;
		}
		else
		{
			this->m_eMethod = eHMT_Unknown;
		}

		return this->m_eMethod != eHMT_Unknown;
	}

	void CHttpRequest::setBody(const char* szBody, uint32_t nCount)
	{
		DebugAst(szBody != nullptr);

		this->m_szBody = std::move(std::string(szBody, nCount));
	}

	const std::string& CHttpRequest::getBody() const
	{
		return this->m_szBody;
	}

	void CHttpRequest::addHeader(const char* szNameBegin, const char* szNameEnd, const char* szValueBegin, const char* szValueEnd)
	{
		std::string szName(szNameBegin, szNameEnd);

		while (szValueBegin < szValueEnd && isspace(*szValueBegin))
		{
			++szValueBegin;
		}

		std::string szValue(szValueBegin, szValueEnd);
		for (size_t i = szValue.size(); i > 0; --i)
		{
			if (isspace(szValue[i - 1]))
				szValue.resize(szValue.size() - 1);
		}

		this->m_mapHeader[szName] = std::move(szValue);
	}

	const char* CHttpRequest::getHeader(const char* szName) const
	{
		DebugAstEx(szName != nullptr, nullptr);

		auto iter = m_mapHeader.find(szName);
		if (iter == m_mapHeader.end())
			return nullptr;

		return iter->second.c_str();
	}

	void CHttpRequest::reset()
	{
		this->m_eMethod = eHMT_Unknown;
		this->m_eVersion = eHV_Unknown;
		this->m_szRequestURI.clear();
		this->m_szQuery.clear();
		this->m_szBody.clear();
		this->m_mapHeader.clear();
	}

	void CHttpRequest::setVersion(EHttpVersion eVersion)
	{
		this->m_eVersion = eVersion;
	}

	EHttpVersion CHttpRequest::getVersion() const
	{
		return this->m_eVersion;
	}

	EHttpMethodType CHttpRequest::getMethod() const
	{
		return this->m_eMethod;
	}

	void CHttpRequest::setRequestURI(const char* szURI, uint32_t nCount)
	{
		DebugAst(szURI != nullptr);

		this->m_szRequestURI.assign(szURI, nCount);
	}

	const std::string& CHttpRequest::getRequestURI() const
	{
		return this->m_szRequestURI;
	}

	void CHttpRequest::setQuery(const char* szQuery, uint32_t nCount)
	{
		DebugAst(szQuery != nullptr);

		this->m_szQuery.assign(szQuery, nCount);
	}

	const std::string& CHttpRequest::getQuery() const
	{
		return m_szQuery;
	}

	const std::map<std::string, std::string>& CHttpRequest::getHeaders() const
	{
		return this->m_mapHeader;
	}
}