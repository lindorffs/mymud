function OnPlayerAccepted(playerId, userId, characterId, name, system, proximity, combat_xp, explore_xp, trade_xp, mining_xp)
    print("Lua: OnPlayerAccepted - Player ID: " .. playerId .. ", User ID: " .. userId .. ", Name: ".. name .. " TradeXP: " .. trade_xp .. " MiningXP: " .. mining_xp)

    local newPlayer = _player(playerId, userId, characterId, name, system, proximity, combat_xp, explore_xp, trade_xp, mining_xp)
    table.insert(Server.players, newPlayer)
	table.insert(Systems[system].players, newPlayer.id)
    print("Lua: Player " .. playerId .. " (" .. newPlayer.name .. ") added to Server. Total: " .. #Server.players)
	movePlayer(playerId, system, proximity)
	local system_ = getSystemData(system)
	local site = getSiteData(system, proximity)
	GameAPI.sendMessageToPlayer(playerId, "PLAYER_INFO", {
		you = true,
		name = newPlayer.name,
		system = system_.systemName,
		proximity = site.siteName,
		combat_xp = newPlayer.xp.combat,
		explore_xp = newPlayer.xp.explore,
		trade_xp = newPlayer.xp.trade,
		mining_xp = newPlayer.xp.mining,
		diplomacy_xp = 0,
		processing_xp = 0,
		research_xp = 0,
		influence_xp = 0,
		trade_illegal_xp = 0,
		site = site,
	})
	GameAPI.sendMessageToPlayer(playerId, "MAP_DATA", {systems=Systems})
	GameAPI.sendMessageToPlayer(playerId, "SYSTEM_DATA", {
		system = system_
	})
	GameAPI.sendMessageToPlayer(playerId, "SITE_DATA", {
		site = site
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
			GameAPI.writeCharacterData(p.characterId, p.currentSystem, p.proximity, p.xp.combat, p.xp.explore,
			p.xp.trade, p.xp.mining)
            break
        end
    end

    if playerIndex > 0 then
        table.remove(Server.players, playerIndex)
        
    else
        print("Lua Warning: OnPlayerDisconnected for unknown Player ID: " .. playerId)
    end
end
