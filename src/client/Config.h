#ifndef RTSENGINE_CONFIG_H
#define RTSENGINE_CONFIG_H

#include <SDL3/SDL_pixels.h>

namespace RTSEngine {

    const int TILE_SIZE = 32;
    const unsigned int STANDARD_MOVE_TIMER = 1000; // milliseconds
    const size_t MAX_COMMAND_HISTORY = 10; // Increased for better usability

    const int WINDOW_DEFAULT_WIDTH = 1280;
    const int WINDOW_DEFAULT_HEIGHT = 720;

    // UI Panel Dimensions (approximate based on original offsets)
    const int SELECTION_PANEL_WIDTH = 245;
    const int COMMAND_PANEL_HEIGHT = 185;

    // Colors
    const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
    const SDL_Color COLOR_RED_PLAYER = {180, 64, 63, 255};
    const SDL_Color COLOR_GREEN_PLAYER = {64, 180, 63, 255};
    const SDL_Color COLOR_BLUE_PLAYER = {64, 63, 180, 255};
    const SDL_Color COLOR_YELLOW_OTHER = {255, 255, 0, 255};

    const SDL_Color UI_BACKGROUND_COLOR = {10, 10, 10, 255};
    const SDL_Color MAP_BACKGROUND_COLOR = {10, 10, 10, 255};
    const SDL_Color DEFAULT_TEXT_COLOR = {200, 200, 200, 255};
	
	#define MAX_DISPLAY_ITEMS_IN_PANEL 10

} // namespace RTSEngine

#endif // RTSENGINE_CONFIG_H