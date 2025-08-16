// In Game.cpp
#include "Game.h"
#include "../Config.h"
#include "../../shared/NetworkMessageTypes.h" // Our enum
#include "../../shared/NetworkMessages.h"   // Our message structs
#include <sstream>                       // For std::stringstream
#include <boost/archive/binary_oarchive.hpp> // For sending
#include <boost/archive/binary_iarchive.hpp> // For receiving
#include <openssl/md5.h>
#include <SDL3/SDL_image.h> // If used by client
#include <SDL3/SDL_ttf.h>   // If used by client
#include <iostream>
#include <algorithm>
#include <random>
#include <sstream>
// For cJSON for message serialization (example)
#include <cJSON.h> // Assuming it's in include path now

#ifdef _WIN32 // Or #if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

static  int g_windowHeight, g_windowWidth;
static float g_displayScale;

namespace RTSEngine {
    namespace Core {

        AsioSession::AsioSession(ssl_socket socket, Game& game_ref, bool is_client_session)
            : ssl_socket_(std::move(socket)), 
              game_ref_(game_ref), 
              is_client_session_(is_client_session), 
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
						game_ref_.send_login_request();
						
                        do_read();
                    } else {
                        std::cerr << (is_client_session_ ? "Client" : "Server Session P" + std::to_string(player_id_))
                                  << ": SSL Handshake failed: " << ec.message() << std::endl;
                        // Trigger disconnection logic on handshake failure
                        
                        game_ref_.handleDisconnection("SSL handshake failed.");
                    }
                });
        }

        void AsioSession::write(const std::string& full_message_data) { // full_message_data = [TypeID][SerializedBoostData]
			asio::post(ssl_socket_.get_executor(), [self = shared_from_this(), full_message_data]() {
				uint32_t msg_len_net_order = boost::asio::detail::socket_ops::host_to_network_long(
					static_cast<uint32_t>(full_message_data.length())
				);

				std::string message_to_send_on_wire;
				message_to_send_on_wire.reserve(sizeof(msg_len_net_order) + full_message_data.length());
				message_to_send_on_wire.append(reinterpret_cast<const char*>(&msg_len_net_order), sizeof(msg_len_net_order));
				message_to_send_on_wire.append(full_message_data);

				bool write_in_progress = !self->write_msgs_.empty();
				self->write_msgs_.push_back(message_to_send_on_wire); // Queue the [Length][TypeID][SerializedBoostData]
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

            game_ref_.handleDisconnection("Connection to server lost."); // Client handles its disconnection
        }

        void AsioSession::process_raw_message(const char* data, std::size_t length) {
            // This data is [MessageTypeID + SerializedBoostData]
            // std::cout << "AsioSession::process_raw_message: processing " << length << " bytes." << std::endl;
            std::string full_msg_content(data, length);
            game_ref_.enqueue_client_message(std::move(full_msg_content));
        }

        void AsioSession::do_read() {
            auto self = shared_from_this();

            if (read_state_ == ReadState::READING_LENGTH) {
                read_buffer_internal_.assign(sizeof(uint32_t), 0);
                read_buffer_offset_ = 0;

                // SSL: Use the ssl_socket_ for async_read
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
            } else { // read_state_ == ReadState::READING_PAYLOAD
                // SSL: Use the ssl_socket_ for async_read
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
            // SSL: Use the ssl_socket_ for async_write
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
                    
                    game_ref_.client_current_game_state_ = ClientGameState::DISCONNECTED;
                }
            });
        }
		
		void AsioSession::do_close() {
			// Update game state immediately
			if (is_client_session_) {
				game_ref_.client_current_game_state_ = ClientGameState::MAIN_MENU;
			}

			// SSL: Perform a clean shutdown of the SSL layer.
			auto self = shared_from_this();
			ssl_socket_.async_shutdown([this, self](const boost::system::error_code& ec) {
				// After SSL shutdown, close the underlying TCP socket.
				// We ignore errors here as the peer might have already disconnected.
				boost::system::error_code close_ec;
				ssl_socket_.lowest_layer().close(close_ec);
			});
		}

    } // namespace Core
} // namespace RTSEngine

