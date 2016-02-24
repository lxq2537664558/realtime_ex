#pragma once

#include "libBaseCommon\noncopyable.h"
#include "libCoreCommon\core_common.h"

#include "message_port.h"

#include <vector>
#include <map>
#include <set>

namespace core
{
	struct SRequestMessageInfo
	{
		std::string szMessageName;
		std::string szMessageData;
		uint32_t	nSessionID;
		std::function<void(const google::protobuf::Message*, EResponseResultType)>
					callback;
	};

	struct SResponseMessageInfo : public SRequestMessageInfo
	{
		uint8_t	nResult;
	};

	class CConnectionToService;
	class CConnectionFromService;
	class CServiceMgr :
		public base::noncopyable
	{
	public:
		CServiceMgr();
		~CServiceMgr();

		static CServiceMgr*	Inst();

		bool					init();

		bool					call(const std::string& szServiceName, SRequestMessageInfo& sRequestMessageInfo);
		bool					response(SResponseMessageInfo& sResponseMessageInfo);

		void					registMessageHandler(const std::string& szMessageName, std::function<void(const google::protobuf::Message*)>& callback);

		CConnectionToService*	getConnectionToService(const std::string& szName) const;
		void					addConnectionToService(CConnectionToService* pConnectionToService);
		void					delConnectionToService(const std::string& szName);
		CConnectionFromService*	getConnectionFromService(const std::string& szName) const;
		void					addConnectionFromService(CConnectionFromService* pConnectionFromService);
		void					delConnectionFromService(const std::string& szName);
		void					addServiceBaseInfo(const SServiceBaseInfo& sServiceBaseInfo);
		void					delServiceBaseInfo(const std::string& szName);
		const SServiceBaseInfo*	getServiceBaseInfo(const std::string& szName) const;
		void					getServiceName(const std::string& szType, std::vector<std::string>& vecServiceName) const;

		void					onDispatch(uint16_t nType, const void* pData, uint16_t nSize);

	private:
		void					onConnectRefuse(const std::string& szContext);
		void					onCheckConnect(uint64_t nContext);
		void					onTimeout(uint64_t nContext);
		void					checkResponseTimeout(uint32_t nSessionID);
		uint32_t				getSessionID() const;
		void					incSessionID();

	private:
		struct SRequestMessageGroupInfo
		{
			uint32_t							nTotalSize;
			bool								bRefuse;
			std::vector<SRequestMessageInfo>	vecRequestMessageInfo;
		};

		struct SResponseInfo
		{
			std::function<void(const google::protobuf::Message*, EResponseResultType)>	callback;
			CTicker																		tickTimeout;
			uint32_t																	nSessionID;
			std::string																	szServiceName;
			std::string																	szMessageName;
		};

		uint32_t																			m_nNextSessionID;
		uint32_t																			m_nWorkSessionID;
		std::string																			m_szWorkServiceName;
		CTicker																				m_tickCheckConnect;
		std::map<uint32_t, SResponseInfo*>													m_mapResponseInfo;
		std::map<const std::string, std::function<void(const google::protobuf::Message*)>>	m_mapMessageHandler;

		std::map<std::string, CConnectionToService*>										m_mapConnectionToService;
		std::map<std::string, CConnectionFromService*>										m_mapConnectionFromService;
		std::map<std::string, SServiceBaseInfo>												m_mapServiceBaseInfo;
		std::map<std::string, SRequestMessageGroupInfo>										m_mapCacheMessageGroupInfo;
	};

}