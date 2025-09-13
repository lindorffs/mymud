#ifndef RTSENGINE_GAME_H
#define RTSENGINE_GAME_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <deque> // For message queues
#include <thread> // For running io_context
#include <mutex>  // For protecting shared data (like message queues)
#include <memory> // For std::shared_ptr, std::enable_shared_from_this
#include <set>
#include <random>

#include <sqlite3.h>

// --- Sol3 Include ---
#define SOL_ALL_SAFETIES_ON 1 // Recommended for development
#define SOL_PRINT_ERRORS 1    // Print errors to cerr by default
#include "sol/sol.hpp"        // Adjust path if you placed it differently
#include "../shared/LobbyTypes.h"


#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
namespace asio = boost::asio; // Alias for convenience
using asio::ip::tcp;         // Alias for TCP types

namespace RTSEngine {
    namespace Core {
		class AsioSession;
		struct Site {
			std::string name;
			std::string description;
		};

		struct System {
			std::string name;
			std::string description;
			std::vector<Site> connected_sites;
		};
        class Server {
        const int SERVER_TICK_RATE_MS = 75;
        private:
			void writeCharacterData_Lua(int id, const std::string& system, const std::string& proximity, int grid_x, int grid_y, unsigned int combat_xp, unsigned int explore_xp, unsigned int trade_xp, unsigned int mining_xp);
			std::map<int, LobbyPlayerInfo> client_lobby_players_; // Keyed by player ID
			bool isInLobby_ = false; // True when connected and before game starts

			// --- Lua/Sol3 Members ---
            sol::state lua_state_;       // The Lua state managed by Sol3
			
            bool lua_initialized_ = false;
            bool isRunning_;
			
			#ifdef WIN32
            uint64_t currentTicks_ = 0;
            uint64_t lastLogicTick_ = 0;
			#else			
            u_int64_t currentTicks_ = 0;
            u_int64_t lastLogicTick_ = 0;
			#endif

            unsigned short serverPort_;

            // --- Boost.Asio Networking Members ---
            asio::io_context io_context_;
			std::thread io_thread_;
			std::atomic<bool> io_thread_should_run_ = {false}; // Renamed for clarity
			std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_; // Hold work_guard as member

			void startAsioThread(); // New helper
			void stopAsioThread();

            // Server-specific Asio members
            std::unique_ptr<tcp::acceptor> acceptor_; // Listens for new connections
            std::map<std::shared_ptr<AsioSession>, bool> active_sessions_; // Manages active client sessions

            // Message queues for communication between game thread and Asio thread
            std::deque<std::pair<std::shared_ptr<AsioSession>, std::string>> server_received_messages_;
            std::mutex server_msg_mutex_;

            int nextPlayerId_;

            

            bool initAsioNetworking();
            bool initLua();
			std::atomic<bool> io_thread_running_ = {false};
            void serverGameLoop();

            // Asio Server methods
            void start_accept();
            void handle_accept(std::shared_ptr<AsioSession> new_session, const boost::system::error_code& error);
            void server_process_incoming_messages(); // Process messages from server_received_messages_
            void server_broadcast(const std::string& message, std::shared_ptr<AsioSession> exclude_session = nullptr);
            void server_send_to_session(std::shared_ptr<AsioSession> session, const std::string& message);
            void server_remove_session(std::shared_ptr<AsioSession> session);


            void enqueue_server_message(std::shared_ptr<AsioSession> session, std::string&& msg);


            void update();
            void cleanup();
			
			
			asio::ssl::context ssl_context_;
			sqlite3 *database_;
        public:
            friend class AsioSession;
			
			void sendCommandAckToPlayer_Lua(int playerId, const std::string& original_command, bool success, const std::string& status_msg);

			const std::map<int, LobbyPlayerInfo>& getLobbyPlayers() const { return client_lobby_players_; }
		    std::atomic<bool> g_serverShouldContinueRunning = true;
			friend void consoleInputLoop(Server* game_ptr);
            Server(const std::string& serverIp, unsigned short port);
            ~Server();

            int run();
            void quitGame();
		
            void broadcastChatMessage_Lua(const std::string& message);
			
            void processClientCommand(int playerId, const std::string& command, const std::vector<std::string>& args);
			
			void setPlayerName_Lua(int playerId, const std::string& name);
			std::string getPlayerName_Lua(int playerId);
			bool kickPlayer_Lua(int playerId, const std::string& msg);
			void broadcastMessageToLobby_Lua(const std::string& type, const sol::table& data);
			void sendMessageToPlayer_Lua(int playerId, const std::string& type, const sol::table& data);
        };
    } // namespace Core
} // namespace RTSEngine


namespace asio = boost::asio; // Alias for convenience
using asio::ip::tcp;         // Alias for TCP types

const size_t ASIO_READ_BUFFER_SIZE = 1024;
namespace RTSEngine {	
	namespace ssl = boost::asio::ssl;
	using ssl_socket = ssl::stream<tcp::socket>;
	
	namespace Core {
		class AsioSession : public std::enable_shared_from_this<AsioSession> {
public:
            AsioSession(ssl_socket socket, Server& server_ref, bool is_client_session);
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
            Server& server_ref_;
            bool is_client_session_; // True if this session object is on the client side
            char read_buffer_[ASIO_READ_BUFFER_SIZE];
            std::deque<std::string> write_msgs_; // Queue of messages to send
            int player_id_; // Server assigns this, client might not use it directly this way
			
        };
    } // namespace Core
} // namespace RTSEngine

#endif // RTSENGINE_GAME_H
