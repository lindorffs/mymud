#include "../Core/Game.h"
namespace RTSEngine {
    namespace Core {
		void Game::setPlayerName_Lua(int playerId, const std::string& name) {
			if (!isServer_) return;
			// Find player session, store name (e.g., in a map<int, PlayerData> on C++)
			// PlayerData could store name, ready status, etc.
			std::cout << "Lua API: Player " << playerId << " name set to " << name << std::endl;
			// Example: Find session and store name on it or associated PlayerData struct
			auto session_it = std::find_if(active_sessions_.begin(), active_sessions_.end(),
				[playerId](const auto& pair){ return pair.first->get_player_id() == playerId; });
			if(session_it != active_sessions_.end()){
				// session_it->first->setPlayerName(name); // If AsioSession has such a method
				// Or update a separate PlayerData map
				// player_data_map_[playerId].name = name;
			}
		}
	}
}