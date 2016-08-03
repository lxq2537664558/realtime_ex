#include "stdafx.h"
#include "table_file.h"
#include "debug_helper.h"
#include "base_function.h"

#include <cstdlib>
#include <fstream>

namespace base
{
	CTableFile::CTableFile() 
		: m_nWidth(0)
		, m_nHeight(0)
		, m_nBufSize(0)
		, m_pBuf(nullptr)
		, m_pOffsetTable(nullptr)
	{
	}

	CTableFile::~CTableFile()
	{
		this->clear();
	}

	void CTableFile::clear()
	{
		SAFE_DELETE_ARRAY(this->m_pBuf);
		SAFE_DELETE_ARRAY(this->m_pOffsetTable);
	}

	bool CTableFile::init(const char* szFileName)
	{
		DebugAstEx(szFileName != nullptr, false);
		DebugAstEx(this->m_pBuf == nullptr, false);

		std::ifstream file(szFileName, std::ios::binary | std::ios::in);

		if (!file)
			return false;

		file.seekg(0, std::ios::end);
		this->m_nBufSize = (size_t)file.tellg();
		if (this->m_nBufSize >= 3)
		{
			static char szHeader[4] = { (char)0xef, (char)0xbb, (char)0xbf, 0 };

			char szBuf[4] = { 0 };
			file.seekg(0, std::ios::beg);
			file.read(szBuf, 3);

			if (_strnicmp(szBuf, szHeader, _countof(szBuf)) == 0)
				this->m_nBufSize -= 3;
			else
				file.seekg(0, std::ios::beg);
		}

		this->m_pBuf = new char[this->m_nBufSize + 1];
		this->m_pBuf[this->m_nBufSize] = 0;
		file.read(this->m_pBuf, this->m_nBufSize);
		file.close();

		if (this->m_nBufSize != 0)
			this->genTableOffset();
		else
			return false;

		return true;
	}

	void CTableFile::genTableOffset()
	{
		size_t nSize = this->m_nBufSize;
		char* szBuf = this->m_pBuf;
		STableOffset* pTabBuf = nullptr;

		if (nullptr == szBuf || 0 == nSize)
			return;

		int32_t nWidth = 0;
		int32_t nHeight = 0;
		size_t nOffset = 0;

		// 读第一行决定有多少列
		while (*szBuf != 0x0d && *szBuf != 0x0a)
		{
			if (*szBuf == ',')
				++nWidth;
			++szBuf;
			++nOffset;
		}
		// 空文件
		if (nOffset == 0)
		{
			this->m_nWidth = 0;
			this->m_nHeight = 0;
			return;
		}
		// 算上尾巴那行
		++nWidth;
		++nHeight;

		// 去掉换行符
		if (*szBuf == 0x0d && *(szBuf + 1) == 0x0a)
		{
			szBuf += 2;
			nOffset += 2;
		}
		else
		{
			++szBuf;
			++nOffset;
		}

		// 读出高度
		while (nOffset < nSize)
		{
			while (*szBuf != 0x0d && *szBuf != 0x0a)
			{
				++szBuf;
				++nOffset;
				if (nOffset >= nSize)
					break;
			}
			++nHeight;

			// 去掉换行符
			if (*szBuf == 0x0d && *(szBuf + 1) == 0x0a)
			{
				szBuf += 2;
				nOffset += 2;
			}
			else
			{
				++szBuf;
				++nOffset;
			}
		}

		this->m_nWidth = nWidth;
		this->m_nHeight = nHeight;

		this->m_pOffsetTable = new char[this->m_nWidth*this->m_nHeight*sizeof(STableOffset)];

		pTabBuf = (STableOffset*)this->m_pOffsetTable;
		szBuf = this->m_pBuf;
		nOffset = 0;
		size_t nLength = 0;
		for (int32_t i = 0; i < nHeight; ++i)
		{
			for (int32_t j = 0; j < nWidth; ++j)
			{
				pTabBuf->nOffset = nOffset;
				nLength = 0;
				while (*szBuf != ',' && *szBuf != 0x0d && *szBuf != 0x0a && nOffset < nSize)
				{
					++szBuf;
					++nOffset;
					++nLength;
				}
				pTabBuf->nLength = nLength;
				++pTabBuf;

				if (*szBuf == 0x0d && *(szBuf + 1) == 0x0a)
				{
					szBuf += 2;
					nOffset += 2;
					break;
				}
				else
				{
					++szBuf;
					++nOffset;
				}
			}
		}
	}

