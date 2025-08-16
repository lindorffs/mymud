#ifndef RTSENGINE_UIMANAGER_H
#define RTSENGINE_UIMANAGER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <string>
#include <vector>
#include <list> // For allUnits
#include <deque> // For CommandHistory

#include "../../shared/LobbyTypes.h" // Include the new header

// Forward declarations
namespace RTSEngine {
    namespace Core { class AssetManager; class Game; class PlayerResources; }
    namespace World { class TileMap; class Camera; }
    namespace GameObjects { struct Unit; struct Building; }
    namespace UI { class CommandLine; class SelectionManager; struct CommandLogEntry; }
}

namespace RTSEngine {
    namespace UI {

		struct NineSliceBorders {
            float left;
            float right;
            float top;
            float bottom;
        };

        // Helper function to render a texture using 9-slice scaling
        void renderNineSlicePanel(
            SDL_Renderer* renderer,
            SDL_Texture* texture,
            const NineSliceBorders& borders, // Defines the non-stretchable border sizes in the source texture
            const SDL_FRect& dest_rect       // The target rectangle on the screen to render the panel into
        );


        class UIManager {
        private:
            SDL_Renderer* renderer_;
            SDL_Window* window_; // For getting window size
            Core::AssetManager& assetManager_; // For fonts and UI textures

            // Rendering sub-components
            float renderCommandLinePanel(const CommandLine& cmdLine); // Returns top Y of the command line input area
            void renderCommandLogPanel(Core::Game* game);
            void renderSelectionInfoPanel(const SelectionManager& selManager, const std::list<GameObjects::Unit>& allUnits, const std::list<GameObjects::Building>& allBuildings, const Core::Game* game);
            void renderPlayingFieldPanel(const World::TileMap& tileMap, const World::Camera& camera, const std::list<GameObjects::Unit>& allUnits, const Core::Game* game);

			      
		void renderTopBar(Core::Game* game);
		SDL_FRect top_bar_rect_; // Store its dimensions
		SDL_FRect menu_button_rect_; // For click detection
		SDL_FRect bottom_cli_panel_rect_; // Store its dimensions
		SDL_FRect side_panel_rect_; // Store its dimensions
			

        public:
            UIManager(SDL_Renderer* renderer, SDL_Window* window, Core::AssetManager& assetManager);

			void renderMainMenu(const std::vector<std::string>& options, int selectedOption, Core::Game* game);
			void renderConnectingScreen(const std::string& statusMessage);
			void renderLobbyScreen(const std::map<int, Core::LobbyPlayerInfo>& players, int myId, int maxPlayers, const CommandLine& cmdLine, Core::Game* game);
			void renderLoadingScreen(const std::string& message);
			void renderDisconnectedScreen(const std::string& message, Core::Game* game);
			void renderCharacterInfo(Core::Game *game);
			void renderText(const std::string& text, float x, float y, TTF_Font* font, SDL_Color color);
			void renderMap(Core::Game* game);
            void renderAll(const CommandLine& cmdLine,
						   Core::Game* game);
        };

    } // namespace UI
} // namespace RTSEngine

#endif // RTSENGINE_UIMANAGER_H