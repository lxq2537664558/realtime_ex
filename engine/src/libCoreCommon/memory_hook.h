#pragma once

namespace core
{
	void	beginMemoryLeakChecker(bool bDetail);
	void	endMemoryLeakChecker(const char* szName);
}