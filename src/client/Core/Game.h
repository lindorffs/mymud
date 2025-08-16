#ifndef RTSENGINE_GAME_H
#define RTSENGINE_GAME_H

#include <SDL3/SDL.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <deque>
#include <thread>
#include <mutex>
#include <memory>
#include <set>
#include <random>

// Boost.Asio includes
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "InputHandler.h"
#include "../../shared/LobbyTypes.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "../UI/CommandLine.h"
#include "../UI/UIManager.h"


namespace asio = boost::asio;
using asio::ip::tcp;

namespace RTSEngine {
	namespace UI { class UIManager; class CommandLine; class SelectionManager; }
    namespace Core {


        const int MAX_PLAYERS = 2;
        const int SERVER_TICK_RATE_MS = 75;
        const size_t ASIO_READ_BUFFER_SIZE = 1024;

        // Forward declaration
        class AsioSession;

		// --- Structs with Normalized Coordinates (Unchanged) ---
		struct Star { float normX, normY, normSize; SDL_Color color; };
		struct ProceduralObject {
			SDL_Texture* texture = nullptr;
			float normX, normY, normSize;
			double angle = 0.0;
			~ProceduralObject() { if (texture) SDL_DestroyTexture(texture); }
			ProceduralObject(const ProceduralObject&) = delete;
			ProceduralObject& operator=(const ProceduralObject&) = delete;
			ProceduralObject(ProceduralObject&& other) noexcept : texture(other.texture), normX(other.normX), normY(other.normY), normSize(other.normSize), angle(other.angle) { other.texture = nullptr; }
			ProceduralObject& operator=(ProceduralObject&& other) noexcept { if (this != &other) { if (texture) SDL_DestroyTexture(texture); texture = other.texture; normX = other.normX; normY = other.normY; normSize = other.normSize; angle = other.angle; other.texture = nullptr; } return *this; }
			ProceduralObject() = default;
		};

		// --- MODIFIED: A struct to hold the parsed scene recipe ---
		struct SceneDescription {
			int numStars = 4000;
			int numGalaxies = 2;
			int numDustClouds = 5;
			SDL_Color backgroundColor = {0, 0, 10, 255};
		};
		class Game;
		class SpaceScene {
		public:
			SpaceScene(SDL_Renderer* renderer, unsigned int seed, Game* game);
			SpaceScene(SDL_Renderer* renderer) : m_renderer(renderer) {}
			SpaceScene() {};

			// The main generation function
			void generate(const std::string& sceneCode);

			void render();
			void set_renderer(SDL_Renderer* renderer) { m_renderer = renderer; }
			void set_seed(unsigned int seed) {m_seed = seed; }

		private:
			void generateStars(int count);

			void generateDustClouds(int count);

			void generateGalaxies(int count);

			SDL_Renderer* m_renderer;
			Game *game_;
			unsigned int m_seed;
			std::string m_sceneCode;
			SceneDescription m_description;
			std::mt19937 m_prng; // Mersenne Twister pseudo-random generator

			std::vector<Star> m_stars;
			std::vector<ProceduralObject> m_galaxies;
			std::vector<ProceduralObject> m_dustClouds;
		};
		
		struct PlayerInfo {
			std::string name;
			std::string system;
			std::string proximity;
			unsigned int combat_xp;
			unsigned int explore_xp;
		};
		
        // Game class remains the central piece
        class Game {
        private:
			ClientGameState client_current_game_state_;
			std::vector<std::string> main_menu_options_ = {"Connect to Server", "Options (N/A)", "Quit"};
            unsigned int main_menu_selected_option_ = 0;

            std::string connection_status_message_ = "Connecting...";
			std::map<int, LobbyPlayerInfo> client_lobby_players_; // Keyed by player ID
			int myPlayerId_ = -1; // Assigned by server
			
			bool isInLobby_ = false; // True when connected and before game starts
			int lobbyMaxPlayers_ = 0; // Received from server

            SDL_Window* window_;
            SDL_Renderer* renderer_;
			
            bool isRunning_;
            unsigned int currentTicks_;
			
			std::string currentScene_ = "s4000g2d5R0G5B15";


            std::string targetServerIp_;
            unsigned short serverPort_;

            // --- Boost.Asio Networking Members ---
            asio::io_context io_context_;
			std::thread io_thread_;
			std::atomic<bool> io_thread_should_run_ = {false}; // Renamed for clarity
			std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_; // Hold work_guard as member

			void startAsioThread(); // New helper
			void stopAsioThread();


            // Client-specific Asio members
            std::shared_ptr<AsioSession> client_session_; // Connection to the server

            std::deque<std::string> client_received_messages_;
            std::mutex client_msg_mutex_;


            int nextPlayerId_;
            bool lobbyFull_;
            bool gameStarted_;

            AssetManager assetManager_;
            
            UI::CommandLine commandLine_;
            UI::UIManager uiManager_; 
            InputHandler inputHandler_;


