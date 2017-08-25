#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "mysql/mysql.h"

namespace base
{
	enum EMysqlErrorType
	{
		eMET_OK						= 0,
		eMET_LostConnection			= 1,	// ���Ӷ�ʧ
		eMET_NoPrivileges			= 1044, // ���ݿ��û�Ȩ�޲���
		eMET_NoDiskSpace			= 1021, // Ӳ��ʣ��ռ䲻��
		eMET_KeyDupForUpdate		= 1022, // �ؼ����ظ������ļ�¼ʧ��
		eMET_NoRecord				= 1032, // ��¼������
		eMET_NoMemory				= 1037, // ϵͳ�ڴ治��
		eMET_OutofConnection		= 1040, // �ѵ������ݿ���������������Ӵ����ݿ����������
		eMET_SQLSyntax				= 1149, // SQL����﷨����
		eMET_KeyDupForInsert		= 1062,	// �ؼ����ظ��������¼ʧ��
		eMET_CommitTrans			= 1180, // �ύ����ʧ��
		eMET_RollbackTrans			= 1181, // �ع�����ʧ��
		eMET_Deadloop				= 1205, // ������ʱ
		eMET_StatementReprepared	= 1615,	// Statement��Ҫ����׼��
		
		eMET_Unknwon = -1
	};


	class CDbRecordset;
	class CDbConnection
	{
		friend class CDbFacade;

	public:
		CDbConnection();
		virtual ~CDbConnection();

		bool			connect(const std::string& szHost, uint16_t nPort, const std::string& szUser, const std::string& szPassword, const std::string& szDbname, const std::string& szCharacterset);
		bool			isConnect() const;
		void			close();
		
		uint32_t		execute(const std::string& szSQL, CDbRecordset** pDbRecordset);
		bool			ping();
		uint64_t		getAffectedRow() const;
		void			autoCommit(bool enable);
		void			begin();
		void			commit();
		void			rollback();
		std::string		escape(const std::string& szSQL);

	private:
		MYSQL*	m_pMysql;	
	};
}