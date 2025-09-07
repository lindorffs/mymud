#include "Server.h"
#include "../shared/NetworkMessageTypes.h"
#include "../shared/NetworkMessages.h"
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <openssl/md5.h>
#include <iostream>
#include <algorithm>
#include <random>
#include <sstream>

#ifdef _WIN32 // Or #if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace RTSEngine {
    namespace Core {
        Server* g_GameInstanceForConsoleHandler = nullptr;
		
		#ifdef _WIN32
        BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType) {
            switch (dwCtrlType) {
                case CTRL_C_EVENT:
                    std::cout << "Server: CTRL_C_EVENT received. Shutting down." << std::endl;
                    if (g_GameInstanceForConsoleHandler) {
                        g_GameInstanceForConsoleHandler->quitGame();
                    }
                    return TRUE;

                case CTRL_BREAK_EVENT:
                    std::cout << "Server: CTRL_BREAK_EVENT received. Shutting down." << std::endl;
                     if (g_GameInstanceForConsoleHandler) {
                        g_GameInstanceForConsoleHandler->quitGame();
                    }
                    return TRUE;

                case CTRL_CLOSE_EVENT:
                    std::cout << "Server: CTRL_CLOSE_EVENT received. Shutting down." << std::endl;
                     if (g_GameInstanceForConsoleHandler) {
                        g_GameInstanceForConsoleHandler->quitGame();
                    }
                    return TRUE; 

                case CTRL_LOGOFF_EVENT:
                    std::cout << "Server: CTRL_LOGOFF_EVENT received. Shutting down." << std::endl;
                     if (g_GameInstanceForConsoleHandler) {
                        g_GameInstanceForConsoleHandler->quitGame();
                    }
                    return TRUE;

                case CTRL_SHUTDOWN_EVENT:
                    std::cout << "Server: CTRL_SHUTDOWN_EVENT received. Shutting down." << std::endl;
                    if (g_GameInstanceForConsoleHandler) {
                        g_GameInstanceForConsoleHandler->quitGame();
                    }
                    return TRUE;

                default:
                    return FALSE;
            }
        }
#endif
        Server::Server(const std::string& serverIp, unsigned short port)
            : isRunning_(false), currentTicks_(0), serverPort_(port),
              acceptor_(nullptr),
              nextPlayerId_(1),
			  io_context_(),
			  ssl_context_(boost::asio::ssl::context::tlsv12)
              {
				lua_initialized_ = false;
                g_serverShouldContinueRunning = true;
#ifdef _WIN32
                g_GameInstanceForConsoleHandler = this;
                if (!SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE)) {
                    std::cerr << "Server: ERROR - Could not set console control handler: " << GetLastError() << std::endl;
                } else {
                    std::cout << "Server: Console control handler registered." << std::endl;
                }