            bool initSDL(bool requireGraphics);
            bool initAsioNetworking(); // Replaces initNetworking
            bool initLua(); // New method to initialize Lua and bindings
            bool loadAssets();
			
			std::atomic<bool> io_thread_running_ = {false};
			
            void clientGameLoop(); // New loop for client logic + processing Asio messages
			void clientMasterLoop();
            // void gameLoop(); // Old gameLoop will be one of the above

            // Asio Client methods
            void client_connect(char username[64], char password[64]);
            void client_process_incoming_messages(); // Process messages from client_received_messages_
            void client_send_to_server(const std::string& message);
            void enqueue_client_message(std::string&& msg);


            void update(); // Becomes client-side update or server-side game tick
            void render(); // Client-side
            void cleanup();
			
			AudioManager audioManager_;
			std::string disconnect_message;
			
			std::string username_;
			std::string password_;
			asio::ssl::context ssl_context_;
        public:
			PlayerInfo myPlayer;
			ImFont *font_title;
			ImFont *font_subtitle;
			ImFont *font_description;
			std::string username;
			std::string password;
			Core::SpaceScene scene_;
			std::vector<SystemInformation> connection_map_;
			SDL_Window* get_window() {return window_;}
			void handleDisconnection(const std::string);
			
			void send_login_request();
			void clientInitiateConnection(char username[64], char password[64]);
			
			AudioManager& getAudioManager() { return audioManager_; }
			bool reinitAsioNetworkingClient();
			void mainMenuNavigate(int direction); // Changes main_menu_selected_option_
			void mainMenuSelect(); // Acts on selected option
			UI::CommandLine& getMutableCommandLine() { return commandLine_; } // If needed
			void transitionToState(ClientGameState newState);
			ClientGameState getClientGameState() const { return client_current_game_state_; }
			void updateMainMenu();
			void updateConnecting() ;
			void updateLoadingMap();
			void updateInGame() ;
			void updateDisconnected();
			void renderClient();
			void updateInLobby(); 
			const std::map<int, LobbyPlayerInfo>& getLobbyPlayers() const { return client_lobby_players_; }
			int getMyPlayerId() const { return myPlayerId_; }
			bool isInLobby() const { return isInLobby_; }
			int getLobbyMaxPlayers() const { return lobbyMaxPlayers_; }
			void client_package_and_send_command(const std::string& command_name, const std::vector<std::string>& args);
            Game(bool startAsServer, const std::string& serverIp, unsigned short port);
            ~Game();

            int run();
            void quitGame();
            void handleResize();
			
            friend class AsioSession;
        };

		namespace ssl = boost::asio::ssl;
		using ssl_socket = ssl::stream<tcp::socket>;

        // --- AsioSession Class (represents a single client connection on server, or client's connection to server) ---
        class AsioSession : public std::enable_shared_from_this<AsioSession> {
        public:
            AsioSession(ssl_socket socket, Game& game_ref, bool is_client_session);
            void start(); // Start reading
            void write(const std::string& msg);
			
            int get_player_id() const { return player_id_; }
            void set_player_id(int id) { player_id_ = id; }
			ssl_socket& get_ssl_socket() { return ssl_socket_; }
			void do_close();
			bool is_open() const {return ssl_socket_.lowest_layer().is_open();};
        private:
			void do_read();
			void process_raw_message(const char* data, std::size_t length); // Helper
			void handle_read_error(const boost::system::error_code& ec);
			  enum class ReadState {
				READING_LENGTH,
				READING_PAYLOAD
			};
			ReadState read_state_ = ReadState::READING_LENGTH;
			uint32_t incoming_msg_payload_length_ = 0;
			std::vector<char> read_buffer_internal_;    // Option 2: std::vector<char>
                                            // Let's use std::vector<char> for more manual control here
			std::size_t read_buffer_offset_ = 0;     // How much of a partial message we've already read into read_buffer_internal_

			// The old `char read_buffer_[ASIO_READ_BUFFER_SIZE];` can be removed or repurposed if
			// you always read into a temporary small buffer first, then append to read_buffer_internal_.
			// For simplicity, async_read_some will read directly into parts of read_buffer_internal_ or a temp.
			// Let's use a temporary buffer for each read_some, then append.
			static const size_t TEMP_READ_BUFFER_SIZE = 1024; // Max chunk to read at once
			char temp_read_buf_[TEMP_READ_BUFFER_SIZE];
            void do_write(); // Write queued messages

			ssl_socket ssl_socket_;
            Game& game_ref_; // Reference to the main Game class
            bool is_client_session_; // True if this session object is on the client side
            char read_buffer_[ASIO_READ_BUFFER_SIZE];
            std::deque<std::string> write_msgs_; // Queue of messages to send
            int player_id_; // Server assigns this, client might not use it directly this way
        };


    } // namespace Core
} // namespace RTSEngine

#endif // RTSENGINE_GAME_H