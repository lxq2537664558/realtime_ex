/*---------------------------------------------------------------------------
完成CSV文件的读取，CSV文件是一个特殊格式的文本文件，将Excel编辑的制表
CSV文件即可，CSV文件每行由回车换行符分隔(0x0d 0x0a)，每个单元之间由
制表符（','）分隔.
//---------------------------------------------------------------------------*/
#pragma once

#include "noncopyable.h"

namespace base
{
	// CSV文件要不是UTF8格式，要不是ANSI格式，如果是ANSI格式，需要逻辑上考虑字符编码问题
	class __BASE_COMMON_API__ CTableFile :
		public noncopyable
	{
	private:
		void				genTableOffset();
		const char*			getValue(int32_t nRow, int32_t nCol) const;
		bool				init(const char* szFileName);

		CTableFile();
		~CTableFile();

	public:
		//返回以1为起点的值
		int32_t				findRow(const char* szRow) const;
		//返回以0为起点的值
		int32_t				findCol(const char* szCol) const;
		inline int32_t		getWidth() const { return this->m_nWidth; }
		inline int32_t		getHeight() const { return this->m_nHeight; }
		const char*			getString(int32_t nRow, const char* szCol, const char* szDefault) const;
		const char*			getString(int32_t nRow, int32_t nCol, const char* szDefault) const;
		int32_t				getInteger(int32_t nRow, const char* szCol, int32_t nDefault) const;
		int32_t				getInteger(int32_t nRow, int32_t nCol, int32_t nDefault) const;
		float				getFloat(int32_t nRow, const char* szCol, float fDefault) const;
		float				getFloat(int32_t nRow, int32_t nColumn, float fDefault) const;
		void				clear();
		void				release();

		static CTableFile*	createNew(const char* szFileName);

	private:
		struct STableOffset
		{
			size_t nOffset;
			size_t nLength;
		};

		int32_t		m_nWidth;
		int32_t		m_nHeight;
		size_t		m_nBufSize;
		char*		m_pBuf;
		char*		m_pOffsetTable;
	};
}