#endif
		}

        Server::~Server() {
            cleanup();
        }
        void Server::cleanup() {
            isRunning_ = false;

			if (acceptor_ && acceptor_->is_open()) {
				acceptor_->close();
			}
			acceptor_.reset();
						
			for (auto const& [session, _] : active_sessions_) {
				session->do_close();
			}
			active_sessions_.clear();
			
			#ifdef _WIN32
			SetConsoleCtrlHandler(ConsoleHandlerRoutine, FALSE);
			g_GameInstanceForConsoleHandler = nullptr;
			std::cout << "Server: Console control handler unregistered." << std::endl;
			#endif
			
			sqlite3_close(database_);
			
        }
		bool Server::initLua() {
            try {
                std::cout << "Server: Initializing Lua..." << std::endl;
                lua_state_.open_libraries(
                    sol::lib::base,
                    sol::lib::package,
                    sol::lib::string,
                    sol::lib::table,
                    sol::lib::math,
                    sol::lib::os,
                    sol::lib::debug
                );

				sol::table game_api = lua_state_.create_table_with(
					
				);
		
				lua_state_["GameAPI"] = game_api;

				game_api.set_function("broadcastMessage", [this](const std::string& type, const sol::table& data){ this->broadcastMessageToLobby_Lua(type, data); });
				game_api.set_function("sendMessageToPlayer", [this](int pid, const std::string& type, const sol::table& data){ this->sendMessageToPlayer_Lua(pid, type, data); });
				game_api.set_function("setPlayerName", [this](int pid, const std::string& name){ this->setPlayerName_Lua(pid, name); });
				game_api.set_function("sendPlayerCommandAck", [this](int playerId, const std::string& original_command, bool success, const std::string& status_msg){ this->sendCommandAckToPlayer_Lua(playerId, original_command, success, status_msg); });
				game_api.set_function("writeCharacterData", [this](int id, const std::string& system, const std::string& proximity, unsigned int combat_xp, unsigned int explore_xp, unsigned int trade_xp, unsigned int mining_xp) { this->writeCharacterData_Lua(id, system, proximity, combat_xp, explore_xp, trade_xp, mining_xp); });

                lua_state_.script_file("./scripts/main_logic.lua");

                std::cout << "Server: Lua initialized and C++ API bound." << std::endl;
                lua_initialized_ = true;
                return true;

            } catch (const sol::error& e) {
                std::cerr << "Server: Sol3/Lua initialization error: " << e.what() << std::endl;
                return false;
            }
        }
	
        void Server::writeCharacterData_Lua(int id, const std::string& system, const std::string& proximity, unsigned int combat_xp, unsigned int explore_xp, unsigned int trade_xp, unsigned int mining_xp) {
			char *sql_update = "update CHARACTERS set " \
				" SYSTEM = ?,PROXIMITY = ?,XP_COMBAT = ?,XP_EXPLORE = ?" \
				",XP_TRADE=? ,XP_MINING = ?"\
				" where ID = ?;";
			
			sqlite3_stmt *update_stmt;
			
			sqlite3_prepare_v2(database_, sql_update, -1, &update_stmt, NULL);

			
			sqlite3_bind_text(update_stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(update_stmt, 2, proximity.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(update_stmt, 3, combat_xp);
			sqlite3_bind_int(update_stmt, 4, explore_xp);
			sqlite3_bind_int(update_stmt, 5, trade_xp);
			sqlite3_bind_int(update_stmt, 6, mining_xp);
			sqlite3_bind_int(update_stmt, 7, id);
			
			int rc = sqlite3_step(update_stmt);
			
			
			std::cout << "Writing Character " << id << " to database" << std::endl;
			
			if (rc != SQLITE_DONE) {
				fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(database_));
			} else {
				std::cout << "Character " << id << ": saved to database" << std::endl;
			}
			
			
			sqlite3_finalize(update_stmt);
		}

		bool isLuaTableAnArray(sol::table t) {
			if (t.empty()) {
				return true;
			}

			size_t sequence_length = t.size();

			if (sequence_length == 0) {
				for (auto& kvp : t) {
					(void)kvp;
					return false;
				}
				return true;
			}

			size_t actual_key_count = 0;
			for (auto& kvp : t) {
				actual_key_count++;
				sol::object key = kvp.first;
				if (key.get_type() != sol::type::number) {
					return false;
				}
				lua_Integer int_key = key.as<lua_Integer>();
				if (int_key <= 0 || int_key > static_cast<lua_Integer>(sequence_length)) {
					return false;
				}
			}

			return actual_key_count == sequence_length;
		}

		void Server::broadcastMessageToLobby_Lua(const std::string& type_str_from_lua, const sol::table& data_lua_table) {
			Network::NetworkMsgType msg_type_id = Network::NetworkMsgType::MSG_UNKNOWN;
			
			if (type_str_from_lua == "CHAT_BROADCAST") msg_type_id = Network::NetworkMsgType::S2C_CHAT_BROADCAST;
			else {
				std::cerr << "Server (broadcastMessageToLobby_Lua): Unknown message type string from Lua: " << type_str_from_lua << std::endl;
				return;
			}

			std::stringstream ss_payload;
			boost::archive::text_oarchive oa(ss_payload);
			bool serialization_ok = false;

			try {
				if (msg_type_id == Network::NetworkMsgType::S2C_CHAT_BROADCAST) {
                    Network::ServerChatBroadcastMsg msg_data;
					
					sol::optional<int32_t> p_id_opt = data_lua_table["sender_player_id"];
					sol::optional<std::string> p_name_opt = data_lua_table["sender_name"];
					sol::optional<std::string> p_message_opt = data_lua_table["text"];
                    if (p_id_opt && p_name_opt && p_message_opt) {
                        msg_data.sender_player_id = *p_id_opt;
                        msg_data.sender_name = *p_name_opt;
                        msg_data.text = *p_message_opt;
                        oa << msg_data;
                        serialization_ok = true;
                    }
				}
				else {
					std::cerr << "Server (broadcastMessageToLobby_Lua): No specific Boost serialization for type " << type_str_from_lua << std::endl;
				}
			} catch (const sol::error& e) {
				std::cerr << "Server (broadcast): Sol3 error accessing Lua data table for type " << type_str_from_lua << ": " << e.what() << std::endl;
				serialization_ok = false;
			} catch (const boost::archive::archive_exception& e) {
				std::cerr << "Server (broadcast): Boost.Archive serialization error for type " << type_str_from_lua << ": " << e.what() << std::endl;
				serialization_ok = false;
			}


			if (serialization_ok) {
				std::string serialized_boost_payload = ss_payload.str();
				if (serialized_boost_payload.empty()) {
					std::cout << "Server Warning: Serialized boost payload is empty for msg type " << type_str_from_lua << ". Not sending." << std::endl;
					return;
				}


				std::string full_message_to_send_on_wire;
				full_message_to_send_on_wire.reserve(sizeof(msg_type_id) + serialized_boost_payload.length());
				full_message_to_send_on_wire.append(reinterpret_cast<const char*>(&msg_type_id), sizeof(msg_type_id));
				full_message_to_send_on_wire.append(serialized_boost_payload);
				
				std::cout << "Server broadcasting (Type: " << type_str_from_lua
							<< ", ID: " << static_cast<int>(msg_type_id) << "): "
							<< "BoostPayloadSize: " << serialized_boost_payload.length() << std::endl;
				server_broadcast(full_message_to_send_on_wire);
			} else {
				std::cout << "Server: Serialization failed or data missing for message type " << type_str_from_lua << ". Not broadcasting." << std::endl;
			}
		}

		void Server::sendCommandAckToPlayer_Lua(int playerId, const std::string& original_command, bool success, const std::string& status_msg) {
			std::shared_ptr<AsioSession> target_session = nullptr;
			for (auto const& [session_ptr, _] : active_sessions_) {
				if (session_ptr->get_player_id() == playerId) {
					target_session = session_ptr;
					break;
				}
			}

			if (!target_session) {
				std::cerr << "Server (sendCommandAckToPlayer_Lua): Player ID " << playerId << " not found. Cannot send ACK." << std::endl;
				return;
			}

			Network::CommandAckMsg ack_data;
			ack_data.original_command_name = original_command;
			ack_data.success = success;
			ack_data.status_message = status_msg;

			std::stringstream ss_payload;
			Network::NetworkMsgType msg_type_id = Network::NetworkMsgType::S2C_COMMAND_ACK;
			bool serialization_ok = false;

			try {
				boost::archive::text_oarchive oa(ss_payload);
				oa << ack_data;
				serialization_ok = true;
			} catch (const boost::archive::archive_exception& e) {
				std::cerr << "Server Error (sendCommandAckToPlayer_Lua): Failed to serialize CommandAckMsg for P" << playerId << ": " << e.what() << std::endl;
			}

			if (serialization_ok) {
				std::string serialized_boost_payload = ss_payload.str();
				std::string full_message_to_send_on_wire;
				full_message_to_send_on_wire.reserve(sizeof(msg_type_id) + serialized_boost_payload.length());
				full_message_to_send_on_wire.append(reinterpret_cast<const char*>(&msg_type_id), sizeof(msg_type_id));
				full_message_to_send_on_wire.append(serialized_boost_payload);

				std::cout << "Server: Sending COMMAND_ACK to P" << playerId << " for '" << original_command << "', Success: " << success << std::endl;
				target_session->write(full_message_to_send_on_wire);
			}
		}

		void Server::sendMessageToPlayer_Lua(int playerId, const std::string& type_str_from_lua, const sol::table& data_lua_table) {
			std::shared_ptr<AsioSession> target_session = nullptr;
			 for (auto const& [session_ptr, _] : active_sessions_) {
				if (session_ptr->get_player_id() == playerId) {
					target_session = session_ptr;
					break;
				}
			}

			if (target_session) {
				Network::NetworkMsgType msg_type_id = Network::NetworkMsgType::MSG_UNKNOWN;
				if (type_str_from_lua == "LOBBY_WELCOME") msg_type_id = Network::NetworkMsgType::S2C_LOBBY_WELCOME;
				else if (type_str_from_lua == "PLAYER_INFO") msg_type_id = Network::NetworkMsgType::S2C_PLAYER_INFO;
				else if (type_str_from_lua == "CHAT_BROADCAST") msg_type_id = Network::NetworkMsgType::S2C_CHAT_BROADCAST;
				else if (type_str_from_lua == "MAP_DATA") msg_type_id = Network::NetworkMsgType::S2C_MAP_DATA;
				else if (type_str_from_lua == "SYSTEM_DATA") msg_type_id = Network::NetworkMsgType::S2C_SYSTEM_DATA;
				else {
					std::cerr << "Server (sendMessageToPlayer_Lua): Unknown message type string from Lua: " << type_str_from_lua << std::endl;
					return;
				}

				std::stringstream ss_payload;
				boost::archive::text_oarchive oa(ss_payload);

				bool serialization_ok = false;
				if (msg_type_id == Network::NetworkMsgType::S2C_LOBBY_WELCOME) {
					Network::LobbyWelcomeMsg welcome_msg_data;
					
					if (data_lua_table["message"].is<std::string>() &&
						data_lua_table["yourId"].is<int32_t>() &&
						data_lua_table["maxPlayers"].is<int32_t>()) {
						
						welcome_msg_data.welcome_message = data_lua_table["message"].get<std::string>();
						welcome_msg_data.your_id = data_lua_table["yourId"].get<int32_t>();
						welcome_msg_data.max_players = data_lua_table["maxPlayers"].get<int32_t>();
						oa << welcome_msg_data;
						serialization_ok = true;
					} else {
						std::cerr << "Server (sendMessageToPlayer_Lua): LOBBY_WELCOME data table from Lua has incorrect field types or missing fields." << std::endl;

					}
				} else if (msg_type_id == Network::NetworkMsgType::S2C_CHAT_BROADCAST) {
                    Network::ServerChatBroadcastMsg msg_data;
					
					sol::optional<int32_t> p_id_opt = data_lua_table["sender_player_id"];
					sol::optional<std::string> p_name_opt = data_lua_table["sender_name"];
					sol::optional<std::string> p_message_opt = data_lua_table["text"];
                    if (p_id_opt && p_name_opt && p_message_opt) {
                        msg_data.sender_player_id = *p_id_opt;
                        msg_data.sender_name = *p_name_opt;
                        msg_data.text = *p_message_opt;
                        oa << msg_data;
                        serialization_ok = true;
                    }
				} else if (msg_type_id == Network::NetworkMsgType::S2C_PLAYER_INFO) {
					Network::PlayerNetInfo msg_data;
					
					sol::optional<bool> p_you_opt = data_lua_table["you"];
					sol::optional<std::string> p_name_opt = data_lua_table["name"];
					sol::optional<std::string> p_system_opt = data_lua_table["system"];
					sol::optional<std::string> p_proximity_opt = data_lua_table["proximity"];
					sol::optional<unsigned int> p_combat_xp_opt = data_lua_table["combat_xp"];
					sol::optional<unsigned int> p_explore_xp_opt = data_lua_table["explore_xp"];
					sol::optional<unsigned int> p_xp_trade_opt = data_lua_table["trade_xp"];
					sol::optional<unsigned int> p_xp_mining_opt = data_lua_table["mining_xp"];
					sol::optional<unsigned int> p_xp_diplomacy_opt = data_lua_table["diplomacy_xp"];
					sol::optional<unsigned int> p_xp_processing_opt = data_lua_table["processing_xp"];
					sol::optional<unsigned int> p_xp_research_opt = data_lua_table["research_xp"];
					sol::optional<unsigned int> p_xp_influence_opt = data_lua_table["influence_xp"];
					sol::optional<unsigned int> p_xp_archaeology_opt = data_lua_table["archaeology_xp"];
					sol::optional<unsigned int> p_xp_trade_illegal_opt = data_lua_table["trade_illegal_xp"];
					sol::optional<sol::table> p_on_grid_location_opt = data_lua_table["on_grid_location"];
					
					if (p_on_grid_location_opt) {
						sol::table on_grid_location = *p_on_grid_location_opt;
						int x = on_grid_location["x"];
						int y = on_grid_location["y"];
						msg_data.grid_x = x;
						msg_data.grid_y = y;
					} else {
						msg_data.grid_x = 0;
						msg_data.grid_y = 0;
					}
					
					msg_data.you = *p_you_opt;
					msg_data.name = *p_name_opt;
					msg_data.system = *p_system_opt;
					msg_data.proximity = *p_proximity_opt;
					msg_data.combat_xp = *p_combat_xp_opt;
					msg_data.explore_xp = *p_explore_xp_opt;
					msg_data.trade_xp = *p_xp_trade_opt;
					msg_data.mining_xp = *p_xp_mining_opt;
					msg_data.diplomacy_xp = *p_xp_diplomacy_opt;
					msg_data.processing_xp = *p_xp_processing_opt;
					msg_data.research_xp = *p_xp_research_opt;
					msg_data.influence_xp = *p_xp_influence_opt;
					msg_data.archaeology_xp = *p_xp_archaeology_opt;
					msg_data.trade_illegal_xp = *p_xp_trade_illegal_opt;
					
					oa << msg_data;
					serialization_ok = true;
				} else if (msg_type_id == Network::NetworkMsgType::S2C_MAP_DATA) {
					Network::MapDataPayload msg_data;

					sol::optional<sol::table> systems_table_opt = data_lua_table["systems"];

					if (systems_table_opt) {
						sol::table systems_table = *systems_table_opt;

						// 2. Iterate over the key-value pairs in the 'Systems' table.
						//    (e.g., key="Sol", value={ description="...", connections={...} })
						for (const auto& kvp : systems_table) {
							// Ensure the key is a string and the value is a table (the system object).
							if (kvp.first.is<std::string>() && kvp.second.is<sol::table>()) {
								
								Core::SystemInformation current_system;
								current_system.systemName = kvp.first.as<std::string>();

								// 3. Get the inner table which holds the description and connections.
								sol::table system_object_table = kvp.second.as<sol::table>();

								sol::optional<sol::table> location_table_opt = system_object_table["location"];
								if (location_table_opt) {
									sol::table location_table = *location_table_opt;
									current_system.x = location_table["x"];
									current_system.y = location_table["y"];
								}

								// 4. Safely parse the 'description' string from the inner table.
								sol::optional<std::string> desc_opt = system_object_table["description"];
								if (desc_opt) {
									current_system.description = *desc_opt;
								}

								sol::optional<sol::table> sites_table_opt = system_object_table["Sites"];
								if (sites_table_opt) {
									sol::table sites_table = *sites_table_opt;

									// Iterate through the array of site objects
									for (const auto& site_kvp : sites_table) {
										//std::cerr << "Got site!" << std::endl;
										// The value is now a table representing a site object, not a string.
										if (site_kvp.second.is<sol::table>()) {
											sol::table site_object = site_kvp.second.as<sol::table>();

											// Create a new C++ Site object to populate
											Core::SiteInformation new_site;

											// Safely get the description string
											new_site.siteName = site_object["siteName"].get_or<std::string>("Anomally");
											new_site.description = site_object["description"].get_or<std::string>("");

											// Add the fully parsed Site object to the system's vector of sites
											current_system.sites.push_back(new_site);
										} else {
											std::cerr << "S2C_MAP_DATA Error: Malformed site data." << std::endl; 	
										}
									}
								}

								// 6. Add the fully populated system object to our payload.
								msg_data.systems.push_back(current_system);

							} else {
								std::cerr << "S2C_MAP_DATA Error: Malformed entry in 'systems' table. Key must be string, value must be table." << std::endl;
							}
						}

						// 7. If we've successfully processed the table, serialize the data.
						oa << msg_data;
						serialization_ok = true;

					}
				} else if (msg_type_id == Network::NetworkMsgType::S2C_SYSTEM_DATA) {
					Network::SystemDataPayload msg_data;
					
					sol::optional<sol::table> system_object_table_opt = data_lua_table["system"];
					sol::table system_object_table = *system_object_table_opt;
					sol::optional<sol::table> sites_table_opt = system_object_table["Sites"];
					if (sites_table_opt) {
						sol::table sites_table = *sites_table_opt;

						// Iterate through the array of site objects
						for (const auto& site_kvp : sites_table) {
							//std::cerr << "Got site!" << std::endl;
							// The value is now a table representing a site object, not a string.
							if (site_kvp.second.is<sol::table>()) {
								sol::table site_object = site_kvp.second.as<sol::table>();

								// Create a new C++ Site object to populate
								Network::SiteSystemData site_data;

								// Safely get the description string
								site_data.name = site_object["siteName"].get_or<std::string>("Anomally");
								sol::optional<sol::table> location_table_opt = site_object["location"];
								sol::table on_grid_location = *location_table_opt;
								site_data.x = on_grid_location["x"];
								site_data.y = on_grid_location["y"];

								// Add the fully parsed Site object to the system's vector of sites
								msg_data.sites.push_back(site_data);
							} else {
								std::cerr << "S2C_MAP_DATA Error: Malformed site data." << std::endl; 	
							}
						}
						
					}
					oa << msg_data;
					serialization_ok = true;
				} else {
					std::cerr << "Server (sendMessageToPlayer_Lua): No specific Boost serialization implemented for type " << type_str_from_lua << std::endl;
				}

				if (serialization_ok) {
					std::string serialized_boost_payload = ss_payload.str();

					std::string full_message_to_send_on_wire;
					full_message_to_send_on_wire.reserve(sizeof(msg_type_id) + serialized_boost_payload.length());
					full_message_to_send_on_wire.append(reinterpret_cast<const char*>(&msg_type_id), sizeof(msg_type_id));
					full_message_to_send_on_wire.append(serialized_boost_payload);
					
					std::cout << "Server sending to P" << playerId << " (Type: " << type_str_from_lua
							  << ", ID: " << static_cast<int>(msg_type_id) << "): "
							  << "BoostPayloadSize: " << serialized_boost_payload.length() << std::endl;
					target_session->write(full_message_to_send_on_wire);
				} else {
					 std::cout << "Server: Serialization failed for message type " << type_str_from_lua << " to P" << playerId << std::endl;
				}
			}  else {
				std::cerr << "Lua API: sendMessageToPlayer_Lua - Player ID " << playerId << " not found." << std::endl;
			}
		}

        bool Server::initAsioNetworking() {
            try {
				ssl_context_.use_certificate_chain_file("server.crt");
				ssl_context_.use_private_key_file("server.key", asio::ssl::context::pem);
				acceptor_ = std::make_unique<tcp::acceptor>(io_context_, tcp::endpoint(tcp::v4(), serverPort_));
				std::cout << "Server listening on port " << serverPort_ << std::endl;
				start_accept();
            } catch (const std::exception& e) {
                std::cerr << "Asio Networking Init Error: " << e.what() << std::endl;
                return false;
            }
            io_thread_ = std::thread([this]() {
                try {
                    io_context_.run();
                } catch (const std::exception& e) {
                    std::cerr << "io_context thread exception: " << e.what() << std::endl;
					return false;
                }
            });
            return true;
        }

		void Server::start_accept() {
			acceptor_->async_accept(
			[this](boost::system::error_code ec, tcp::socket socket) {
				if (!ec) {
					auto new_session = std::make_shared<AsioSession>(
						asio::ssl::stream<tcp::socket>(std::move(socket), ssl_context_),
						*this,
						false
					);

					active_sessions_[new_session] = true;

					int new_player_id = nextPlayerId_++; 
					new_session->set_player_id(new_player_id);
					new_session->start();
				}

				start_accept();
			});
		}

        void Server::server_remove_session(std::shared_ptr<AsioSession> session) {

            auto it = active_sessions_.find(session);
            if (it != active_sessions_.end()) {
                int leavingPlayerId = session->get_player_id();
                std::cout << "Server: Player " << leavingPlayerId << " disconnected." << std::endl;
                active_sessions_.erase(it);
				if (lua_initialized_) {
					try {
						sol::protected_function lua_on_disconnect = lua_state_["OnPlayerDisconnected"];
						if (lua_on_disconnect.valid()) {
							sol::protected_function_result result = lua_on_disconnect(leavingPlayerId);
							if (!result.valid()) { /* log error */ }
						} else { std::cerr << "Server: Lua function OnPlayerDisconnected not found." << std::endl; }
					} catch (const sol::error& e) { /* log error */ }
				}
            }
        }

		void Server::enqueue_server_message(std::shared_ptr<AsioSession> session, std::string&& msg) {
            std::lock_guard<std::mutex> lock(server_msg_mutex_);
            server_received_messages_.emplace_back(session, std::move(msg));
        }

		std::string calculateMD5(const std::string& input) {
			unsigned char digest[MD5_DIGEST_LENGTH];
			MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), digest);

			char mdString[33];
			for (int i = 0; i < 16; i++) {
				sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
			}
			return std::string(mdString);
		}

        void Server::server_process_incoming_messages() {
            std::deque<std::pair<std::shared_ptr<AsioSession>, std::string>> current_batch;
            {
                std::lock_guard<std::mutex> lock(server_msg_mutex_);
                current_batch.swap(server_received_messages_);
            }

            for (auto& SourcedMessage : current_batch) {
                std::shared_ptr<AsioSession>& session = SourcedMessage.first;
                std::string& full_msg_content_from_asio = SourcedMessage.second;
                int playerId = session->get_player_id();

                if (full_msg_content_from_asio.length() < sizeof(Network::NetworkMsgType)) {
                    std::cerr << "Server Error: Received message too short for Type ID from P" << playerId
                              << ". Length: " << full_msg_content_from_asio.length() << std::endl;
                    continue;
                }

                Network::NetworkMsgType msg_type_id;
                std::memcpy(&msg_type_id, full_msg_content_from_asio.data(), sizeof(Network::NetworkMsgType));

                std::string serialized_payload_data = full_msg_content_from_asio.substr(sizeof(Network::NetworkMsgType));
                std::stringstream ss_in(serialized_payload_data);
                
                try {
                    boost::archive::text_iarchive ia(ss_in);

                    switch (msg_type_id) {
						case Network::NetworkMsgType::C2S_LOGIN_REQUEST: {
							Network::ClientLoginRequestMsg received_login_request;
							ia >> received_login_request;
							
							std::string username = received_login_request.username;
							std::string challenged_password_hash = calculateMD5(received_login_request.password);
							std::cout << "Recieved login request for: " << username << std::endl;
							
							sqlite3_stmt *login_stmt;
							std::string login_query = "SELECT password_hash, id FROM users WHERE username = ? LIMIT 1;";
							
							sqlite3_prepare_v2(database_, login_query.c_str(), -1, &login_stmt, NULL);
							
							sqlite3_bind_text(login_stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
							int rc = sqlite3_step(login_stmt);
							bool user_found = false;
							bool authorized = false;
							int user_id = 0;
							while (rc != SQLITE_DONE) {
								if (rc == SQLITE_ROW) {
									user_found = true;
									const unsigned char *password_hash = sqlite3_column_text(login_stmt, 0);
									if (password_hash) {
										std::string password_hash_from_db(reinterpret_cast<const char*>(password_hash));
										
										authorized = (challenged_password_hash == password_hash_from_db);
										if (authorized) {
											user_id = sqlite3_column_int(login_stmt,1 );
										}
									}
								}
								rc = sqlite3_step(login_stmt);
							}
							sqlite3_finalize(login_stmt);
							if (!user_found) {
								rc = 0;
								
								sqlite3_stmt *new_user_stmt;
								char *sql_insert_new_user = "INSERT INTO USERS (USERNAME,PASSWORD_HASH) VALUES (?, ?);";
								
								rc = sqlite3_prepare_v2(database_, sql_insert_new_user, -1, &new_user_stmt, NULL);
								sqlite3_bind_text(new_user_stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);		
								sqlite3_bind_text(new_user_stmt, 2, challenged_password_hash.c_str(), -1, SQLITE_TRANSIENT);

								rc = sqlite3_step(new_user_stmt);
								if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
									std::cout << "Failed to create new user: " << username.c_str() << std::endl;
									fprintf(stderr, "Error during sqlite3_step(): Code %d - %s\n", sqlite3_errcode(database_), sqlite3_errmsg(database_));
									sqlite3_finalize(new_user_stmt);
									authorized = false;
								} else {
									sqlite3_finalize(new_user_stmt);
									sqlite3_stmt *new_user_id_statement;
									std::string user_id_query = "SELECT id FROM users WHERE username = ? LIMIT 1;";
									sqlite3_prepare_v2(database_, user_id_query.c_str(), -1, &new_user_id_statement, NULL);
									sqlite3_bind_text(new_user_id_statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);
									rc = sqlite3_step(new_user_id_statement);
									if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
										std::cout << "Failed to get new user id for: " << username.c_str() << std::endl;
										fprintf(stderr, "Error during sqlite3_step(): Code %d - %s\n", sqlite3_errcode(database_), sqlite3_errmsg(database_));
										sqlite3_finalize(new_user_id_statement);
										authorized = false;
									} else {
										int new_user_id = sqlite3_column_int(new_user_id_statement,0 );
										sqlite3_finalize(new_user_id_statement);
										sqlite3_stmt *new_char_stmt;
										char *sql_insert_new_char = "INSERT INTO CHARACTERS (USER_ID," \
										   "NAME,"\
										   "NEW,"\
										   "SYSTEM,"\
										   "PROXIMITY,"\
										   "XP_COMBAT,"\
										   "XP_EXPLORE,"\
										   "XP_TRADE,"\
										   "XP_MINING,"\
										   "XP_DIPLOMACY,"\
										   "XP_PROCESSING,"\
										   "XP_RESEARCH,"\
										   "XP_INFLUENCE,"\
										   "XP_ARCHAEOLOGY,"\
										   "XP_TRADE_ILLEGAL"\
										   ")"\
										   "VALUES (?, ?, True, 'Cygnus_Prime', 'State_Military_Academy', 0, 100, 0, 0, 0, 0, 0, 0, 0, 0);";
										sqlite3_prepare_v2(database_, sql_insert_new_char, -1, &new_char_stmt, NULL);
										sqlite3_bind_int(new_char_stmt, 1, new_user_id);
										sqlite3_bind_text(new_char_stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
										rc = sqlite3_step(new_char_stmt);
										if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
											std::cout << "Failed to create new character for: " << username.c_str() << std::endl;
											fprintf(stderr, "Error during sqlite3_step(): Code %d - %s\n", sqlite3_errcode(database_), sqlite3_errmsg(database_));
											sqlite3_finalize(new_char_stmt);
											authorized = false;
										} else {
											std::cout << "Created new character for: " << username.c_str() << std::endl;
											sqlite3_finalize(new_char_stmt);
											authorized = true;
											user_id = new_user_id;
										}
									}
								}									
							}
							if (authorized) {
								try {
									const unsigned char *name_from_db;
									const unsigned char *system_from_db;
									const unsigned char *proximity_from_db;
									int character_id; 
									
									std::string character_name;
									std::string system_string;
									std::string proximity_string;
									unsigned int xp_combat;
									unsigned int xp_explore;
									unsigned int xp_trade;
									unsigned int xp_mining;
									unsigned int xp_diplomacy;
									unsigned int xp_processing;
									unsigned int xp_research;
									unsigned int xp_influence;
									unsigned int xp_archaeology;
									unsigned int xp_trade_illegal;
									sqlite3_stmt *character_stmt;

									std::string query = "SELECT ID, NAME, SYSTEM, PROXIMITY, XP_COMBAT, XP_EXPLORE "\
									" XP_TRADE,"\
									" XP_MINING"\
									" FROM characters WHERE user_id = ? LIMIT 1;";
									
									sqlite3_prepare_v2(database_, query.c_str(), -1, &character_stmt, NULL);
									sol::protected_function lua_on_connect = lua_state_["OnPlayerAccepted"];
									
									
									sqlite3_bind_int(character_stmt, 1, user_id);
									rc = sqlite3_step(character_stmt);
									
									while (rc != SQLITE_DONE) {
										if (rc == SQLITE_ROW) {
											name_from_db = sqlite3_column_text(character_stmt, 1);
											character_name = reinterpret_cast<const char*>(name_from_db);
											system_from_db = sqlite3_column_text(character_stmt, 2);
											system_string = reinterpret_cast<const char*>(system_from_db);
											proximity_from_db = sqlite3_column_text(character_stmt, 3);
											proximity_string = reinterpret_cast<const char*>(proximity_from_db);
											xp_combat = sqlite3_column_int(character_stmt, 4);
											xp_explore = sqlite3_column_int(character_stmt, 5);
											xp_trade = sqlite3_column_int(character_stmt, 6);
											xp_mining = sqlite3_column_int(character_stmt, 7);
											character_id = sqlite3_column_int(character_stmt, 0);
										}
										rc = sqlite3_step(character_stmt);
									}
									sqlite3_finalize(character_stmt);
									std::cout << "Found character! " << character_name << ":"<<character_id<<", " << system_string << ", " << proximity_string << std::endl;
									if (lua_on_connect.valid()) {
										sol::protected_function_result result = lua_on_connect(playerId, user_id, character_id, character_name, system_string, proximity_string,
										xp_combat, xp_explore, xp_trade, xp_mining);
										if (!result.valid()) {
											sol::error err = result;
											std::cerr << "Server: Lua error in OnPlayerAccepted: " << err.what() << std::endl;
										}
									} else { std::cerr << "Server: Lua function OnPlayerAccepted not found." << std::endl; }
								} catch (const sol::error& e) { std::cerr << "Server: Sol3 error calling OnPlayerAccepted: " << e.what() << std::endl; }
							} else {
								this->kickPlayer_Lua(playerId, "Unable to authenticate.");
							}
							break;
						}
                        case Network::NetworkMsgType::C2S_CLIENT_COMMAND:
                        {
                            Network::ClientCommandMsg received_cmd_data;
                            ia >> received_cmd_data;

                            std::string commandName = received_cmd_data.command_name;

                            std::cout << "Server: Received C2S_CLIENT_COMMAND from P" << playerId
                                      << ": Cmd='" << commandName
                                      << "', ArgsCount=" << received_cmd_data.args.size()
                                      << ", SelectedIDsCount=" << received_cmd_data.selected_entity_ids.size() << std::endl;

                            if (!lua_initialized_) {
                                std::cerr << "Server Error: Lua not initialized. Cannot process command '" << commandName << "' from P" << playerId << std::endl;
                                break;
                            }

                            sol::table lua_args_table = lua_state_.create_table(received_cmd_data.args.size());
                            for (size_t i = 0; i < received_cmd_data.args.size(); ++i) {
                                lua_args_table[i + 1] = received_cmd_data.args[i];
                            }

                            sol::table lua_selected_ids_table = lua_state_.create_table(received_cmd_data.selected_entity_ids.size());
                            for (size_t i = 0; i < received_cmd_data.selected_entity_ids.size(); ++i) {
                                lua_selected_ids_table[i + 1] = received_cmd_data.selected_entity_ids[i];
                            }

							sol::protected_function generic_lua_handler = lua_state_["OnGenericPlayerCommand"];
							if (generic_lua_handler.valid()) {
								sol::protected_function_result res = generic_lua_handler(playerId, commandName, lua_args_table, lua_selected_ids_table);
								if (!res.valid()) { sol::error err = res; std::cerr << "Lua error (OnGenericPlayerCommand for '" << commandName << "'): " << err.what() << std::endl; }
							} else {
								std::cout << "Server: No specific or generic Lua handler found for command '" << commandName << "' from P" << playerId << std::endl;
							}

                        }
                        break;
                        case Network::NetworkMsgType::S2C_ASSIGN_PLAYER_ID:
                        case Network::NetworkMsgType::S2C_LOBBY_WELCOME:
                            std::cerr << "Server Warning: Received an S2C message type (" << static_cast<int>(msg_type_id)
                                      << ") from Player " << playerId << ". Ignoring." << std::endl;
                            break;
                        default:
                            std::cerr << "Server: Received unhandled/unknown Boost.Archive message type ID from P" << playerId
                                      << ": " << static_cast<int>(msg_type_id) << std::endl;
                            break;
                    }
                } catch (const boost::archive::archive_exception& e) {
                    std::cerr << "Server Error: Deserialization failed for P" << playerId
                              << ", TypeID " << static_cast<int>(msg_type_id) << ". Error: " << e.what()
                              << ". Payload snippet: '" << serialized_payload_data.substr(0, std::min(100, (int)serialized_payload_data.length())) << "'" << std::endl;
                    // This could indicate corrupted data, or a mismatch in serialized struct definitions
                    // between when the client sent it and what the server expects for ClientCommandMsg.
                } catch (const sol::error& e_sol) { // Catch Sol3 errors during Lua dispatch
                    std::cerr << "Server Error: Exception during LUA command dispatch for P" << playerId
                              << ", TypeID " << static_cast<int>(msg_type_id) << ". Error: " << e_sol.what() << std::endl;
                } catch (const std::exception& e_std) { // Catch other standard exceptions
                    std::cerr << "Server Error: Standard exception while processing message for P" << playerId
                              << ", TypeID " << static_cast<int>(msg_type_id) << ". Error: " << e_std.what() << std::endl;
                }
            } // End of for loop over current_batch
        } // End of Game::server_process_incoming_messages()
		
        void Server::server_broadcast(const std::string& message, std::shared_ptr<AsioSession> exclude_session) {
            for (auto const& [session_ptr, _] : active_sessions_) {
                if (session_ptr != exclude_session) {
                    session_ptr->write(message);
                }
            }
        }
        
        void Server::server_send_to_session(std::shared_ptr<AsioSession> session, const std::string& message) {
            if (session) {
                session->write(message);
            }
        }

		void Server::setPlayerName_Lua(int playerId, const std::string& name) {
			std::cout << "Lua API: Player " << playerId << " name set to " << name << std::endl;
			auto session_it = std::find_if(active_sessions_.begin(), active_sessions_.end(),
				[playerId](const auto& pair){ return pair.first->get_player_id() == playerId; });
			if(session_it != active_sessions_.end()){
				
			}
		}
		bool Server::kickPlayer_Lua(int playerId, const std::string& reason) {
			std::shared_ptr<AsioSession> session_to_kick = nullptr;
			for (const auto& session_pair : active_sessions_) {
				if (session_pair.first && session_pair.first->get_player_id() == playerId) {
					session_to_kick = session_pair.first;
					break;
				}
			}

			if (session_to_kick) {
				std::cout << "Server: Preparing to kick Player " << playerId << " for reason: " << reason << std::endl;

				Network::ClientKickedMsg kick_msg_data;
				kick_msg_data.message = reason;

				std::stringstream ss_payload_stream;
				Network::NetworkMsgType msg_type_id = Network::NetworkMsgType::S2C_CLIENT_KICKED;
				
				try {
					ss_payload_stream.write(reinterpret_cast<const char*>(&msg_type_id), sizeof(msg_type_id));
					boost::archive::text_oarchive oa(ss_payload_stream);
					oa << kick_msg_data;
				} catch (const boost::archive::archive_exception& e) {
					std::cerr << "Server Error (kickPlayer_Lua): Failed to serialize kick message for P" << playerId << ". Error: " << e.what() << std::endl;
				}

				std::string full_message_to_send = ss_payload_stream.str();
				
				session_to_kick->write(full_message_to_send);

				server_remove_session(session_to_kick);

				return true;
			} else {
				std::cerr << "Server Warning (kickPlayer_Lua): Could not find Player " << playerId << " to kick." << std::endl;
				return false;
			}
		}

		void consoleInputLoop(Server* server_ptr) {
            std::string line;
            std::cout << "Server Console: Type 'quit' or 'exit' to shut down the server." << std::endl;

            while (server_ptr->g_serverShouldContinueRunning.load()) {
                if (std::getline(std::cin, line)) {
                    if (!server_ptr->g_serverShouldContinueRunning.load()) break;

                    std::transform(line.begin(), line.end(), line.begin(),
                                   [](unsigned char c){ return std::tolower(c); });

                    if (line == "quit" || line == "exit") {
                        std::cout << "Server Console: Shutdown command '" << line << "' received." << std::endl;
                        if (server_ptr) {
                            server_ptr->quitGame();
                        }
                        server_ptr->g_serverShouldContinueRunning = false;
                        break;
                    } else if (!line.empty()) {
                        std::cout << "Server Console: Unknown command '" << line << "'. Type 'quit' or 'exit'." << std::endl;
                    }
                } else {
                    if (!server_ptr->g_serverShouldContinueRunning.load()) break;

                    if (std::cin.eof()) {
                        if (server_ptr) server_ptr->quitGame();
                        server_ptr->g_serverShouldContinueRunning = false;
                        break;
                    } else if (std::cin.bad() || std::cin.fail()) {
                        std::cerr << "Server Console: Error reading from stdin. Console input disabled." << std::endl;
                        server_ptr->g_serverShouldContinueRunning = false;
                        break;
                    }
                }
				
                if (server_ptr->g_serverShouldContinueRunning.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Yield a bit
                }
            }
            std::cout << "Server Console: Input thread exiting." << std::endl;
        }

        void Server::serverGameLoop() {
			auto now = std::chrono::system_clock::now();
			#ifdef WIN32
            uint64_t currentTicks_ = std::chrono::duration_cast<std::chrono::milliseconds>(
				now.time_since_epoch()
			).count();
			#else			
            u_int64_t currentTicks_ = std::chrono::duration_cast<std::chrono::milliseconds>(
				now.time_since_epoch()
			).count();
			#endif
            std::cout << "Server game loop started." << std::endl;
			std::thread consoleInputThread;
            consoleInputThread = std::thread(consoleInputLoop, this);
			while (isRunning_) {
				server_process_incoming_messages();

				if (lua_initialized_) {
					try {
						sol::protected_function lua_update_lobby = lua_state_["UpdateLobbyTick"];
						if (lua_update_lobby.valid()) {
							sol::protected_function_result result = lua_update_lobby(1);
							if (!result.valid()) { /* log error */ }
						}
					} catch (const sol::error& e) { /* log error */ }
				}
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
			std::cout << "Server game loop ended. Signaling console input thread to exit if still running." << std::endl;
            g_serverShouldContinueRunning = false; 

            if (consoleInputThread.joinable()) {
                if (consoleInputThread.get_id() != std::thread::id()) {
                    std::cout << "Server: Attempting to join console input thread..." << std::endl;
                    consoleInputThread.join();
                    std::cout << "Server: Console input thread joined." << std::endl;
				}
			}
        }
		    
        static int callback(void *data, int argc, char **argv, char **azColName) {
			int i;
			fprintf(stderr, "%s: ", (const char*)data);
			for (i = 0; i < argc; i++) {
				printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			}
			printf("\n");
			return 0;
		}
        int Server::run() {
			if (!initAsioNetworking()) { cleanup(); return 1; }
			if (!initLua()) { cleanup(); return 1; }
			if(sqlite3_open("seed.db", &database_)) { cleanup(); return 1; } else {
				
				char *zErrMsg = 0;
				char *sql_users_create = "CREATE TABLE IF NOT EXISTS USERS(" \
				   "ID INTEGER PRIMARY 		KEY     AUTOINCREMENT," \
				   "USERNAME           		TEXT    NOT NULL," \
				   "PASSWORD_HASH           TEXT    NOT NULL,"\
				   "ADMIN					BOOL);";
				
				if (sqlite3_exec(database_, sql_users_create, callback, 0, &zErrMsg) != SQLITE_OK) {
					fprintf(stderr, "SQL error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				} else {
					fprintf(stdout, "Users Table created successfully\n");
				}
				
				char *sql_characters_create = "CREATE TABLE IF NOT EXISTS CHARACTERS(" \
				   "ID INTEGER PRIMARY KEY     AUTOINCREMENT," \
				   "USER_ID           INT    NOT NULL," \
				   "NAME           TEXT     NOT NULL,"\
				   "NEW				BOOL	NOT NULL,"\
				   "SYSTEM			TEXT	NOT NULL,"\
				   "PROXIMITY		TEXT	NOT NULL,"\
				   "XP_COMBAT		INT		NOT NULL,"\
				   "XP_EXPLORE		INT		NOT NULL,"\
				   "XP_TRADE		INT		NOT NULL,"\
				   "XP_MINING		INT		NOT NULL,"\
				   "XP_DIPLOMACY	INT		NOT NULL,"\
				   "XP_PROCESSING	INT		NOT NULL,"\
				   "XP_RESEARCH		INT		NOT NULL,"\
				   "XP_INFLUENCE	INT		NOT NULL,"\
				   "XP_ARCHAEOLOGY	INT		NOT NULL,"\
				   "XP_TRADE_ILLEGAL INT	NOT NULL"\
				   ");";
				
				if (sqlite3_exec(database_, sql_characters_create, callback, 0, &zErrMsg) != SQLITE_OK) {
					fprintf(stderr, "SQL error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				} else {
					fprintf(stdout, "Characters Table created successfully\n");
				}
			}

			isRunning_ = true;
			
			serverGameLoop();
			
			return 0;
		}

        void Server::quitGame() {
			isRunning_ = false;
			g_serverShouldContinueRunning = false;
			io_context_.stop();
        }

    } // namespace Core
} // namespace RTSEngine


namespace RTSEngine {
    namespace Core {
        AsioSession::AsioSession(ssl_socket socket, Server& server_ref, bool is_client)
            : ssl_socket_(std::move(socket)), 
              server_ref_(server_ref), 
              is_client_session_(false), 
              player_id_(-1) 
        {
        }
        void AsioSession::start() {
			auto self = shared_from_this();
            ssl::stream_base::handshake_type handshake_type = 
                is_client_session_ ? ssl::stream_base::client : ssl::stream_base::server;
            ssl_socket_.async_handshake(handshake_type,
                [this, self](const boost::system::error_code& ec) {
                    if (!ec) {
                        do_read();
                    } else {
                        std::cerr << (is_client_session_ ? "Client" : "Server Session P" + std::to_string(player_id_))
                                  << ": SSL Handshake failed: " << ec.message() << std::endl;
					
						server_ref_.server_remove_session(self);
                    }
                });
        }

        void AsioSession::write(const std::string& full_message_data) {
			asio::post(ssl_socket_.get_executor(), [self = shared_from_this(), full_message_data]() {
				uint32_t msg_len_net_order = boost::asio::detail::socket_ops::host_to_network_long(
					static_cast<uint32_t>(full_message_data.length())
				);

				std::string message_to_send_on_wire;
				message_to_send_on_wire.reserve(sizeof(msg_len_net_order) + full_message_data.length());
				message_to_send_on_wire.append(reinterpret_cast<const char*>(&msg_len_net_order), sizeof(msg_len_net_order));
				message_to_send_on_wire.append(full_message_data);

				bool write_in_progress = !self->write_msgs_.empty();
				self->write_msgs_.push_back(message_to_send_on_wire);
				if (!write_in_progress) {
					self->do_write();
				}
			});
		}

        void AsioSession::handle_read_error(const boost::system::error_code& ec) {
            if (ec == boost::asio::error::eof || ec == ssl::error::stream_truncated) {
                std::cout << (is_client_session_ ? "Client" : "Server Session P" + std::to_string(player_id_))
                          << ": Connection closed by peer." << std::endl;
            } else if (ec != boost::asio::error::operation_aborted) {
                std::cerr << (is_client_session_ ? "Client" : "Server Session P" + std::to_string(player_id_))
                          << ": Read error: " << ec.message() << std::endl;
            }

            server_ref_.server_remove_session(shared_from_this());
        }

        void AsioSession::process_raw_message(const char* data, std::size_t length) {
            std::string full_msg_content(data, length);
            
            server_ref_.enqueue_server_message(shared_from_this(), std::move(full_msg_content));
        }

        void AsioSession::do_read() {
            auto self = shared_from_this();

            if (read_state_ == ReadState::READING_LENGTH) {
                read_buffer_internal_.assign(sizeof(uint32_t), 0);
                read_buffer_offset_ = 0;

                asio::async_read(ssl_socket_,
                    asio::buffer(read_buffer_internal_.data(), sizeof(uint32_t)),
                    [this, self](boost::system::error_code ec, std::size_t length_read) {
                    if (!ec && length_read == sizeof(uint32_t)) {
                        uint32_t msg_len_net_order;
                        std::memcpy(&msg_len_net_order, read_buffer_internal_.data(), sizeof(uint32_t));
                        incoming_msg_payload_length_ = boost::asio::detail::socket_ops::network_to_host_long(msg_len_net_order);

                        const size_t MAX_ALLOWED_MSG_SIZE = 1024 * 1024;
                        if (incoming_msg_payload_length_ > 0 && incoming_msg_payload_length_ < MAX_ALLOWED_MSG_SIZE) {
                            read_state_ = ReadState::READING_PAYLOAD;
                            read_buffer_internal_.assign(incoming_msg_payload_length_, 0);
                            read_buffer_offset_ = 0;
                            do_read();
                        } else {
                            std::cerr << (is_client_session_ ? "Client" : "Server Session P" + std::to_string(player_id_))
                                      << ": Invalid incoming message length: " << incoming_msg_payload_length_ << std::endl;
                            handle_read_error(boost::asio::error::invalid_argument);
                        }
                    } else {
                        handle_read_error(ec);
                    }
                });
            } else {
                asio::async_read(ssl_socket_,
                    asio::buffer(read_buffer_internal_.data(), incoming_msg_payload_length_),
                    [this, self](boost::system::error_code ec, std::size_t length_read) {
                    if (!ec && length_read == incoming_msg_payload_length_) {
                        process_raw_message(read_buffer_internal_.data(), length_read);
                        read_state_ = ReadState::READING_LENGTH;
                        incoming_msg_payload_length_ = 0;
                        read_buffer_internal_.clear();
                        read_buffer_offset_ = 0;
                        do_read();
                    } else {
                        handle_read_error(ec);
                    }
                });
            }
        }

        void AsioSession::do_write() {
            auto self = shared_from_this();
            asio::async_write(ssl_socket_, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    self->write_msgs_.pop_front();
                    if (!self->write_msgs_.empty()) {
                        self->do_write();
                    }
                } else {
                     std::cerr << (is_client_session_ ? "Client" : "Server Session (P" + std::to_string(player_id_) + ")")
                              << ": Write error: " << ec.message() << std::endl;	  
					server_ref_.server_remove_session(self);
                }
            });
        }
		
		void AsioSession::do_close() {
			auto self = shared_from_this();
			ssl_socket_.async_shutdown([this, self](const boost::system::error_code& ec) {
				boost::system::error_code close_ec;
				ssl_socket_.lowest_layer().close(close_ec);
			});
		}

    } // namespace Core
} // namespace RTSEngine
