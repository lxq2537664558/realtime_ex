#pragma once

namespace core
{
	class CInvokerTrace
	{
	public:
		CInvokerTrace();
		~CInvokerTrace();

		void	startNewTrace();
		void	AppendTraceText();
	};
}