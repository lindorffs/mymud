#ifndef RTSENGINE_INPUTHANDLER_H
#define RTSENGINE_INPUTHANDLER_H

#include <SDL3/SDL_events.h>
#include <list> // For allUnits

// Forward declarations
struct SDL_Window;

namespace RTSEngine {
    namespace UI { class CommandLine; class SelectionManager; }
    namespace World { class Camera; class TileMap; }
    namespace GameObjects { struct Unit; }
    namespace Core { class Game; } // To signal quit
}

namespace RTSEngine {
    namespace Core {

        class InputHandler {
        public:
            InputHandler();
            // processInput takes references to systems that need to react to input.
            // Returns false if SDL_QUIT event is received.
            bool processInput(Core::Game& game, // To set isRunning = false
                              SDL_Window* window, // For text input context
                              UI::CommandLine& commandLine); 
        };

    } // namespace Core
} // namespace RTSEngine

#endif // RTSENGINE_INPUTHANDLER_H