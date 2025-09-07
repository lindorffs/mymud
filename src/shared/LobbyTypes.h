      
#ifndef RTSENGINE_LOBBYTYPES_H
#define RTSENGINE_LOBBYTYPES_H

#include <string>
#include <map> // For use in Game.h
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp> // For std::vector
#include <boost/serialization/list.hpp>   // If you choose std::list for players array

namespace RTSEngine {
    namespace Core { // Or a new namespace like NetworkData or ClientData

        struct LobbyPlayerInfo {
            int id = -1;
            std::string name = "Unknown";
            bool isReady = false;
            // Add other info if needed
        };

        // You could also put ClientGameState enum here if it's broadly used by UI and Game
        
        // New enum for client game states
        enum class ClientGameState {
            MAIN_MENU,
            CONNECTING, // Trying to connect to the server
            IN_LOBBY,
            LOADING_MAP, // After MAP_DATA received, before first game state update with units
            IN_GAME,
            DISCONNECTED, // Or some error state
            QUITTING
        };
		
		struct SiteInformation {
			std::string siteName = "";
			std::string description = "";
			std::vector<std::string> activities;
			
			friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
				ar & siteName;
				ar & description;
				ar & activities;
			}
		};

		struct SystemInformation {
			std::string systemName = "";
			std::string description = "";
			std::vector<std::string> connections;
			std::vector<SiteInformation> sites;
			int x;
			int y;
			
			friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & systemName;
				ar & description;
                ar & connections;
				ar & sites;
				ar & x;
				ar & y;
			}
		};
    } // namespace Core
} // namespace RTSEngine

#endif // RTSENGINE_LOBBYTYPES_H

    