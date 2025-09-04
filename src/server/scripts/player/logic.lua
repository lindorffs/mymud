function GetPlayerById(playerId)
    for _, p in ipairs(Server.players) do
        if p.id == playerId then return p end
    end
    return nil
end

function GetIPSanitizedPlayersTable()
	local sanitized = {}
    for _, p_orig in ipairs(Server.players) do
        local p_copy = {}
        for k,v in pairs(p_orig) do
            if k ~= "ip" and k ~= "joinTime" then -- Don't send IP or joinTime to other clients
                 p_copy[k] = v
            end
        end
        table.insert(sanitized, p_copy)
    end
	return sanitized
end

function movePlayer(playerId, system_key, site_key)
	print(system_key, site_key)
	local player = GetPlayerById(playerId)
	for _, p in ipairs(Systems[player.currentSystem].players) do
		if p == playerId then
			table.remove(Systems[player.currentSystem].players, _)
		end
	end
	table.insert(Systems[system_key].players, playerId)
	for _, p in ipairs(Server.players) do
        if p.id == playerId then
			Server.players[_].currentSystem = system_key
			Server.players[_].proximity = site_key
			Server.players[_].target_proximity = ""
			Server.players[_].target_system = ""
		end
    end
end

function PlayerUpdateTick(current_ms)
	for _a, p_orig in ipairs(Server.players) do
			if p_orig.jumping then
				if (current_ms >= p_orig.jump_end_time) then
					Server.players[_a].jumping = false
					movePlayer(p_orig.id, p_orig.target_system, p_orig.target_proximity)
					GameAPI.sendPlayerCommandAck(p_orig.id, "Server", true, "Jump complete.")
				end
				return
			else if p_orig.warping then
				if (current_ms >= p_orig.warp_end_time) then
					Server.players[_a].warping = false
					movePlayer(p_orig.id, p_orig.currentSystem, p_orig.target_proximity)
					GameAPI.sendPlayerCommandAck(p_orig.id, "Server", true, "Warp complete.")
				end
				return
			else
				local system = getSystemData(p_orig.currentSystem)
				local site = system.Sites[p_orig.proximity]
				for _b, reward in pairs(site.rewards) do
					if not (p_orig.jumping or p_orig.warping) then
						Server.players[_a].xp[_b] = p_orig.xp[_b] + reward
					end
				end
			end
		end
	end
end

function SendPlayerUpdate()
	for _, p_orig in ipairs(Server.players) do
		local system = getSystemData(p_orig.currentSystem)
		local site = system.Sites[p_orig.proximity]
		local proximity = ""
		if p_orig.jumping then
			proximity = "In Jump"
		elseif p_orig.warping then
			proximity = "In Warp"
		else
			proximity = site.siteName
		end
		GameAPI.sendMessageToPlayer(p_orig.id, "PLAYER_INFO", {
			you = true,
			name = p_orig.name,
			system = system.systemName,
			proximity = proximity,
			combat_xp = p_orig.xp.combat,
			explore_xp = p_orig.xp.explore
		})
	end
end