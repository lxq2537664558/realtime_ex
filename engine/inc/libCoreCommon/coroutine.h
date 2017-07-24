#pragma once

#include <stdint.h>
#include <functional>

#include "core_common.h"

namespace core
{
	namespace coroutine
	{

		enum ECoroutineState
		{
			eCS_NONE,
			eCS_DEAD,	// ����״̬
			eCS_READY,	// �ȴ�ִ�У�������û�б�ִ�й���
			eCS_RUNNING,// ִ��״̬
			eCS_SUSPEND,// ����״̬
		};

		__CORE_COMMON_API__ void		init(uint32_t nStackSize);
		/**
		@brief: ����Э�̣�����Э����ں���
		��������ջ��С��0�Ļ����ù���ջ��������ö���ջ
		*/
		__CORE_COMMON_API__ uint64_t	create(uint32_t nStackSize, const std::function<void(uint64_t)>& fn);
		/**
		@brief: �ر�ָ��Э��
		*/
		__CORE_COMMON_API__ void		close(uint64_t nID);
		/**
		@brief: �ָ�ĳһ��Э��ִ�У����Դ�һ��������yield�������غ��ȡ�������Э�̵�һ��ִ�о�����ں����Ĳ���
		*/
		__CORE_COMMON_API__ void		resume(uint64_t nID, uint64_t nContext);
		/**
		@brief: ����ǰִ�е�Э�̣���Э���´λ�ִ��ʱ�᷵��resume�����������Ĳ���
		*/
		__CORE_COMMON_API__ uint64_t	yield();
		/**
		@brief: ��ȡЭ��״̬
		*/
		__CORE_COMMON_API__ uint32_t	getState(uint64_t nID);
		/**
		@brief: ����Э������
		*/
		__CORE_COMMON_API__ void		setLocalData(uint64_t nID, const char* szName, uint64_t nData);
		/**
		@brief: ��ȡЭ������
		*/
		__CORE_COMMON_API__ bool		getLocalData(uint64_t nID, const char* szName, uint64_t& nData);
		/**
		@brief: ɾ��Э������
		*/
		__CORE_COMMON_API__ void		delLocalData(uint64_t nID, const char* szName);
		/**
		@brief: ��ȡ��ǰִ��Э�̵�ID
		*/
		__CORE_COMMON_API__ uint64_t	getCurrentID();
		/**
		@brief: ��ȡЭ������
		*/
		__CORE_COMMON_API__ uint32_t	getCoroutineCount();
		/**
		@brief: ��ȡ�ܵĶ���ջ��С
		*/
		__CORE_COMMON_API__ uint64_t	getTotalStackSize();
	}
}