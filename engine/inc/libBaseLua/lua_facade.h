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

		// 这两个函数主要用于生命周期由lua控制的对象，在C++层面持有时，为了防止lua gc回收这个对象而做的引用计数
		void		addObjectRef(void* pObject);
		void		delObjectRef(void* pObject);

		// 这个函数只要用于生命周期由C++控制的对象，在C++层面释放后，lua中可能还有对这个对象的持有，需要设置对象的为NULL，防止出现野指针
		void		unbindObject(void* pObject);

	private:
		lua_State*	m_pMainLuaState;
		lua_State*	m_pActiveLuaState;
	};
}