namespace RTSEngine {
    namespace Core {
        Game::Game(bool startAsServer, const std::string& serverIp, unsigned short port)
            : window_(nullptr), renderer_(nullptr), isRunning_(false), currentTicks_(0), targetServerIp_(serverIp), serverPort_(port),
              // io_context_ is default constructed
              client_session_(nullptr),
              nextPlayerId_(1), lobbyFull_(false), gameStarted_(false),
              assetManager_(),
              commandLine_(), 
              uiManager_(nullptr, nullptr, assetManager_),
              inputHandler_(),
			  io_context_(),
			  ssl_context_(boost::asio::ssl::context::tlsv12)
              {
				  
			  }

        Game::~Game() {
            cleanup(); // Ensure io_context stops and thread joins
        }
        void Game::cleanup() {
            isRunning_ = false; // Signal loops to stop

			stopAsioThread();

			
			audioManager_.quit();
			
			
            if (renderer_) SDL_StopTextInput(window_);
            assetManager_.destroyAssets(); 


			ImGui_ImplSDLRenderer3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();

            if (renderer_) SDL_DestroyRenderer(renderer_);
            if (window_) SDL_DestroyWindow(window_);
            
            if ( window_ ) {
                if(TTF_WasInit()) TTF_Quit(); // Check if initialized before quitting
            }
            SDL_Quit();
            
			
            renderer_ = nullptr;
            window_ = nullptr;
        }
		
