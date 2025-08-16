#ifndef RTSENGINE_NETWORKMESSAGES_H
#define RTSENGINE_NETWORKMESSAGES_H


#include <string>
#include <vector>
#include <list> // For std::list if needed, but vector is common for network arrays
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp> // For std::vector
#include <boost/serialization/list.hpp>   // If you choose std::list for players array

// No need for binary_oarchive/iarchive includes here, those are used in Game.cpp

namespace RTSEngine {
    namespace Network {

        // Server to Client: Assign Player ID
        struct AssignPlayerIdMsg {
            int32_t playerId = -1; // Using fixed-size int for clarity with network
            // You can add other initial info here if needed, like max lobby size
            // std::string welcome_message; // Moved to LOBBY_WELCOME for modularity
            // int32_t max_players;

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & playerId;
                // ar & welcome_message;
                // ar & max_players;
            }
        };
		struct LobbyWelcomeMsg {
			std::string welcome_message;
			int32_t your_id = -1;        // Client's assigned player ID
			int32_t max_players = 0;   // Max players for the lobby

			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version) {
				ar & welcome_message;
				ar & your_id;
				ar & max_players;
			}
		};
		struct LobbyPlayerNetInfo {
            int32_t id = -1;
            std::string name = "Unknown";
            bool is_ready = false;
            // Add other displayable info if needed (e.g., chosen_faction_id, color_id)

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & id;
                ar & name;
                ar & is_ready;
            }
        };
		// Server to Client: Acknowledgment for a received ClientCommand
		struct CommandAckMsg {
            std::string original_command_name; // The command name the client sent
            bool success = true;               // Was the command understood/initially accepted?
            std::string status_message;        // e.g., "Order received.", "Invalid target.", "Not enough resources."

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & original_command_name;
                ar & success;
                ar & status_message;
            }
        };
		
		// Server to Client: The client has been kicked
		struct ClientKickedMsg {
			int32_t player_id = -1;
			std::string message;
			
			friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & player_id;
                ar & message;
            }
		};
		
        // --- Server to Client ---
        // We can reuse TextMessagePayload for broadcasting chat
        // Or create a specific one if it needs more info like sender's name from server authority
        struct ServerChatBroadcastMsg {
            int32_t sender_player_id = -1;
            std::string sender_name = "Unknown"; // Server will fill this
            std::string text;

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & sender_player_id;
                ar & sender_name;
                ar & text;
            }
        };
		
		
		struct TextMessagePayload {
            std::string text;

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & text;
            }
        };
		
		struct LoginRequestResponse {
			bool authorized;
			uint32_t auth_token;
			std::string reason;
			
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive &ar, const unsigned int version) {
				ar & authorized;
				ar & auth_token;
				ar & reason;
			}
		};

		// Client to Server: Generic command from client's command line or UI action
        struct ClientCommandMsg {
            std::string command_name;           // e.g., "move", "set_ready", "build_unit"
            std::vector<std::string> args;      // e.g., {"X,Y"} for move, {"true"} for ready, {"barracks_id", "infantry_type"} for build
            std::vector<int32_t> selected_entity_ids; // e.g., unit IDs for a move command, or building ID if command targets a selected building

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & command_name;
                ar & args;
                ar & selected_entity_ids;
            }
        };
		
		struct ClientChatMessageMsg {
            std::string text;

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & text;
            }
        };
		
		struct ClientLoginRequestMsg {
			std::string username;
			std::string password;
			
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive &ar, const unsigned int version) {
				ar & username;
				ar & password;
			}
		};

    } // namespace Network
} // namespace RTSEngine

#endif // RTSENGINE_NETWORKMESSAGES_H