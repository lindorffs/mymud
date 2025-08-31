require('os')
require('scripts.handlers.main')
require('scripts.systems.main')
require('scripts.player.main')

Server = {
	players = {}
}

last_tick = 0
last_client_update = 0
function UpdateLobbyTick()
	local current_ms = os.clock() * 1000
	local elapsed_tick = current_ms - last_tick
	if elapsed_tick > 1000 then
		PlayerUpdateTick(current_ms)
		SendPlayerUpdate()
		
		print("SERVER UPDATE TICK")
		last_tick = os.clock() * 1000
	end
end
