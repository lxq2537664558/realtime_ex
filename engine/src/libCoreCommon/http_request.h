#pragma once

#include "http_base.h"

#include <map>
#include <string>

namespace core
{
	class CHttpRequest
	{
	public:
		CHttpRequest();
		~CHttpRequest();

		void				reset();

		void				setVersion(EHttpVersion eVersion);
		EHttpVersion		getVersion() const;

		bool				setMethod(const char* szMethod);
		EHttpMethodType		getMethod() const;

		void				setRequestURI(const char* szURI, uint32_t nCount);
		const std::string&	getRequestURI() const;

		void				setQuery(const char* szQuery, uint32_t nCount);
		const std::string&	getQuery() const;

		void				setBody(const char* szBody, uint32_t nCount);
		const std::string&	getBody() const;

		void				addHeader(const char* szNameBegin, const char* szNameEnd, const char* szValueBegin, const char* szValueEnd);
		const char*			getHeader(const char* szName) const;

		const std::map<std::string, std::string>&
							getHeaders() const;

	private:
		EHttpMethodType						m_eMethod;
		EHttpVersion						m_eVersion;
		std::string							m_szRequestURI;
		std::string							m_szQuery;
		std::string							m_szBody;
		std::map<std::string, std::string>	m_mapHeader;
	};
}
