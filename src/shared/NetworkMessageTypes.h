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
            S2C_LOBBY_MESSAGE_TEXT = 8,
            S2C_COMMAND_ACK = 12,
            S2C_ERROR_MESSAGE_TEXT = 13,
			S2C_CHAT_BROADCAST = 14,
			S2C_CLIENT_KICKED = 15,
			S2C_LOGIN_RESPONSE = 16,
			S2C_PLAYER_DATA = 17,
            // Client to Server
            C2S_LOGIN_REQUEST = 100, // Example initial join message
            C2S_CLIENT_COMMAND = 101,
			C2S_CHAT_MESSAGE = 102,
            // Add other C2S message types as needed
        };

    } // namespace Network
} // namespace RTSEngine

#endif // RTSENGINE_NETWORKMESSAGETYPES_H