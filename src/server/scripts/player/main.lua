function _player(playerId, userId, characterId, name, system, proximity, combat_xp, explore_xp, trade_xp, mining_xp)
	local newPlayer = {
        id = playerId,
		userId = userId,
		characterId = characterId,
        name = name,
        joinTime = os.time(),
		currentSystem = system,
		proximity = proximity,
		target_proximity = "",
		target_system = "",
		on_grid_location = {x = 0, y = 0},
		warping = false,
		warp_start_time = 0,
		jumping = false,
		jump_start_time = 0,
		jump_end_time = 0,
		docked = false,
		xp = {
			combat = combat_xp,
			explore = explore_xp,
			trade = trade_xp,
			mining = mining_xp,
		},
		inventory = {}
    }
	return newPlayer
end

require("scripts.player.commands")
require("scripts.player.logic")