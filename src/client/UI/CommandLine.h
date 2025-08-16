#ifndef RTSENGINE_COMMANDLINE_H
#define RTSENGINE_COMMANDLINE_H

#include "CommandLog.h" // For CommandLogEntry, CommandHistory
#include <string>
#include <vector>
#include <list> // For allUnits list
#include <SDL3/SDL_keycode.h> // For SDL_Keycode


// Forward declarations
namespace RTSEngine {
	namespace Core { class Game; }
    namespace UI { class SelectionManager; }
    namespace World { class TileMap; class Camera; }
    namespace GameObjects { struct Unit; }
}

namespace RTSEngine {
    namespace UI {

        class CommandLine {
        private:
            std::string commandBuffer_;
            CommandHistory commandHistory_;
            bool showCommandLine_; // Could be controlled by a toggle command later

            // Helper for parsing
            
            // The actual interpretation logic

        public:
            CommandLine();
            std::string interpretCommand(const std::string& command_name, const std::vector<std::string>& args, const std::string& full_command,
										 Core::Game* game); // Add other refs as needed (e.g. Camera)
            std::vector<std::string> parseFullCommand(const std::string& input, std::string& command_name_out);

            void handleTextInput(const char* text);
            void handleKeyDown(SDL_Keycode key,
							   Core::Game& game); // Add other refs as needed

            const std::string& getCommandBuffer() const;
            const CommandHistory& getCommandHistory() const;
            bool isVisible() const;
            void addLogEntry(const std::string& command, const std::string& output);
        };

    } // namespace UI
} // namespace RTSEngine

#endif // RTSENGINE_COMMANDLINE_H