/*---------------------------------------------------------------------------
���CSV�ļ��Ķ�ȡ��CSV�ļ���һ�������ʽ���ı��ļ�����Excel�༭���Ʊ�
CSV�ļ����ɣ�CSV�ļ�ÿ���ɻس����з��ָ�(0x0d 0x0a)��ÿ����Ԫ֮����
�Ʊ����','���ָ�.
//---------------------------------------------------------------------------*/
#pragma once

#include "noncopyable.h"

namespace base
{
	// CSV�ļ�Ҫ����UTF8��ʽ��Ҫ����ANSI��ʽ�������ANSI��ʽ����Ҫ�߼��Ͽ����ַ���������
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
		//������1Ϊ����ֵ
		int32_t				findRow(const char* szRow) const;
		//������0Ϊ����ֵ
		int32_t				findCol(const char* szCol) const;
		inline int32_t		getWidth() const { return this->m_nWidth; }
		inline int32_t		getHeight() const { return this->m_nHeight; }
		const char*			getString(int32_t nRow, const char* szCol, const char* szDefault) const;
		const char*			getString(int32_t nRow, int32_t nCol, const char* szDefault) const;
		int32_t				getInteger(int32_t nRow, const char* szCol, int32_t nDefault) const;
		int32_t				getInteger(int32_t nRow, int32_t nCol, int32_t nDefault) const;
		float				getFloat(int32_t nRow, const char* szCol, float nDefault) const;
		float				getFloat(int32_t nRow, int32_t nColumn, float nDefault) const;
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