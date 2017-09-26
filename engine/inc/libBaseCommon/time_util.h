#pragma once

#include "base_common.h"

namespace base
{
	namespace time_util
	{
		struct STime
		{
			uint32_t nYear;	// ��	[0, -)
			uint32_t nMon;	// ��	[1,12]
			uint32_t nDay;	// ��	[1,31]
			uint32_t nWDay;	// ����	[0, 6](���쿪ʼ)
			uint32_t nHour;	// ʱ	[0,23]
			uint32_t nMin;	// ��	[0,59]
			uint32_t nSec;	// ��	[0,59]
			uint32_t nMSec;	// ���� [0,999]

			STime()
			{
				memset(this, 0, sizeof(STime));
			}
		};

		/**
		@brief: ��ȡ��ǰ�ĸ�������ʱ�䣬��λ����
		*/
		__BASE_COMMON_API__ int64_t	getGmtTime();
		/**
		@brief: ��ȡ�������θ�ʽ����ʱ��
		*/
		__BASE_COMMON_API__ STime	getGmtTimeTM(int64_t nTime = 0);
		/**
		@brief: ���ݸ�ʽ���ĸ�������ʱ���ȡ��ֵʱ�䣬��λ����
		*/
		__BASE_COMMON_API__ int64_t	getGmtTimeByTM(const STime& sTime);
		/**
		@brief: �����ַ�����ʽ��ʱ���ȡ��ֵʱ��(�ַ�����ʽ��2010-01-01 01:01:01)����λ����
		*/
		__BASE_COMMON_API__ int64_t	getGmtTimeByBuf(const char* szBuf);
		/**
		@brief: ��ȡ����ʱ�䣬��λ����
		*/
		__BASE_COMMON_API__ int64_t	getLocalTime();
		/**
		@brief: ��ȡ���ظ�ʽ����ʱ��
		*/
		__BASE_COMMON_API__ STime	getLocalTimeTM(int64_t nTime = 0);
		/**
		@brief: ���ݸ�ʽ���ı���ʱ���ȡ��ֵʱ�䣬��λ����
		*/
		__BASE_COMMON_API__ int64_t	getLocalTimeByTM(const STime& sTime);
		/**
		@brief: �����ַ�����ʽ��ʱ���ȡ��ֵʱ��(�ַ�����ʽ��2010-01-01 01:01:01)����λ����
		*/
		__BASE_COMMON_API__ int64_t	getLocalTimeByBuf(const char* szBuf);
		/**
		@brief: ��ȡ��ǰʱ������λ����
		*/
		__BASE_COMMON_API__ int32_t	getZoneTime();
		/**
		@brief: ��������ʱ��ת����ʱ�䣬��λ����
		*/
		__BASE_COMMON_API__ int64_t	gmt2LocalTime(int64_t nTime);
		/**
		@brief: ����ʱ��ת��������ʱ�䣬��λ����
		*/
		__BASE_COMMON_API__ int64_t	local2GmtTime(int64_t nTime);
		/**
		@brief: ��ʽ������ʱ��(�ַ�����ʽ��2010-01-01 01:01:01)
		*/
		__BASE_COMMON_API__ size_t	formatLocalTime(char* szBuf, size_t nBufSize, int64_t nTime = 0);
		/**
		@brief: ��ʽ����������ʱ��(�ַ�����ʽ��2010-01-01 01:01:01)
		*/
		__BASE_COMMON_API__ size_t	formatGmtTime(char* szBuf, size_t nBufSize, int64_t nTime = 0);
		/**
		@brief: ��ȡ��ǰʱ�䵽ָ��ʱ��֮���ʱ��������λ����
		*/
		__BASE_COMMON_API__ int64_t	getLocalSpanTime(const STime& sTime);
		/**
		@brief: ���ø�������ʱ��
		*/
		__BASE_COMMON_API__ void	setGmtTime(int64_t nTime);
		/**
		@brief: ��ȡ�������������ڵ�ʱ�䣨�߾��ȣ�΢�
		*/
		__BASE_COMMON_API__ int64_t	getProcessPassTime();


		/**
		@brief: ���ʱ�������
		*/
		class __BASE_COMMON_API__ CTimeElapse
		{
		private:
			int64_t	m_nBeginTime;

		public:
			CTimeElapse();
			~CTimeElapse();

			void  beginTime();
			int64_t getElapse() const;
		};
	}
}