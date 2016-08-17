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