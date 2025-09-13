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

function movePlayer(playerId, system_key, site_key,reset_grid)
	local player = GetPlayerById(playerId)
	local current_site = Systems[player.currentSystem].Sites[player.proximity]
	
	for _, e in ipairs(current_site.entities) do
		if (e.id == playerId) then
			table.remove(Systems[player.currentSystem].Sites[player.proximity].entities, _)
		end
	end
	if (not reset_grid) then
		table.insert(Systems[system_key].Sites[site_key].entities, {id = playerId, location = {x = 0, y = 0}})
	else
		table.insert(Systems[system_key].Sites[site_key].entities, {id = playerId, location = player.on_grid_location})
	end
	if (system_key ~= player.currentSystem) then
		for _, p in ipairs(Systems[player.currentSystem].players) do
			if p == playerId then
				table.remove(Systems[player.currentSystem].players, _)
			end
		end
		table.insert(Systems[system_key].players, playerId)
		GameAPI.sendMessageToPlayer(playerId, "SYSTEM_DATA", {
			system = Systems[system_key]
		})
	end
	GameAPI.sendMessageToPlayer(playerId, "SITE_DATA", {
		site = Systems[system_key].Sites[site_key]
	})
	for _, p in ipairs(Server.players) do
        if p.id == playerId then
			Server.players[_].currentSystem = system_key
			Server.players[_].proximity = site_key
			Server.players[_].target_proximity = ""
			Server.players[_].target_system = ""
			if (not reset_grid) then
				Server.players[_].on_grid_location = { x = 0, y = 0}
			else
			
			end
		end
    end
end
    
function PlayerUpdateTick(current_ms)
    local GRID_MOVE_SPEED = 3 -- Units per tick

    for _a, p_orig in ipairs(Server.players) do
        if p_orig.jumping then
            if (current_ms >= p_orig.jump_end_time) then
                Server.players[_a].jumping = false
                movePlayer(p_orig.id, p_orig.target_system, p_orig.target_proximity)
                GameAPI.sendPlayerCommandAck(p_orig.id, "Navigation", true, "Jump complete.")
            end
        elseif p_orig.warping then
            if (current_ms >= p_orig.warp_end_time) then
                Server.players[_a].warping = false
                movePlayer(p_orig.id, p_orig.currentSystem, p_orig.target_proximity)
                GameAPI.sendPlayerCommandAck(p_orig.id, "Navigation", true, "Warp complete.")
            end
        elseif p_orig.moving_on_grid and p_orig.on_grid_target then
            local current_x = p_orig.on_grid_location.x
            local current_y = p_orig.on_grid_location.y
            local target_x = p_orig.on_grid_target.x
            local target_y = p_orig.on_grid_target.y

            local dx = target_x - current_x
            local dy = target_y - current_y

            local distance_sq = dx*dx + dy*dy

            if distance_sq > (GRID_MOVE_SPEED * GRID_MOVE_SPEED) then -- If not yet at target, move towards it
                local distance = math.sqrt(distance_sq)
                local move_x = (dx / distance) * GRID_MOVE_SPEED
                local move_y = (dy / distance) * GRID_MOVE_SPEED

                Server.players[_a].on_grid_location.x = current_x + move_x
                Server.players[_a].on_grid_location.y = current_y + move_y
				for _f, p in ipairs(Systems[Server.players[_a].currentSystem].Sites[Server.players[_a].proximity].entities) do
					if (p.id == p_orig.id) then
						Systems[Server.players[_a].currentSystem].Sites[Server.players[_a].proximity].entities[_f].location.x = math.floor(Server.players[_a].on_grid_location.x)
						Systems[Server.players[_a].currentSystem].Sites[Server.players[_a].proximity].entities[_f].location.y = math.floor(Server.players[_a].on_grid_location.y)
					end
				end
            else -- Close enough, snap to target
                Server.players[_a].on_grid_location.x = target_x
                Server.players[_a].on_grid_location.y = target_y
                Server.players[_a].moving_on_grid = false
                Server.players[_a].on_grid_target = nil -- Clear the target
                GameAPI.sendPlayerCommandAck(p_orig.id, "Navigation", true, "Arrived at grid target: " .. target_x .. ", " .. target_y)
				for _f, p in ipairs(Systems[Server.players[_a].currentSystem].Sites[Server.players[_a].proximity].entities) do
					if (p.id == p_orig.id) then
						Systems[Server.players[_a].currentSystem].Sites[Server.players[_a].proximity].entities[_f].location.x = math.floor(Server.players[_a].on_grid_location.x)
						Systems[Server.players[_a].currentSystem].Sites[Server.players[_a].proximity].entities[_f].location.y = math.floor(Server.players[_a].on_grid_location.y)
					end
				end
            end
        else
            -- Original reward granting logic, ensuring player isn't actively jumping/warping/moving on grid
            if not (p_orig.jumping or p_orig.warping or p_orig.moving_on_grid) then
                local system = getSystemData(p_orig.currentSystem)
                local site = system.Sites[p_orig.proximity]
                for _b, reward in pairs(site.rewards) do
                    Server.players[_a].xp[_b] = p_orig.xp[_b] + reward
                end
            end
        end
    end
	print("PLAYER_UPDATE_ENDED_AS_EXPECTED")
end

local function floor_coords(coords)
	local floored = {x = 0, y = 0}
	floored.x = math.floor(coords.x)
	floored.y = math.floor(coords.y)
	return floored
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
			explore_xp = p_orig.xp.explore,
			trade_xp = p_orig.xp.trade,
			mining_xp = p_orig.xp.mining,
			diplomacy_xp = 0,
			processing_xp = 0,
			research_xp = 0,
			influence_xp = 0,
			archaeology_xp = 0,
			trade_illegal_xp = 0,
			site_location = floor_coords(site.location),
			on_grid_location = floor_coords(p_orig.on_grid_location),
			site = site
		})
	end
end