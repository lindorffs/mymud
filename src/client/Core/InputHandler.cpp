#include "InputHandler.h"
#include "Game.h" // For game.quit()
#include "../UI/CommandLine.h"
#include <SDL3/SDL.h>


#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
namespace RTSEngine {
    namespace Core {

        InputHandler::InputHandler() {}

        bool InputHandler::processInput(Core::Game& game,
                                          SDL_Window* window,
                                          UI::CommandLine& commandLine) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_QUIT) return false;
			
			   ImGui_ImplSDL3_ProcessEvent(&event);
				ClientGameState currentState = game.getClientGameState(); // Get current state
				if (event.type == SDL_EVENT_KEY_DOWN) {
					if (event.key.key == SDLK_RETURN) {
						game.getAudioManager().playSoundEffect("command_enter");
					} else {
						game.getAudioManager().playSoundEffect("keypress");
					}
				}
				if (currentState == ClientGameState::MAIN_MENU) {
				   
			   } else if (currentState == ClientGameState::IN_LOBBY || currentState == ClientGameState::IN_GAME) {
				   
			   }  else if (currentState == ClientGameState::DISCONNECTED) {
					
				}
			   // Window resize should always call game.handleResize()
			   if (event.type == SDL_EVENT_WINDOW_RESIZED) game.handleResize();
			   }
			return true;   
		}
    } // namespace Core
} // namespace RTSEngine