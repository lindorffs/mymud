#ifndef RTSENGINE_RENDERUTILS_H
#define RTSENGINE_RENDERUTILS_H

#include <SDL3/SDL_pixels.h>
#include "../Config.h" // For color constants

namespace RTSEngine {
    namespace Utils {

        class RenderUtils {
        public:
            static SDL_Color getOwnerColor(int player, int owner, bool selected) {
                if (selected && owner == player) { return COLOR_WHITE; }
                else if (selected && owner != player) { return COLOR_RED_PLAYER; }
                switch (owner) {
                    case 0: return COLOR_WHITE;        // Neutral
                    case 1: return COLOR_YELLOW_OTHER;   // Player 1
                    case 2: return COLOR_GREEN_PLAYER; // Player 2
                    case 3: return COLOR_BLUE_PLAYER;  // Player 3
                    default: return COLOR_YELLOW_OTHER; // Others
                }
            }
        };

    } // namespace Utils
} // namespace RTSEngine

#endif // RTSENGINE_RENDERUTILS_H