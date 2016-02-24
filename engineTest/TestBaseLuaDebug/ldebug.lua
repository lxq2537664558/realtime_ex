local max_breakpoint_index	= 0		--最大的断点索引
local last_command_expr		= ""	--最近的命令表达式
local last_trace_count		= 0		--最近一次调试的栈深度
local last_func				= nil	--最近一次调试的函数
local step_into				= false	--逐语句调试	
local next_over				= false	--逐过程调试
local print_tbl_flag		= {}	--打印table时防止死循环做的标记
local breakpoint_info		= {}	--断点信息
local debug_state			= "CLOSE"	--调试状态 CLOSE OPEN RUN
local CMD = {}

local function read_src_file(name, num)
    local file = io.open( name, "r" )
    if file == nil then
		return nil
	end
	
	local line
    for str in file:lines() do
		num = num - 1
		if num <= 0 then
			line = str
			break;
		end
    end
    file:close()                   --调用结束后记得关闭  
	
	return line
end

local function debug_log( log_str, new_line, show_ldbg )
	local ldbg = ""
	if show_ldbg then
		ldbg = "ldbg> "
	end
	if new_line then
		debugger.write( ldbg .. log_str .. "\n" )
	else
		debugger.write( ldbg .. log_str )
	end
end

local function print_var( name, value, level )
	local prefix = string.rep( "    ", level )
	local str = string.format( "%s%s = %s", prefix, name, tostring(value) )
	
	if type( value ) == "table" then
		if print_tbl_flag[value] then
			--已在临时表中的,只打印表地址
			debug_log( str, true, false )
			return
		end
		--加到临时表中,以免表出现循环引用时,打印也产生死循环
		print_tbl_flag[value] = true
		--打印表中所有数据
		debug_log( string.format( "%s%s = {", prefix, name ), true, false )
		for k, v in pairs( value ) do
			--不打印 "_"开头的内部变量
			if string.sub( k, 1, 1 ) ~= "_" then
				print_var( k, v, level + 1 )
			end
		end
		debug_log( prefix .. "}", true, false )
	elseif type( value ) == "string" then
		debug_log( str, true, false )
	else
		debug_log( str, true, false )
	end
end

local function print_expr( var, level )
	--清空table标记表
	print_tbl_flag = {}
	
	local index = 1
	--找局部变量
	while true do
		local name, value = debug.getlocal( level, index )
		if not name then
			break
		end
		index = index + 1
		if name == var then
			print_var( var, value, 0 )
			return
		end
	end
	
	--找upvale变量
	local func = debug.getinfo( level, "f" ).func
	local index = 1
	while true do
		local name, value = debug.getupvalue( func, index )
		if not name then
			break
		end
		index = index + 1
		if name == var then
			print_var( var, value, 0 )
			return
		end
	end
	
	--找全局变量
	if _G[var] ~= nil then
		print_var( var, _G[var], 0 )
		return
	end
	
	debug_log( "No symbol \"" .. var .. "\" in current context.", true, false )
end

function CMD.list_source( num )
end

function CMD.add_breakpoint( expr, log_repeat )
	if debug_state == "CLOSE" then
		return
	end
	
	local si = string.find( expr, ":" )
	if nil == si then
		debug_log( "add breakpoint error, expr (" .. expr .. ") invalid", true, false )
		return
	end
	
	local line = string.sub( expr, si + 1 )
	local line = tonumber( line )
	local source = string.sub( expr, 1, si - 1 )
	
	--先查找有不有相同断点
	if ( breakpoint_info[line] ~= nil ) and ( breakpoint_info[line][source] ~= nil ) then
		if log_repeat then
			debug_log( string.format( "breakpoint %s:%d existed", source, line ), true, false )
		end
		return
	end
	
	local tbl = {}
	tbl.source = source
	tbl.line = line
	tbl.active = true
	tbl.number = max_breakpoint_index + 1
	
	if breakpoint_info[line] == nil then
		breakpoint_info[line] = {}
	end
	
	breakpoint_info[line][source] = tbl
	max_breakpoint_index = max_breakpoint_index + 1
	debug_log( "breakpoint " .. tbl.number .. " at " .. expr, true, false )
end

function CMD.list_breakpoints()
	for k, v in pairs( breakpoint_info ) do
		if type( v ) == "table" then
			for k1, v1 in pairs( v ) do
				local str = string.format( "breakpoint number:%d  %s:%d  active:", v1.number, v1.source, v1.line )
				if v1.active then
					str = str .. "enable"
				else
					str = str .. "disable"
				end
				debug_log( str, true, false )
			end
		end
	end
end

function CMD.del_breakpoint( expr )
	local number = tonumber( expr )
	for k, v in pairs( breakpoint_info ) do
		if type( v ) == "table" then
			for k1, v1 in pairs( v ) do
				if v1.number == number then
					breakpoint_info[k][k1] = nil
					debug_log( "del breakpoint:" .. number .. " ok", false )
				end
			end
		end
	end
end

