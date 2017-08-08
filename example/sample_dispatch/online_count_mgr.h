#pragma once
#include "libBaseCommon/noncopyable.h"

#include <map>

class COnlineCountMgr :
	public base::noncopyable
{
public:
	COnlineCountMgr();
	virtual ~COnlineCountMgr();

	uint32_t	getSuitableGasID() const;
	uint32_t	getSuitableGateID() const;

	const std::string& 
				getGateAddr(uint32_t nGateID) const;

	void		setGateAddr(uint32_t nGateID, const std::string& szAddr);

	void		setGasOnlineCount(uint32_t nGasID, uint32_t nCount);
	void		setGateOnlineCount(uint32_t nGateID, uint32_t nCount);

private:
	std::map<uint32_t, uint32_t>	m_mapGasOnline;
	std::map<uint32_t, uint32_t>	m_mapGateOnline;
	std::map<uint32_t, std::string>	m_mapGateAddr;
};