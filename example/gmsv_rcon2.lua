require("rcon2")

--[[
	The module also provide values of enum ServerDataRequestType_t

	SERVERDATA_REQUESTVALUE = 0
	SERVERDATA_SETVALUE = 1
	SERVERDATA_EXECCOMMAND = 2
	SERVERDATA_AUTH = 3 // special RCON command to authenticate a connection
	SERVERDATA_VPROF = 4 // subscribe to a vprof stream
	SERVERDATA_REMOVE_VPROF = 5 // unsubscribe from a vprof stream
	SERVERDATA_TAKE_SCREENSHOT = 6
	SERVERDATA_SEND_CONSOLE_LOG = 7
]]

RconAllowList = {
	["192.168.137.5"] = "CoolPasswdInLocalArea",
	["1.3.3.7"] = "AnotherPasswordForYou"
}

RconRestictedCmds = {
	"lua_run"
}

RconAuthList = {}

hook.Add("OnRconCheckPassword", "RCON_IsPassword", function( password, listenerId, isLan, ip, port )
	-- Any error in rcon hooks will cause "Lua Panic" error and server crash
	-- This is simple try\catch style call
	local state, msgOrArgs = pcall( function()
		local passRef = RconAllowList[ ip ]
		if ( !passRef ) then
			-- Return nil to pass check onto engine
			-- Return false to mark password invalid and pass onto engine
			-- Return true to mark password valid and pass auth packet onto engine
			return
		end

		-- For example allow lan computers sending rcon with any or empty passwords
		if ( isLan ) then
			ServerLog(Format("[RCON] Auth from private lan network [%s:%d] successfull!\n", ip, port))
			return true
		end

		-- Check custom password for address entry
		if ( passRef == password ) then
			ServerLog(Format("[RCON] Auth from [%s:%d] successfull!\n", ip, port))
			return true
		end

		ServerLog(Format("[RCON] Auth from [%s:%d] failed!\n", ip, port))
		return false
	end )

	if ( !state ) then
		Error( msgOrArgs )
		return false
	end

	return msgOrArgs
end)

hook.Add("OnRconWriteRequest", "RCON_WriteRequest", function( listenerId, requestId, requestType, data, isLan, ip, port )
	-- Any error in rcon hooks will cause "Lua Panic" error and server crash
	-- This is simple try\catch style call
	local state, msgOrArgs = pcall( function()
		local passRef = RconAllowList[ ip ]

		if ( !passRef ) then
			ServerLog(Format("[RCON] Unauthorized user from [%s:%d]\n", ip, port))
			return
		end

		-- Check for user command
		if ( requestType == SERVERDATA_EXECCOMMAND ) then
			local resticted = false

			for _, cmd in pairs(RconRestictedCmds) do
				if ( string.StartWith(data, cmd) ) then
					resticted = true
					break
				end
			end

			-- Disallow resticted commands from being called
			if ( resticted ) then
				ServerLog(Format("[RCON] User from [%s:%d] trying call resticted command '%s'\n", ip, port, data))

				-- Prevent engine to process this command packet
				return false
			end

			ServerLog(Format("[RCON] User from [%s:%d] execute command '%s'\n", ip, port, data))
		elseif ( requestType == SERVERDATA_AUTH ) then
			ServerLog(Format("[RCON] User from [%s:%d] trying to auth\n", ip, port))
		end
	end )

	if ( !state ) then
		Error( msgOrArgs )
		return false
	end

	return msgOrArgs
end)