
--library functions
--library was designed for wrapping c++ classes in lua

-- This file is a system for maintaining selective lua style table access to member variables
-- In a OOP style environment where access to internal variables is not allowed/impossible
-- (see comments below for a better explanation of the library idea)

--now to work with the c++ side of things

--provide for inheritance ???
luaclass = {
	create = function()
		return setmetatable({}, { __call = function(cls,...) return cls.new(...) end })
	end,
	-- takes { name, { get, set }} pair
	make_proxytable = function(proxies)
		if type(proxies) == "table" then
			local t = { get = {}, set = {} }
			for k, v in pairs(proxies) do
				t.get[k] = v[1]
				t.set[k] = v[2]
			end
			return t
		end
	end,
	strictify = function(klass, proxies)
		klass.proxy = luaclass.make_proxytable(proxies)

		klass.__index = function(cls, key)
			if klass.proxy.get[key] then
				return klass.proxy.get[key](cls)
			else
				return klass[key]
			end
		end

		klass.__newindex = function(cls, key, value)
			if klass.proxy.set[key] then
				klass.proxy.set[key](cls, value)
			else
				cls[key] = value
			end
		end
	end,
	vars = setmetatable({}, { __mode = "k" })
}
--
function members(cls)
	return luaclass.vars[cls]
end

-- new function	creates a callable table
function makeCallable()
	return setmetatable({}, { __call = function(self, ...) return self.call(...) end })
end

--
--example
--

--MyClass now has the '__call' method defined
--standard 'classes in lua through tables' design
local MyClass = luaclass.create()

--and the 'new' method '__call' needs
function MyClass.new(init, typ)
	local self = setmetatable({}, MyClass)
	luaclass.vars[self] = {
		value = init,
		ttype = typ
	}
	return self
end

--
--a '__tostring' for debug
MyClass.__tostring = function(cls) return members(cls).value .. " " .. members(cls).ttype end

--several other class methods
--members(self) is sugar for luaclass.vars[self]
--	luaclass stores class private variables in luaclass.vars
--	key is the instance of the class, so every one is unique
--	slightly more 'lua-y' way of hiding variables
--	(that doesn't involve closures, i have my reasons)
--  i should note that this is only relevant for lua only "classes"
--to access static variables, use members([classname])

function MyClass:setval(newval)
	members(self).value = newval
end

function MyClass:getval()
	return members(self).value
end

function MyClass:settype(new_type)
	members(self).ttype = new_type
end

function MyClass:gettype()
	return members(self).ttype
end

--now here comes the hard part of explaining my work (and idea)
--  luaclass was designed for wrapping c++ classes in lua and extended to lua
--  part of the idea for the c++ bindings was to have the ability
--  to directly access some variables without using getters or setters in code
--  'strictify' is how this style is accomplished within pure lua (using a proxy table)
--  'strictify' takes a class and a proxy table ( of form: name = { getter, setter } )
--  and sets the class's '__index' and '__newindex' metamethods to first check
--  inside the proxy table, for behavior in regards to the registered names

luaclass.strictify(MyClass, { value = { MyClass.getval, MyClass.setval } })


-- demonstration
local i = MyClass(5, "test")
local x = i.value			-- "wrapper" for i:getval(), but it doesn't have to be ???
print(x)

i.value	= 1					-- "wrapper" for i:setval(1)
print(i)

i.value = 4					-- since i.value is never actually assigned a value, this syntax remains valid
print(i)

print(i.ttype)				-- nil, since ttype is not a "accessible" variable
print(i.gettype())			-- instead one must use the "OOP" getter




-- possible alternate method for class creation (more coder friendly)

-- instances do not inherit methods from class ???
function addVar(t, methods)
	setmetatable(t, { __call = function(t,...) return t.new(...) end })

	for k,v in pairs(methods) do
		t[k] = v
	end

	luaclass.strictify(t, nil, nil)
end

local logmethods = {
	printmsg = function(self, msg) print("Msg of level "..self.x.." says \""..msg.."\"") end
}

local Log2 = {
	new = function(l)
		return setmetatable({ x = l }, Log2)
	end,
}
addVar(Log2, logmethods)


local y = Log2(3)
Log2.printmsg(y, "Hello")

