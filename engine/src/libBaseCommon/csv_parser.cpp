#include <fstream>
#include <sstream>

#include "csv_parser.h"
#include "debug_helper.h"

namespace base
{
	namespace
	{
		struct SCSVRow
		{
			std::vector<std::string> vecValue;
		};
	}

	struct SCSVParserInfo
	{
		std::string					szName;
		char						cDelim;
		std::vector<std::string>	vecHeader;
		std::vector<SCSVRow>		vecRow;
	};

	namespace
	{
		void parseCSVHeader(SCSVParserInfo* pParserInfo, const std::string& szHeader)
		{
			std::stringstream ss(szHeader);
			std::string szItem;

			while (std::getline(ss, szItem, pParserInfo->cDelim))
			{
				pParserInfo->vecHeader.push_back(szItem);
			}
		}

		bool parseCSVRow(SCSVParserInfo* pParserInfo, const std::vector<std::string>& vecContent)
		{
			for (size_t i = 1; i < vecContent.size(); ++i)
			{
				const std::string& szLine = vecContent[i];
				bool bQuoted = false;	// 这个为了解决内容里面有","的情况
				size_t nTokenStart = 0;

				pParserInfo->vecRow.push_back(SCSVRow());

				SCSVRow& sRow = pParserInfo->vecRow.back();
				sRow.vecValue.reserve(pParserInfo->vecHeader.size());

				for (size_t j = 0; j < szLine.size(); ++j)
				{
					if (szLine.at(j) == '"')
					{
						bQuoted = ((bQuoted) ? (false) : (true));
					}
					else if (szLine.at(j) == pParserInfo->cDelim && !bQuoted)
					{
						std::string szValue = szLine.substr(nTokenStart, j - nTokenStart);
						szValue = base::string_util::trim(szValue, "\"");
						sRow.vecValue.push_back(szValue);
						nTokenStart = j + 1;
					}
				}

				//end
				std::string szValue = szLine.substr(nTokenStart, szLine.length() - nTokenStart);
				szValue = base::string_util::trim(szValue, "\"");
				sRow.vecValue.push_back(szValue);

				// 数据缺失
				if (sRow.vecValue.size() != pParserInfo->vecHeader.size())
					return false;
			}

			return true;
		}
	}

	CCSVParser::CCSVParser()
		: m_pParserInfo(nullptr)
	{
	}

	CCSVParser::~CCSVParser()
	{
		SAFE_DELETE(this->m_pParserInfo);
	}

	void CCSVParser::clear()
	{
		SAFE_DELETE(this->m_pParserInfo);
	}

	bool CCSVParser::load(const std::string& szName, char cDelim)
	{
		DebugAstEx(this->m_pParserInfo == nullptr, false);

		std::ifstream file(szName.c_str());
		if (!file.is_open())
			return false;

		std::vector<std::string> vecContent;
		std::string szLine;
		while (file.good())
		{
			std::getline(file, szLine);
#ifndef _WIN32
			szLine = base::string_util::rtrim(szLine, "\r");
#endif
			if (!szLine.empty())
				vecContent.push_back(szLine);
		}
		file.close();

		// 不能连头都没有
		if (vecContent.empty())
			return false;

		this->m_pParserInfo = new SCSVParserInfo();
		this->m_pParserInfo->szName = szName;
		this->m_pParserInfo->cDelim = cDelim;
		parseCSVHeader(this->m_pParserInfo, vecContent[0]);
		if (!parseCSVRow(this->m_pParserInfo, vecContent))
		{
			SAFE_DELETE(this->m_pParserInfo);
			return false;
		}

		return true;
	}

	int32_t CCSVParser::findColumn(const char* szColumn) const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, -1);

		for (size_t i = 0; i < this->m_pParserInfo->vecHeader.size(); ++i)
		{
			const char* szSrcColumn = this->m_pParserInfo->vecHeader[i].c_str();

			if (base::string_util::equal(szSrcColumn, szColumn, false))
				return (int32_t)i;
		}

		return -1;
	}

	const char* CCSVParser::getValue(uint32_t nRow, uint32_t nColumn) const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, nullptr);

		if (nRow >= this->m_pParserInfo->vecRow.size())
			return nullptr;
		
		SCSVRow& sRow = this->m_pParserInfo->vecRow[nRow];
		
		if (nColumn >= sRow.vecValue.size())
			return nullptr;

		return sRow.vecValue[nColumn].c_str();
	}

	const char* CCSVParser::getValue(uint32_t nRow, const char* szColumn) const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, nullptr);
		DebugAstEx(szColumn != nullptr, nullptr);

		int32_t nColumn = this->findColumn(szColumn);
		if (nColumn < 0)
			return nullptr;

		return this->getValue(nRow, nColumn);
	}

	uint32_t CCSVParser::getRowCount() const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, 0);

		return (uint32_t)this->m_pParserInfo->vecRow.size();
	}
	
	uint32_t CCSVParser::getColumnCount() const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, 0);

		return (uint32_t)this->m_pParserInfo->vecHeader.size();
	}
	
	const char* CCSVParser::getHeaderElement(uint32_t nRow) const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, nullptr);

		if (nRow >= this->m_pParserInfo->vecHeader.size())
			return nullptr;

		return this->m_pParserInfo->vecHeader[nRow].c_str();
	}
	
	const char* CCSVParser::getFileName() const
	{
		DebugAstEx(this->m_pParserInfo != nullptr, nullptr);

		return this->m_pParserInfo->szName.c_str();
	}
}