function OnPlayerChatMessage(playerId, argsTable)
    local sending_player = GetPlayerById(playerId)
    if not sending_player then return end
    local chatText = argsTable and argsTable[1]
    if not chatText or type(chatText) ~= "string" or #chatText == 0 then return end
    
    print("Lua: P" .. playerId .. " (" .. sending_player.name .. ") says: " .. chatText)
	
	for _, receiving_player in pairs(Systems[sending_player.currentSystem].players) do
		receiving_player = GetPlayerById(receiving_player)
		GameAPI.sendMessageToPlayer(receiving_player.id, "CHAT_BROADCAST", {
			sender_player_id = playerId,
			sender_name = sending_player.name,
			text = chatText
		})
	end
end

function OnPlayerWhereCommand(playerId, argsTable)
	local player = GetPlayerById(playerId)
    if not player then return end
	
	local system = Systems[player.currentSystem]
	local site = system.Sites[player.proximity]
	
	GameAPI.sendPlayerCommandAck(playerId, "Server", true, "In proximity of: " .. site.description .. ", " .. system.description)
end

function OnPlayerJumpCommand(playerId, argsTable)
	local player = GetPlayerById(playerId)
	if not player then return end
	
	local current_system = Systems[player.currentSystem]
	local current_site = current_system.Sites[player.proximity]
	local target_system = ""
	local site_found = false
	local jump_initiated = false
	-- The player is only able to Jump to a system if they are in proximity of a site that has a connection to their target.
	if (#argsTable == 0) then
		GameAPI.sendPlayerCommandAck(playerId, "Server", false, "Unable to Jump. No System target defined.")
		return
	elseif (#current_site.connections == 0)	then
		GameAPI.sendPlayerCommandAck(playerId, "Server", false, "Unable to Jump. No Systems to Jump to.")
		return
	else
		target_system = argsTable[1]
		for _, system in pairs(current_site.connections) do
			if (system.target_system == target_system) then
				site_found = true
				for _, p in ipairs(Server.players) do
					if p.id == playerId then
						Server.players[_].jumping = true
						Server.players[_].target_system = system.target_system
						Server.players[_].target_proximity = system.target_proximity
						Server.players[_].jump_start_time = os.clock() * 1000
						Server.players[_].jump_end_time = Server.players[_].jump_start_time + (system.distance * 500) -- system.distance * 500 = 2 au/s
					end
				end
				GameAPI.sendPlayerCommandAck(playerId, "Server", true, "Jumping to System. " .. tostring(system.distance / 2) .. " Seconds to Landing.")
				return
			end
		end
	end
	if not (site_found) then
		GameAPI.sendPlayerCommandAck(playerId, "Server", true, "Unable to Jump. No Connection to System.")
	end
end

function OnPlayerWarpCommand(playerId, argsTable)
	local player = GetPlayerById(playerId)
	if not player then return end
	
	local current_system = Systems[player.currentSystem]
	local current_site = current_system.Sites[player.proximity]
	local target_site = ""
	local site_found = false
	local jump_initiated = false
	-- The player is only able to Jump to a system if they are in proximity of a site that has a connection to their target.
	print(current_system.Sites)
	if (#argsTable == 0) then
		GameAPI.sendPlayerCommandAck(playerId, "Server", false, "Unable to Warp. No System target defined.")
		return
	else
		target_site = argsTable[1]
		for site_id, site in pairs(current_system.Sites) do
			if (site_id == target_site) then
				site_found = true
				for _, p in ipairs(Server.players) do
					if p.id == playerId then
						Server.players[_].warping = true
						Server.players[_].target_proximity = site_id
						Server.players[_].warp_start_time = os.clock() * 1000
						Server.players[_].warp_end_time = Server.players[_].warp_start_time + (2000) -- system.distance * 500 = 2 au/s
					end
				end
				GameAPI.sendPlayerCommandAck(playerId, "Server", true, "Warping to Site. " .. "2 Seconds to Landing.")
				return
			end
		end
	end
	if not (site_found) then
		GameAPI.sendPlayerCommandAck(playerId, "Server", true, "Unable to Warp. Site Not Found.")
	end
end