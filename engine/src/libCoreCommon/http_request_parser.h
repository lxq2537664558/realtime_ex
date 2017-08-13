#pragma once

#include "http_request.h"

namespace core
{
	class CHttpRequestParser
	{
	public:
		CHttpRequestParser();
		~CHttpRequestParser();

		bool			parseRequest(const char* szData, uint32_t nDataSize);

		CHttpRequest&	getRequest();
		
		EHttpParseState	getParseState() const;
		uint32_t		getParseDataCount() const;

		void			reset();

	private:
		bool			parseRequestLine(const char* szData, uint32_t nDataSize);

	private:
		EHttpParseState	m_eParseState;
		uint32_t		m_nParseDataCount;
		CHttpRequest	m_request;
	};
}