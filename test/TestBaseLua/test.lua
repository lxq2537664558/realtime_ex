function print_r ( t )  
    local print_r_cache={}
    local function sub_print_r(t,indent)
        if (print_r_cache[tostring(t)]) then
            print(indent.."*"..tostring(t))
        else
            print_r_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        print(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
                        print(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        print(indent.."["..pos..'] => "'..val..'"')
                    else
                        print(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                print(indent..tostring(t))
            end
        end
    end
    if (type(t)=="table") then
        print(tostring(t).." {")
        sub_print_r(t,"  ")
        print("}")
    else
        sub_print_r(t,"  ")
    end
    print()
end

print_r(cc)


local pBB = cc.CBB.new(100, 200)

local pAA = cc.funAA(pBB)
print(pAA.a)

function fun_lua(a, b)
	print(a .. "aaaaaaa" .. b)
	return 500
end


function class(classname, super)
	
	local cls = nil

    if super ~= nil then
    	
    	if type(super) ~= "table" then
    		return nil;
    	end
    	if super.__ctype ~= 2 then
    		return nil
    	end

        cls = {}

        cls.super	= super
        cls.__cname = classname
        cls.__ctype = 2
        cls.__index = cls
        
	    function cls.new(...)
	    	local instance = setmetatable({}, cls)
	        instance.class = cls
	        instance:ctor(...)
	       	return instance
	    end

	    setmetatable(cls, super)
    else
    	cls = {}

        cls.ctor	= function() end
        cls.__cname = classname
        cls.__ctype = 2
        cls.__index = cls

        function cls.new(...)
            local instance = setmetatable({}, cls)
            instance.class = cls
            instance:ctor(...)
            return instance
        end
    end

    return cls
end

--[[
CCC = class("CCC")

function CCC:ctor()
	self.a = 100
end

function CCC:fun0()
	print(self.a)
end

DDD = class("DDD", CCC)

function DDD:ctor()
	self.a = 200
end

function DDD:fun0()
	self.super.fun0(self)
	print(self.a)
end

ddd = DDD.new()
print(ddd)
print(DDD)
print(CCC)

ddd:fun0()
]]