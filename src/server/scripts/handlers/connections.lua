function OnPlayerAccepted(playerId, userId, characterId, name, system, proximity, combat_xp, explore_xp)
    --print("Lua: OnPlayerAccepted - Player ID: " .. playerId .. ", User ID: " .. userId .. ", Name: ".. name)

    local newPlayer = _player(playerId, userId, characterId, name, system, proximity, combat_xp, explore_xp)
    table.insert(Server.players, newPlayer)
	table.insert(Systems[system].players, newPlayer.id)
    print("Lua: Player " .. playerId .. " (" .. newPlayer.name .. ") added to Server. Total: " .. #Server.players)

	local system_ = getSystemData(system)
	local site = getSiteData(system, proximity)
	GameAPI.sendMessageToPlayer(playerId, "PLAYER_INFO", {
		you = true,
		name = newPlayer.name,
		system = system_.description,
		proximity = site.description,
		combat_xp = newPlayer.xp.combat,
		explore_xp = newPlayer.xp.explore
	})
end

function OnPlayerDisconnected(playerId)
    print("Lua: OnPlayerDisconnected - Player ID: " .. playerId)
    local playerIndex = -1
    local disconnectedPlayerName = "Player" .. playerId
    for i, p in ipairs(Server.players) do
        if p.id == playerId then
            playerIndex = i
            disconnectedPlayerName = p.name
			GameAPI.writeCharacterData(p.characterId, p.currentSystem, p.proximity, p.xp.combat, p.xp.explore)
            break
        end
    end

    if playerIndex > 0 then
        table.remove(Server.players, playerIndex)
        
    else
        print("Lua Warning: OnPlayerDisconnected for unknown Player ID: " .. playerId)
    end
end