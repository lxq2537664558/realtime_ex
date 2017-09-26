#pragma once
#include <stdint.h>

#include "lua-5.3.2/lua.hpp"

#ifdef _WIN32
#	ifdef __BUILD_BASE_LUA_DLL__
#		define __BASE_LUA_API__ __declspec(dllexport)
#	else
#		define __BASE_LUA_API__ __declspec(dllimport)
#	endif
#else
#	define __BASE_LUA_API__
#endif

namespace base
{

	class __BASE_LUA_API__ CLuaFacade
	{
	public:
		CLuaFacade();
		~CLuaFacade();

		bool		open();
		void		close();
		bool		loadFile(const char* szName);
		bool		runString(const char* szBuf, const char* szBufName);
		void		addSeachPath(const char* szPath);
		bool		startDebug(const char* szIP, uint16_t nPort);
		void		updateDebug();
		
		lua_State*	getState() const;

		void		setActiveLuaState(lua_State* pL);
		lua_State*	getActiveLuaState() const;

		// ������������Ҫ��������������lua���ƵĶ�����C++�������ʱ��Ϊ�˷�ֹlua gc�������������������ü���
		void		addObjectRef(void* pObject);
		void		delObjectRef(void* pObject);

		// �������ֻҪ��������������C++���ƵĶ�����C++�����ͷź�lua�п��ܻ��ж��������ĳ��У���Ҫ���ö����ΪNULL����ֹ����Ұָ��
		void		unbindObject(void* pObject);

	private:
		lua_State*	m_pMainLuaState;
		lua_State*	m_pActiveLuaState;
	};
}