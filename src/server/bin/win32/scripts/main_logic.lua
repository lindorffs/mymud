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
	local current_ms =os.time()* 1000
	local elapsed_tick = current_ms - last_tick
	--local elapsed_client_update = current_ms - last_client_update
	if elapsed_tick > 2000 then
		PlayerUpdateTick(current_ms)
		print(elapsed_tick)
		SendPlayerUpdate()
		print("SERVER UPDATE TICK")
		last_tick =os.time()* 1000
	end
end