	const char*	CTableFile::getString(int32_t nRow, const char* szCol, const char* szDefault) const
	{
		int32_t nCol = this->findCol(szCol);
		const char* szValue = this->getValue(nRow, nCol);
		if (nullptr == szValue)
			return szDefault;

		return szValue;
	}

	const char* CTableFile::getString(int32_t nRow, int32_t nCol, const char* szDefault) const
	{
		const char* szValue = this->getValue(nRow, nCol);
		if (nullptr == szValue)
			return szDefault;

		return szValue;
	}

	int32_t CTableFile::getInteger(int32_t nRow, const char* szCol, int32_t nDefault) const
	{
		int32_t nCol = this->findCol(szCol);

		const char* szValue = this->getValue(nRow, nCol);
		if (nullptr == szValue)
			return nDefault;

		return atoi(szValue);
	}

	int32_t CTableFile::getInteger(int32_t nRow, int32_t nCol, int32_t nDefault) const
	{
		const char* szValue = this->getValue(nRow, nCol);
		if (nullptr == szValue)
			return nDefault;

		return atoi(szValue);
	}

	float CTableFile::getFloat(int32_t nRow, const char* szCol, float fDefault) const
	{
		int32_t nCol = this->findCol(szCol);

		const char* szValue = this->getValue(nRow, nCol);
		if (nullptr == szValue)
			return fDefault;

		return (float)atof(szValue);
	}

	float CTableFile::getFloat(int32_t nRow, int32_t nCol, float fDefault) const
	{
		const char* szValue = this->getValue(nRow, nCol);
		if (nullptr == szValue)
			return fDefault;

		return (float)atof(szValue);
	}

	const char* CTableFile::getValue(int32_t nRow, int32_t nCol) const
	{
		if (nRow >= this->m_nHeight || nCol >= this->m_nWidth || nRow < 0 || nCol < 0)
			return nullptr;

		STableOffset* pTempOffset = (STableOffset*)this->m_pOffsetTable;
		char* szBuf = this->m_pBuf;

		pTempOffset += nRow * this->m_nWidth + nCol;

		szBuf += pTempOffset->nOffset;

		return szBuf;
	}

	int32_t CTableFile::findRow(const char* szRow) const
	{
		for (int32_t i = 0; i < this->m_nHeight; ++i)	// 从1开始，跳过第一行的字段行
		{
			const char* szTemp = this->getValue(i, 0);
			if (_strnicmp(szTemp, szRow, -1) == 0)
				return i;
		}
		return -1;
	}

	int32_t CTableFile::findCol(const char* szCol) const
	{
		for (int32_t i = 0; i < this->m_nWidth; ++i)	// 从1开始，跳过第一列的字段行
		{
			const char* szTemp = this->getValue(0, i);
			if (_strnicmp(szTemp, szCol, -1) == 0)
				return i;
		}
		return -1;
	}

	void CTableFile::release()
	{
		delete this;
	}

	CTableFile* CTableFile::createNew(const char* szFileName)
	{
		DebugAstEx(szFileName != nullptr, nullptr);

		CTableFile* pTableFile = new CTableFile();
		if (!pTableFile->init(szFileName))
		{
			SAFE_DELETE(pTableFile);
			return nullptr;
		}

		return pTableFile;
	}
}