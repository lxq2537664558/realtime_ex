function lua_fun2(b)
	print(b)
	coroutine.yield()
end

function lua_fun(a)
	local co = coroutine.create(
	function()
		local pAA = CAA.new(100)
		pAA:fun3(1, 2, 3)
	end)
	
	coroutine.resume(co)
	coroutine.resume(co)
	return 100
end