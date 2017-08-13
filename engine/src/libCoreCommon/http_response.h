#pragma once

#include "http_base.h"

#include <map>
#include <string>

namespace core
{
	class CHttpResponse
	{
	public:
		CHttpResponse();
		~CHttpResponse();

		void setStatusCode(int32_t nStatusCode);
		void setStatusMessage(const std::string& szMessage);
		void setKeepAlive(bool bEnable);
		bool isKeepAlive() const;
		void setContentType(const std::string& szContentType);
		void addHeader(const std::string& szKey, const std::string& szValue);
		void write(const char* szData);
		void write(const char* szData, uint32_t nSize);
		void redirect();

		void serialize(std::string& szData) const;

	private:
		std::map<std::string, std::string> m_mapHeader;
		int32_t								m_nStatusCode;
		std::string							m_szStatusMessage;
		std::string							m_szBody;
		bool								m_bKeepAlive;
	};
}