function CMD.enable_breakpoint( expr )
	local number = tonumber( expr )
	for k, v in pairs( breakpoint_info ) do
		if type( v ) == "table" then
			for k1, v1 in pairs( v ) do
				if v1.number == number then
					v1.active = true
					debug_log( "enable breakpoint:" .. number, true, false )
				end
			end
		end
	end
end

function CMD.disable_breakpoint( expr )
	local number = tonumber( expr )
	for k, v in pairs( breakpoint_info ) do
		if type( v ) == "table" then
			for k1, v1 in pairs( v ) do
				if v1.number == number then
					v1.active = false
					debug_log( "disable breakpoint:" .. number, true, false )
				end
			end
		end
	end
end

function CMD.debug_help()
	print( "h             help info"			)
	print( "c             continue"				)
	print( "s             step"					)
	print( "n             next"					)
	print( "p var         print variable"		)
	print( "b src:line    add breakpoint"		)
	print( "d number      del breakpoint"		)
	print( "bl            list breakpoint"		)
	print( "be number     enable breakpoint"	)
	print( "bd number     disable breakpoint"	)
	print( "bt            print traceback"		)
	print( "l             list source"			)
end

local function execute_command( env )
	debug_log( "", false, true )
	local command_expr = nil
	
	while true do
		command_expr = debugger.read()
		if command_expr ~= nil then
			break
		end
		
		if debug_state == "CLOSE" then
			return true
		end
	end
	print("command " .. command_expr)
	--io.write( "ldbg> " )
	--local command_expr = io.read()
	
	--取上一次的命令,方便调试
	if command_expr ~= "" then
		last_command_expr = command_expr
	else
		command_expr = last_command_expr
	end
	
	local command = command_expr
	local expr = ""
	local si = string.find( command_expr, " " )
	
	if si ~= nil then
		command = string.sub( command_expr, 1, si - 1 )
		expr = string.sub( command_expr, string.find( command_expr, " %w" ) + 1 )
	end
	
	if command == "c" then
		step_into = false
		next_over = false
		return true
	elseif command == "s" then
		step_into = true
		next_over = false
		return true
	elseif command == "n" then
		step_into = false
		next_over = true
		last_func = env.func
		last_trace_count = traceback_count() - 1
		return true
	elseif command == "p" then
		print_expr( expr, 4 )
	elseif command == "b" then
		CMD.add_breakpoint( expr, true )
	elseif command == "bl" then
		CMD.list_breakpoints()
	elseif command == "d" then
		CMD.del_breakpoint( expr )
	elseif command == "be" then
		CMD.enable_breakpoint( expr )
	elseif command == "bd" then
		CMD.disable_breakpoint( expr )
	elseif command == "bt" then
		--干掉开头的空行
		local trace = debug.traceback( "", 3 )
		local trace = string.sub( trace, 2 )
		debug_log( trace, true, false )
	elseif command == "h" then
		CMD.debug_help()
	elseif command == "l" then
		CMD.list_source( expr )
	else
		debug_log( "invalid command_expr:" .. command_expr, true, false )
	end
	
	return false
end

local skip_hook = debug.getinfo( execute_command, "S" ).short_src

local function debug_hook( event, line )
	local env = debug.getinfo( 2 )
	--本文件不调试
	if env.short_src == skip_hook then
		return
	end
	print(env.short_src .. " " .. line)
	-- s, n, 断点时中断
	local interrupt_run = false
	--判断是否在next调试
	if next_over then
		local trace_count = traceback_count()
		--函数返回了,调用栈数量就会比现在小
		if trace_count < last_trace_count then
			interrupt_run = true
		elseif trace_count == last_trace_count then
			--相同函数
			if last_func == env.func then
				interrupt_run = true
			end
		end
		next_over = false
	end
	
	--判断是否有断点命中
	local breakpoint_number = -1
	if breakpoint_info[line] ~= nil then
		local tbl = breakpoint_info[line][env.short_src]
		if  ( tbl ~= nil ) and tbl.active then
			interrupt_run = true
			breakpoint_number = tbl.number
		end
	end
	
	if step_into then
		interrupt_run = true
	end
	
	if debug_state == "OPEN" then
		interrupt_run = true
		debug_state = "RUN"
		print("debug_state: " .. debug_state)
	end
	
	if interrupt_run then
		local src = read_src_file( env.short_src, line )
		local funname = env.name or "unknow"
		if breakpoint_number ~= -1 then
			debug_log( "breakpoint " .. breakpoint_number .. ", " .. string.format( "%s:%d(%s)\n%d\t%s\n", env.short_src, line, funname, line, src ), false, false )
		else
			debug_log( string.format( "%d\t%s\n", line, src ), false, false )
		end
		while not execute_command( env ) do
		end
	end
end

function open_debug()
	step_into = false
	debug_state = "OPEN"
	print("debug_state: " .. debug_state)
	debug.sethook( debug_hook, "l" )
end

--关闭debugger
function close_debug()
	step_into = false
	next_over = false
	debug_state = "CLOSE"
	print("debug_state: " .. debug_state)
	debug.sethook()
end

function ldbg()
	local info = debug.getinfo( 2, "Slf" )
	local expr = info.short_src .. ":" .. (info.currentline+1)
	CMD.add_breakpoint( expr, false )
end

--open_debug()