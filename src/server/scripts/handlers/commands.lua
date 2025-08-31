function OnGenericPlayerCommand(playerId, commandName, argsTable, selectedIdsTable)
    if commandName == "chat_message" then -- Handle chat from generic if C++ routes it here
        OnPlayerChatMessage(playerId, argsTable)
        return
	elseif commandName == "where" then -- Handle chat from generic if C++ routes it here
        OnPlayerWhereCommand(playerId, argsTable)
        return
	elseif commandName == "jump" then -- Handle chat from generic if C++ routes it here
        OnPlayerJumpCommand(playerId, argsTable)
        return
	elseif commandName == "warp" then -- Handle chat from generic if C++ routes it here
        OnPlayerWarpCommand(playerId, argsTable)
        return
	end
    print("Lua: OnGenericPlayerCommand for P" .. playerId .. " - Unhandled Cmd: " .. commandName)
    GameAPI.sendPlayerCommandAck(playerId, "Server", false, "Unknown command: " .. commandName)
end