        // Modified initSDL
        bool Game::initSDL(bool requireGraphics) {
            if (SDL_Init(requireGraphics ? SDL_INIT_VIDEO | SDL_INIT_EVENTS : SDL_INIT_EVENTS) == 0) { // Only init video if needed
                std::cerr << "SDL Init Failed: " << SDL_GetError() << std::endl;
                return false;
            }

            if (requireGraphics) {
                if (TTF_Init() == 0) {
                    std::cerr << "TTF Init Failed: " << SDL_GetError() << std::endl;
                    SDL_Quit();
                    return false;
                }
				
				
				g_displayScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

                window_ = SDL_CreateWindow("Untitled Project",
                                         (int)(WINDOW_DEFAULT_WIDTH), (int)(WINDOW_DEFAULT_HEIGHT),
                                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
				g_windowWidth = WINDOW_DEFAULT_WIDTH;
				g_windowHeight = WINDOW_DEFAULT_HEIGHT;
                if (!window_) {
                    std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
                    TTF_Quit(); SDL_Quit();
                    return false;
                }

                renderer_ = SDL_CreateRenderer(window_, NULL);
                if (!renderer_) {
                    std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
                    SDL_DestroyWindow(window_);
                    TTF_Quit();SDL_Quit();
                    return false;
                }
                
				if (!audioManager_.init()) { // Initialize AudioManager
					std::cerr << "Game Error: AudioManager failed to initialize!" << std::endl;
					// Potentially cleanup other SDL subsystems and return false
					// For now, game can proceed without audio if it fails, but log it.
				}
                assetManager_.init(renderer_);
                new (&uiManager_) UI::UIManager(renderer_, window_, assetManager_); // Re-init with valid renderer
                SDL_StartTextInput(window_);
				ImGui::CreateContext();
				
				ImGuiIO& io_context = ImGui::GetIO(); (void)io_context;
				io_context.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
				
				ImGui::StyleColorsDark();
				ImGuiStyle& style = ImGui::GetStyle();
				style.ScaleAllSizes(g_displayScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
				style.FontScaleDpi = g_displayScale;  
				style.FontSizeBase = 16.0f;
				ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
				ImGui_ImplSDLRenderer3_Init(renderer_);
				font_title = io_context.Fonts->AddFontFromFileTTF("./assets/fonts/default.ttf", 19.0f);
				font_subtitle = io_context.Fonts->AddFontFromFileTTF("./assets/fonts/default.ttf", 17.0f);
				font_description = io_context.Fonts->AddFontFromFileTTF("./assets/fonts/default.ttf", 15.0f);
            }
            return true;
        }

        bool Game::initAsioNetworking() {
            try {
				ssl_context_.set_verify_mode(asio::ssl::verify_peer);
				ssl_context_.use_certificate_chain_file("server.crt");
                client_connect("","");
            } catch (const std::exception& e) {
                std::cerr << "Asio Networking Init Error: " << e.what() << std::endl;
                return false;
            }
            // Start the io_context in a separate thread
            io_thread_ = std::thread([this]() {
                try {
                    io_context_.run(); // This will block until io_context_.stop() or all work is done
                } catch (const std::exception& e) {
                    std::cerr << "io_context thread exception: " << e.what() << std::endl;
                }
            });
            return true;
        }

		void Game::send_login_request() {
			if (client_session_) {
				Network::ClientLoginRequestMsg login_request;
				login_request.username = this->username;
				login_request.password = this->password;
				
				std::stringstream ss;
				Network::NetworkMsgType msg_type_id = Network::NetworkMsgType::C2S_LOGIN_REQUEST;
				
				ss.write(reinterpret_cast<const char*>(&msg_type_id), sizeof(msg_type_id));

				try {
					boost::archive::binary_oarchive oa(ss);
					oa << login_request;
				} catch (const boost::archive::archive_exception& e) {
					std::cerr << "Client Error: Failed to serialize ClientLoginRequest: " << e.what() << std::endl;
					commandLine_.addLogEntry("Error", "Failed to send login request (serialization).");
					return;
				}

				std::string full_message_to_send = ss.str();
				this->client_send_to_server(full_message_to_send);
			}
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

        // --- Asio Client Methods ---
        void Game::client_connect(char username[64], char password[64]) { // Initiates connection

            // Ensure io_context is not in a stopped state before posting work
            if (io_context_.stopped()) {
                std::cout << "Client (client_connect): io_context was stopped, resetting." << std::endl;
                io_context_.restart(); // Or .restart()
            }
            // The I/O thread should be started by initAsioNetworking or reinitAsioNetworkingClient.
            // If it's not running, async_connect's handler will never be called.

            std::cout << "Client (client_connect): Attempting to resolve and connect to " << targetServerIp_ << ":" << serverPort_ << std::endl;
            connection_status_message_ = "Resolving host..."; // Update status for UI
            try {
                tcp::resolver resolver(io_context_);
                auto endpoints = resolver.resolve(targetServerIp_, std::to_string(serverPort_));
                
                auto new_socket = std::make_shared<tcp::socket>(io_context_);
                // Important: The AsioSession constructor takes a moved socket.
                // To avoid moving from new_socket before async_connect, pass a reference or re-think ownership.
                // Let's create session after connect, or pass socket by rvalue ref to async_connect's lambda.

                // Create session immediately, it will own the socket.
                client_session_ = std::make_shared<AsioSession>(asio::ssl::stream<tcp::socket>(io_context_, ssl_context_),*this,true); // new_socket is now moved-from

                connection_status_message_ = "Connecting...";
                auto& socket_layer = client_session_->get_ssl_socket().lowest_layer();
				boost::asio::async_connect(socket_layer, endpoints,
				[this, session = client_session_](const boost::system::error_code& ec, const tcp::endpoint&) {
					if (!ec) {
                        std::cout << "Client: Successfully connected/reconnected to server." << std::endl;
                        this->connection_status_message_ = "Connected! Waiting for server data...";
                        this->isInLobby_ = false; 
                        if (this->client_session_) { // Check if session still exists (it should)
                           this->client_session_->start(); 
                        }
                        // ... Send initial join/rejoin message using 'this->' ...
						
						
                    } else {
                        std::cerr << "Client: Connect/Reconnect attempt failed: " << ec.message() << std::endl;
                        this->connection_status_message_ = "Connection Failed: " + ec.message();
                        if (this->client_session_) {
                            this->client_session_.reset();
                        }
                        this->handleDisconnection("Failed to reocnnect."); 
                    }
                });
            } catch (const std::exception& e) {
                 std::cerr << "Client (client_connect): Exception during connect setup: " << e.what() << std::endl;
                 connection_status_message_ = "Connection Error: " + std::string(e.what());
                 handleDisconnection("Failed to connect."); // Go to disconnected state
            }
        }

        void Game::enqueue_client_message(std::string&& msg) {
            std::lock_guard<std::mutex> lock(client_msg_mutex_);
            client_received_messages_.push_back(std::move(msg));
        }
        
              
		void Game::client_process_incoming_messages() {
			std::deque<std::string> current_batch;
            {
                std::lock_guard<std::mutex> lock(client_msg_mutex_);
                current_batch.swap(client_received_messages_);
            }

            for (const std::string& full_msg_content_from_asio : current_batch) {
                // full_msg_content_from_asio is what AsioSession's framed read provides
                // (i.e., after it has read the length prefix and then the message body)

                if (full_msg_content_from_asio.length() < sizeof(Network::NetworkMsgType)) {
                    std::cerr << "Client: Received message too short to contain a Type ID. Length: "
                              << full_msg_content_from_asio.length() << std::endl;
                    continue;
                }

                // 1. Read Message Type ID from the start of the content
                Network::NetworkMsgType msg_type_id;
                std::memcpy(&msg_type_id, full_msg_content_from_asio.data(), sizeof(Network::NetworkMsgType));

                // 2. Get the actual serialized payload data (after the Type ID)
                std::string serialized_payload_data = full_msg_content_from_asio.substr(sizeof(Network::NetworkMsgType));
                std::stringstream ss_in(serialized_payload_data); // Create input stream from payload
                
                try {
                    boost::archive::binary_iarchive ia(ss_in);

                    switch (msg_type_id) {
                        case Network::NetworkMsgType::S2C_ASSIGN_PLAYER_ID:
                        {
                            Network::AssignPlayerIdMsg assign_msg_data;
                            ia >> assign_msg_data; // Deserialize into the struct

                            myPlayerId_ = assign_msg_data.playerId;
                            // lobbyMaxPlayers_ = assign_msg_data.max_players; // If you add these back
                            // std::string welcomeText = assign_msg_data.welcome_message;

                            std::cout << "Client: Assigned Player ID (Boost.Archive): " << myPlayerId_ << std::endl;
                            //commandLine_.addLogEntry("System", "My Player ID: " + std::to_string(myPlayerId_));
                            isInLobby_ = true; // Now considered in lobby
                            // If max_players and welcome_message were part of this:
                            // commandLine_.addLogEntry("Server", welcomeText);
                        }
                        break;

                        case Network::NetworkMsgType::S2C_LOBBY_WELCOME:
                        {
                            Network::LobbyWelcomeMsg welcome_data;
                            ia >> welcome_data; // Deserialize into the struct

                            // It's possible ASSIGN_PLAYER_ID comes first, or this one sets/confirms it.
                            if (myPlayerId_ == -1 || myPlayerId_ == welcome_data.your_id) { // Ensure consistency or set if not already
                                myPlayerId_ = welcome_data.your_id;
                            } else {
                                std::cerr << "Client Warning: LOBBY_WELCOME 'yourId' (" << welcome_data.your_id 
                                          << ") mismatches previously assigned ID (" << myPlayerId_ << ")." << std::endl;
                                // Potentially disconnect or flag error. For now, trust LOBBY_WELCOME if it comes.
                                myPlayerId_ = welcome_data.your_id;
                            }
                            
                            lobbyMaxPlayers_ = welcome_data.max_players;
                            std::string welcomeText = welcome_data.welcome_message;

                            std::cout << "Client: LOBBY_WELCOME - My ID: " << myPlayerId_
                                      << ", Max Players: " << lobbyMaxPlayers_
                                      << ", Msg: [" << welcomeText << "]" << std::endl;
                            commandLine_.addLogEntry("Server", welcomeText);
                            isInLobby_ = true;
                            client_current_game_state_ = ClientGameState::IN_LOBBY; // Explicitly transition if connecting
                            connection_status_message_ = "Connected to lobby!"; // Update status
                        }
                        break;
						
						case Network::NetworkMsgType::S2C_CHAT_BROADCAST:
                        {
                            Network::ServerChatBroadcastMsg chat_data;
                            ia >> chat_data; // Deserialize

                            std::string display_name = chat_data.sender_name;
                            if (chat_data.sender_player_id == myPlayerId_) {
                                // display_name = "Me"; // Or keep name, UI can color it differently
                            }
                            
                            std::string formatted_chat = "[" + display_name + "]: " + chat_data.text;
                            std::cout << "Client Chat RCV: " << formatted_chat << std::endl;
                            commandLine_.addLogEntry(display_name, chat_data.text); // Add to command log
                            // If you have a dedicated chat UI panel, you'd add it there.
                        }
                        break;
						
						case Network::NetworkMsgType::S2C_COMMAND_ACK:
                        {
                            Network::CommandAckMsg ack_data;
                            ia >> ack_data; // Deserialize

                            std::string log_prefix = "[*]";
                            if (!ack_data.success) {
                                log_prefix = "[ ]"; // Not Acknowledged / Negative Acknowledgment
                            }
                            
                            std::string full_log_message = "[" + ack_data.original_command_name + "]: " + ack_data.status_message;
                            
                            std::cout << "Client: " << log_prefix << " for '" << ack_data.original_command_name
                                      << "' - Success: " << ack_data.success
                                      << ", Status: " << ack_data.status_message << std::endl;
                            
                            // Add to command log
                            // The command the user typed was already logged when sent.
                            // This ACK is the server's response.
                            commandLine_.addLogEntry(log_prefix, full_log_message);

                            // Optional: Trigger a specific confirmation/error sound
                            if (ack_data.success) {
                               audioManager_.playSoundEffect("command_ack_positive");
                            } else {
                               audioManager_.playSoundEffect("command_ack_negative");
                            }
                        }
                        break;
                        
                        case Network::NetworkMsgType::S2C_CLIENT_KICKED:
                        {
                            Network::ClientKickedMsg client_kicked_msg;
                            ia >> client_kicked_msg; // Deserialize into the struct

                            // lobbyMaxPlayers_ = assign_msg_data.max_players; // If you add these back
                            // std::string welcomeText = assign_msg_data.welcome_message;

                            std::cout << "Client: Kicked from server. Reason: " << client_kicked_msg.message << std::endl;
                            handleDisconnection(client_kicked_msg.message);
                        }
                        break;
						
						case Network::NetworkMsgType::S2C_PLAYER_INFO:
                        {
                            Network::PlayerNetInfo player_info_msg;
                            ia >> player_info_msg; // Deserialize into the struct

							if (player_info_msg.you) {
								myPlayer.name = player_info_msg.name;
								myPlayer.system = player_info_msg.system;
								myPlayer.proximity = player_info_msg.proximity;
								myPlayer.combat_xp = player_info_msg.combat_xp;
								myPlayer.explore_xp = player_info_msg.explore_xp;
							}
                        }
						break;
						
                        default:
                            std::cerr << "Client: Received unhandled Boost.Archive message type ID: "
                                      << static_cast<int>(msg_type_id) << std::endl;
                            break;
                    }
                } catch (const boost::archive::archive_exception& e) {
                    std::cerr << "Client: Boost.Archive deserialization error for type ID "
                              << static_cast<int>(msg_type_id) << ". Content snippet: '"
                              << serialized_payload_data.substr(0, std::min(50, (int)serialized_payload_data.length())) << "' Error: " << e.what() << std::endl;
                    // This message might be corrupted, or client/server are out of sync on struct definitions,
                    // or the message type ID was wrong and we tried to deserialize into the wrong struct type.
                } catch (const std::exception& e) { // Catch other potential errors from stringstream etc.
                     std::cerr << "Client: General error processing message type ID "
                              << static_cast<int>(msg_type_id) << ": " << e.what() << std::endl;
                }// Delete json at the end of processing for one message
			} // End of for loop over current_batch
		} // End of function Game::client_process_incoming_messages()

        void Game::client_send_to_server(const std::string& message) {
            if (client_session_) {
                client_session_->write(message);
            } else {
                std::cerr << "Client: Not connected, cannot send message." << std::endl;
            }
        }

        bool Game::loadAssets() {
			
            if (!assetManager_.loadTexture("wallpaper", "./assets/wallpapers/main.png")) {};
			if (!audioManager_.loadSoundEffect("keypress", "./assets/sounds/keypress.wav")) {
				std::cerr << "Game Warning: Failed to load keypress sound." << std::endl;
			}
			if (!audioManager_.loadSoundEffect("command_enter", "./assets/sounds/command_enter.wav")) {
				std::cerr << "Game Warning: Failed to load keypress sound." << std::endl;
			}
			if (!audioManager_.loadSoundEffect("command_ack_positive", "./assets/sounds/command_confirm.wav")) {
				std::cerr << "Game Warning: Failed to load command_ack_positive sound." << std::endl;
			}
			if (!audioManager_.loadSoundEffect("command_ack_negative", "./assets/sounds/command_fail.wav")) {
				std::cerr << "Game Warning: Failed to load command_ack_negative sound." << std::endl;
			}
			if (!audioManager_.loadSoundEffect("alert", "./assets/sounds/alert.wav")) {
				std::cerr << "Game Warning: Failed to load alert sound." << std::endl;
			}
            return true;
        }

		void Game::startAsioThread() {
            if (io_thread_should_run_.load()) {
                std::cout << "Client: Asio thread already marked to run." << std::endl;
                if (io_thread_.joinable()) {
                     std::cerr << "Client Warning: startAsioThread called, io_thread_should_run_ is true, but io_thread_ is joinable. Attempting to join previous." << std::endl;
                     io_thread_.join();
                } else if(io_context_.stopped()){
                    io_context_.restart();
                }
            }

            std::cout << "Client: Starting Asio I/O thread..." << std::endl;
            io_thread_should_run_.store(true);

            if (io_context_.stopped()) {
                io_context_.restart();
            }

            work_guard_ = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
                io_context_.get_executor()
            );

            io_thread_ = std::thread([this]() {
				std::cout << "Client: Asio I/O thread started. TID: " << std::this_thread::get_id() << std::endl;

				try {
					io_context_.run(); 
				} catch (const std::exception& e) {
					std::cerr << "Client: Asio I/O thread exception: " << e.what() << std::endl;
				}
				
				std::cout << "Client: io_context_.run() has returned. Asio I/O thread finishing. TID: " << std::this_thread::get_id() << std::endl;
				io_thread_should_run_.store(false);
			});
        }

        void Game::stopAsioThread() {
			std::cout << "Client: Attempting to stop Asio I/O thread..." << std::endl;
			
			if (work_guard_) {
				std::cout << "Client: Resetting Asio work_guard." << std::endl;
				work_guard_->reset();
				work_guard_.reset();
			}

			if (!io_context_.stopped()) {
				std::cout << "Client: Posting stop to io_context." << std::endl;
				post(io_context_, [this]() {
					if (!this->io_context_.stopped()) {
						this->io_context_.stop();
					}
				});
			} else {
				std::cout << "Client: io_context already stopped." << std::endl;
			}

			io_thread_should_run_.store(false); 

			if (io_thread_.joinable()) {
				std::cout << "Client: Joining Asio I/O thread..." << std::endl;
				io_thread_.join();
				std::cout << "Client: Asio I/O thread joined." << std::endl;
			} else {
				std::cout << "Client: Asio I/O thread was not joinable (already joined, never started, or exited due to error)." << std::endl;
			}

			if (io_context_.stopped()) {
				std::cout << "Client: Resetting (restarting) stopped io_context." << std::endl;
				io_context_.restart();
			}
		}
		bool Game::reinitAsioNetworkingClient() {
			std::cout << "Client: Re-initializing Asio networking for reconnection..." << std::endl;
			transitionToState(ClientGameState::CONNECTING);
			connection_status_message_ = "Preparing to reconnect...";

			if (client_session_) { client_session_.reset(); }
			
			stopAsioThread();

			startAsioThread();
			client_connect("","");

			connection_status_message_ = "Reconnecting to " + targetServerIp_ + "...";
			std::cout << "Client: Re-initialization sequence complete. Connection attempt posted." << std::endl;
			return true;
		}
		void Game::handleDisconnection(const std::string message) {
            std::cout << "Client: handleDisconnection called." << std::endl;
            if(client_current_game_state_ == ClientGameState::DISCONNECTED && !connection_status_message_.empty() && connection_status_message_ != "Disconnected."){

            } else {
				
                connection_status_message_ = "Disconnected.";
            }
            
            if (client_session_) {
                if (client_session_->is_open()) {
                    boost::system::error_code ec_ignored;
                    client_session_->do_close();
                }
				std::cout << "Client: resetting asio session." << std::endl;
                client_session_.reset();
            }


			disconnect_message = message;
            transitionToState(ClientGameState::DISCONNECTED);
			std::cout << "Client: Disconnected" << std::endl;
            isInLobby_ = false;
            gameStarted_ = false;
        }

		void Game::clientInitiateConnection(char username[64], char password[64]) {
            this->username = std::string(username);
			this->password = std::string(password);
            std::cout << "Client: User initiated connection." << std::endl;
            transitionToState(ClientGameState::CONNECTING);
            connection_status_message_ = "Initializing network...";

            if (!io_thread_should_run_.load() || (io_thread_.joinable() && !io_context_.stopped() /* implies thread died */ ) ) {
                if (io_thread_.joinable()) {
                    stopAsioThread();
                }
                startAsioThread();
            } else if(io_context_.stopped()){
                io_context_.restart();
            }

            client_connect(username, password);
        }
		
        void Game::clientGameLoop() {
            const int TARGET_FPS = 60;
            const int FRAME_DELAY = 1000 / TARGET_FPS;
            Uint32 frameStart;
            int frameTime;
            std::cout << "Client game loop started." << std::endl;

            while (isRunning_) {
                frameStart = SDL_GetTicks();
                currentTicks_ = frameStart;

                bool stillRunning = inputHandler_.processInput(*this, window_, commandLine_);
                if (!stillRunning) { isRunning_ = false; break; }

                client_process_incoming_messages();

                render();

                frameTime = SDL_GetTicks() - frameStart;
                if (FRAME_DELAY > frameTime) {
                    SDL_Delay(FRAME_DELAY - frameTime);
                }
            }
        }
        int Game::run() {
			if (!initSDL(true)) return 1;
			if (!loadAssets()) { cleanup(); return 1; }
					
			isRunning_ = true;
			
			clientMasterLoop();

			return 0;
		}

		// New: clientMasterLoop
		void Game::clientMasterLoop() {
			const int TARGET_FPS = 60;
			const int FRAME_DELAY = 1000 / TARGET_FPS;
			Uint32 frameStart;
			int frameTime;

			std::cout << "Client master loop started. Initial state: MAIN_MENU" << std::endl;
			client_current_game_state_ = ClientGameState::MAIN_MENU;
			while (isRunning_) {
				frameStart = SDL_GetTicks();
				currentTicks_ = frameStart;

				bool stillRunning = inputHandler_.processInput(*this, window_, commandLine_);
				if (!stillRunning) { isRunning_ = false; break; }

				switch (client_current_game_state_) {
					case ClientGameState::MAIN_MENU:
						updateMainMenu();
						break;
					case ClientGameState::CONNECTING:
						updateConnecting();
						break;
					case ClientGameState::IN_LOBBY:
						updateInLobby();
						break;
					case ClientGameState::LOADING_MAP:
						updateLoadingMap();
						break;
					case ClientGameState::IN_GAME:
						updateInGame();
						break;
					case ClientGameState::DISCONNECTED:
						updateDisconnected();
						break;
					default:
						break;
				}
				
				renderClient();

				frameTime = SDL_GetTicks() - frameStart;
				if (FRAME_DELAY > frameTime) {
					SDL_Delay(FRAME_DELAY - frameTime);
				}
			}
		}
		
		void Game::transitionToState(ClientGameState newState){ client_current_game_state_ = newState; }

		void Game::updateMainMenu() { /* Logic handled by input for now */ }
		void Game::updateConnecting() {
			if (client_session_ && client_session_->is_open()) {
			client_process_incoming_messages();
				client_current_game_state_ = ClientGameState::IN_LOBBY;
				std::cout << "Client: Transitioning to IN_LOBBY state." << std::endl;
			}
		}
		void Game::updateInLobby() {
			client_process_incoming_messages();
		}
		void Game::updateLoadingMap() {
			client_process_incoming_messages();
		}
		void Game::updateInGame() {
			client_process_incoming_messages();
		}
		void Game::updateDisconnected() {
		}

		static int seed = 0;
		void Game::renderClient() {
			
			SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
			SDL_RenderClear(renderer_);
			
			SDL_Texture* bg_tex = assetManager_.getTexture("wallpaper");
			SDL_FRect texRect = {
				0.0f, 0.0f,
				(float)g_windowWidth, (float)g_windowHeight
			};
			SDL_RenderTexture(renderer_, bg_tex, nullptr, &texRect);

			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			switch (client_current_game_state_) {
				case ClientGameState::MAIN_MENU:
					uiManager_.renderMainMenu(main_menu_options_, main_menu_selected_option_, this);
					break;
				case ClientGameState::CONNECTING:
					uiManager_.renderConnectingScreen(connection_status_message_);
					break;
				case ClientGameState::IN_LOBBY:
					
					uiManager_.renderLobbyScreen(client_lobby_players_, myPlayerId_, lobbyMaxPlayers_, commandLine_, this);
					break;
				case ClientGameState::LOADING_MAP:
					uiManager_.renderLoadingScreen("...");
					break;
				case ClientGameState::IN_GAME:
					break;
				case ClientGameState::DISCONNECTED:
					uiManager_.renderDisconnectedScreen(disconnect_message, this);
					break;
				default:
					break;
			}
			
			ImGui::Render();
			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
			SDL_RenderPresent(renderer_);
		}

        void Game::quitGame() {
			isRunning_ = false;
			io_context_.stop(); 
        }

        void Game::render() {
            if (!renderer_ || !window_) return;
			
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            uiManager_.renderAll(commandLine_, this);
            SDL_RenderPresent(renderer_);
        }
        
        void Game::handleResize() {
            if (window_ && renderer_) {
                SDL_GetWindowSizeInPixels(window_, &g_windowWidth, &g_windowHeight);
				g_windowWidth = g_windowWidth;
				g_windowHeight = g_windowHeight;
				std::cout << "Handle Resize finished." << std::endl;
            }
        }
						
		void Game::client_package_and_send_command(const std::string& command_name_lower,
                                                   const std::vector<std::string>& original_args) {
            if (!client_session_ || !client_session_->is_open()) {
                commandLine_.addLogEntry("Error", "Not connected to server. Cannot send command.");
                return;
            }
            if (command_name_lower.empty()) {
                commandLine_.addLogEntry("Error", "Cannot send empty command.");
                return;
            }

            Network::ClientCommandMsg cmd_to_send;
            cmd_to_send.command_name = command_name_lower;
            cmd_to_send.args = original_args;
			
            if (command_name_lower == "move" ||command_name_lower == "mov" ||
			command_name_lower == "build"  ||command_name_lower == "bld" || 
			command_name_lower == "attack"  ||command_name_lower == "atk" || 
			command_name_lower == "halt" || command_name_lower == "hlt" /* add other unit-group commands */) {
                if (cmd_to_send.selected_entity_ids.empty()) {
                    commandLine_.addLogEntry("Client", "No units selected for command: " + command_name_lower);
                    return; 
                }
            }

            std::stringstream ss;
            Network::NetworkMsgType msg_type_id = Network::NetworkMsgType::C2S_CLIENT_COMMAND;
            ss.write(reinterpret_cast<const char*>(&msg_type_id), sizeof(msg_type_id));

            try {
                boost::archive::binary_oarchive oa(ss);
                oa << cmd_to_send;
            } catch (const boost::archive::archive_exception& e) {
                std::cerr << "Client Error: Failed to serialize ClientCommandMsg: " << e.what() << std::endl;
                commandLine_.addLogEntry("Error", "Failed to send command (serialization).");
                return;
            }

            std::string full_message_to_send = ss.str();
            client_send_to_server(full_message_to_send);

            std::string args_str_for_log;
            for(const auto& arg : original_args) args_str_for_log += arg + " ";
            if(!args_str_for_log.empty()) args_str_for_log.pop_back();
            commandLine_.addLogEntry(">" + command_name_lower + (args_str_for_log.empty() ? "" : " " + args_str_for_log), "Command sent.");
        }
    } // namespace Core
} // namespace RTSEngine