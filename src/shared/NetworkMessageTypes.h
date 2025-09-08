#ifndef RTSENGINE_NETWORKMESSAGETYPES_H
#define RTSENGINE_NETWORKMESSAGETYPES_H

#include <cstdint> // For uint8_t

namespace RTSEngine {
    namespace Network {

        enum class NetworkMsgType : uint8_t {
            MSG_UNKNOWN = 0,
            // Server to Client
            S2C_ASSIGN_PLAYER_ID = 1,
            S2C_LOBBY_WELCOME = 2,
            S2C_LOBBY_MESSAGE_TEXT = 3,
            S2C_COMMAND_ACK = 4,
            S2C_ERROR_MESSAGE_TEXT = 5,
			S2C_CHAT_BROADCAST = 6,
			S2C_CLIENT_KICKED = 7,
			S2C_LOGIN_RESPONSE = 8,
			S2C_PLAYER_INFO = 9,
			S2C_MAP_DATA = 10,
			S2C_SYSTEM_DATA = 11,
			S2C_SITE_DATA = 12,
            // Client to Server
            C2S_LOGIN_REQUEST = 100, // Example initial join message
            C2S_CLIENT_COMMAND = 101,
			C2S_CHAT_MESSAGE = 102,
            // Add other C2S message types as needed
        };

    } // namespace Network
} // namespace RTSEngine

#endif // RTSENGINE_NETWORKMESSAGETYPES_H