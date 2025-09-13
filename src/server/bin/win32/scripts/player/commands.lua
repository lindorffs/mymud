function OnPlayerChatMessage(playerId, argsTable)
    local sending_player = GetPlayerById(playerId)
    if not sending_player then return end
    local chatText = argsTable and argsTable[1]
    if not chatText or type(chatText) ~= "string" or #chatText == 0 then return end
    
    print("Lua: P" .. playerId .. " (" .. sending_player.name .. ") says: " .. chatText)
	
	for _, receiving_player in pairs(Systems[sending_player.currentSystem].players) do
		GameAPI.sendMessageToPlayer(receiving_player, "CHAT_BROADCAST", {
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
	
	local where_string = "("..tostring(player.on_grid_location.x)..","..tostring(player.on_grid_location.y)..") @ "..
		site.siteName .. "("..tostring(site.location.x)..","..tostring(site.location.y).."), "..
		system.systemName .. "("..tostring(system.location.x)..","..tostring(system.location.y)..")"
	
	GameAPI.sendPlayerCommandAck(playerId, "Navigation", true, where_string)
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
		GameAPI.sendPlayerCommandAck(playerId, "Navigation", false, "Unable to Jump. No System target defined.")
		return
	elseif (#current_site.connections == 0)	then
		GameAPI.sendPlayerCommandAck(playerId, "Navigation", false, "Unable to Jump. No Systems to Jump to.")
		return
	else
		target_system = argsTable[1]
		local distance_to_jumpgate = 0
		for _, object in pairs(current_site.objects) do
			if _ == "Jumpgate" then
				distance_to_jumpgate = math.sqrt((object.location.x - player.on_grid_location.x)^2 + (object.location.y - player.on_grid_location.y)^2)
			end
		end
		if (distance_to_jumpgate >= 3) then
			GameAPI.sendPlayerCommandAck(playerId, "Navigation", false, "You are too far from the jumpgate to activate it. Distance: " .. tostring(distance_to_jumpgate))
			return
		end
		for _, system in pairs(current_site.connections) do
			if (system.target_system == target_system) then
				site_found = true
				local target_system = getSystemData(target_system)
				if (target_system) then
					local target_system_location = target_system.location
					local current_system_location = current_system.location
					local distance = math.sqrt((target_system_location.x - current_system_location.x)^2 + (target_system_location.y - current_system_location.y)^2)
					
					for _, p in ipairs(Server.players) do
						if p.id == playerId then
							Server.players[_].jumping = true
							Server.players[_].target_system = system.target_system
							Server.players[_].target_proximity = system.target_proximity
							Server.players[_].jump_start_time = os.time() * 1000
							Server.players[_].jump_end_time = Server.players[_].jump_start_time + (distance * 1000)/2 -- (system.distance * 1000)/2 = 2 gu/s
						end
					end
					GameAPI.sendPlayerCommandAck(playerId, "Navigation", true, "Jumping to System. " .. tostring(distance/2) .. " Seconds to Landing.")
					return
				end
				site_found = false
			end
		end
	end
	if not (site_found) then
		GameAPI.sendPlayerCommandAck(playerId, "Navigation", true, "Unable to Jump. No Connection to System.")
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
	--print(current_system.Sites)
	if (#argsTable == 0) then
		GameAPI.sendPlayerCommandAck(playerId, "Navigation", false, "Unable to Warp. No System target defined.")
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
						Server.players[_].warp_start_time = os.time() * 1000
						Server.players[_].warp_end_time = Server.players[_].warp_start_time + (2000) -- system.distance * 500 = 2 au/s
					end
				end
				GameAPI.sendPlayerCommandAck(playerId, "Navigation", true, "Warping to Site. " .. "2 Seconds to Landing.")
				return
			end
		end
	end
	if not (site_found) then
		GameAPI.sendPlayerCommandAck(playerId, "Navigation", true, "Unable to Warp. Site Not Found.")
	end
end

function OnPlayerGotoCommand(playerId, argsTable)
    local player = GetPlayerById(playerId)
    if not player then return end

    if #argsTable == 0 then
        GameAPI.sendPlayerCommandAck(playerId, "Navigation", false, "Unable to Goto. No grid target defined.")
        return
    end

    local target_x = tonumber(argsTable[1])
    local target_y = tonumber(argsTable[2])

    if not target_x or not target_y then
        GameAPI.sendPlayerCommandAck(playerId, "Navigation", false, "Unable to Goto. Invalid grid coordinates. Usage: /goto <x> <y>")
        return
    end

    -- Set the player's on_grid_target
    for _a, p in ipairs(Server.players) do
        if p.id == playerId then
            Server.players[_a].on_grid_target = {x = target_x, y = target_y}
            Server.players[_a].moving_on_grid = true -- A flag to indicate we are moving on the grid
            GameAPI.sendPlayerCommandAck(playerId, "Navigation", true, "Moving to grid target: " .. target_x .. ", " .. target_y)
            return
        end
    end
end