#pragma once
#include "libBaseCommon/noncopyable.h"

#include "core_common.h"

#include <map>
#include <vector>

namespace core
{
	class CMessageDirectory :
		public base::noncopyable
	{
	public:
		CMessageDirectory();
		~CMessageDirectory();

		bool	init();

		void	registerCallback(const std::string& szMessageName, const ServiceCallback& callback);
		void	registerCallback(const std::string& szMessageName, const GateClientCallback& callback);
		ServiceCallback&	
				getCallback(uint32_t nMessageID);
		GateClientCallback&
				getGateClientCallback(uint32_t nMessageID);

		void	addGlobalBeforeFilter(const ServiceGlobalFilter& callback);
		void	addGlobalAfterFilter(const ServiceGlobalFilter& callback);
		const std::vector<ServiceGlobalFilter>&
				getGlobalBeforeFilter();
		const std::vector<ServiceGlobalFilter>&
				getGlobalAfterFilter();

		const std::string&
				getMessageName(uint32_t nMessageID) const;

		void	onConnectToMaster();

		void	addOtherServiceMessageInfo(const std::string& szServiceName, const std::vector<SMessageSyncInfo>& vecMessageSyncInfo);
		void	delOtherServiceMessageInfo(const std::string& szServiceName);
		const std::set<std::string>&
				getOtherServiceName(const std::string& szMessageName) const;

	private:
		void	sendMessageInfo(const std::string& szMessageName);

	private:
		std::map<uint32_t, ServiceCallback>				m_mapServiceCallback;
		std::map<uint32_t, GateClientCallback>			m_mapGateClientCallback;
		std::map<uint32_t, std::string>					m_mapOwnerMessageName;
		std::map<std::string, std::set<std::string>>	m_mapOtherMessageDirectoryByMessageName;
		std::map<std::string, std::set<std::string>>	m_mapOtherMessageDirectoryByServiceName;

		std::vector<ServiceGlobalFilter>				m_vecServiceGlobalBeforeFilter;
		std::vector<ServiceGlobalFilter>				m_vecServiceGlobalAfterFilter;
	};
}