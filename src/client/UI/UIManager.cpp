#include "UIManager.h"
#include "../Core/AssetManager.h"
#include "../Core/Game.h"
#include "CommandLine.h"
#include "../Utils/RenderUtils.h" // For getOwnerColor
#include "../Config.h" // For TILE_SIZE, panel dimensions, colors
#include <SDL3/SDL_image.h> // For potential UI textures, not strictly needed for text
#include <sstream>
#include <algorithm>
#include <iostream> // For error messages


namespace RTSEngine {
    namespace UI {
		
		static bool render_logout_confirm = false;
		static bool render_quit_confirm = false;
		static bool render_disconnect_confirm = true;
		static bool render_system_list = false;
		static bool render_command_line = false;
		const NineSliceBorders side_panel_borders = {11.0f, 11.0f, 11.0f, 11.0f}; // Example: 16px borders
		const NineSliceBorders top_bar_borders    = {5.0f, 5.0f, 5.0f, 5.0f};    // Example
		const NineSliceBorders bottom_panel_borders = {11.0f, 11.0f, 11.0f, 11.0f};
		const NineSliceBorders selection_item_shadow_borders = {3.0f, 3.0f, 3.0f, 3.0f}; // Example
		void renderNineSlicePanel(
            SDL_Renderer* renderer,
            SDL_Texture* texture,
            const NineSliceBorders& borders,
            const SDL_FRect& dest_rect)
        {
            if (!renderer || !texture || dest_rect.w <= 0 || dest_rect.h <= 0) {
                return;
            }

            float tex_w, tex_h;
            SDL_GetTextureSize(texture, &tex_w, &tex_h);

            // Source rectangles for the 9 slices from the texture
            SDL_FRect src_rects[9];
            // Destination rectangles for the 9 slices on the screen
            SDL_FRect dst_rects[9];

            // --- Define Source Rectangles (from the texture) ---
            // Top-Left Corner
            src_rects[0] = {0, 0, borders.left, borders.top};
            // Top Edge
            src_rects[1] = {borders.left, 0, tex_w - borders.left - borders.right, borders.top};
            // Top-Right Corner
            src_rects[2] = {tex_w - borders.right, 0, borders.right, borders.top};

            // Middle-Left Edge
            src_rects[3] = {0, borders.top, borders.left, tex_h - borders.top - borders.bottom};
            // Center
            src_rects[4] = {borders.left, borders.top, tex_w - borders.left - borders.right, tex_h - borders.top - borders.bottom};
            // Middle-Right Edge
            src_rects[5] = {tex_w - borders.right, borders.top, borders.right, tex_h - borders.top - borders.bottom};

            // Bottom-Left Corner
            src_rects[6] = {0, tex_h - borders.bottom, borders.left, borders.bottom};
            // Bottom Edge
            src_rects[7] = {borders.left, tex_h - borders.bottom, tex_w - borders.left - borders.right, borders.bottom};
            // Bottom-Right Corner
            src_rects[8] = {tex_w - borders.right, tex_h - borders.bottom, borders.right, borders.bottom};

            // --- Define Destination Rectangles (on the screen) ---
            // Calculate stretchable center width and height
            // Ensure these are not negative if dest_rect is smaller than combined borders
            float center_w = std::max(0.0f, dest_rect.w - borders.left - borders.right);
            float center_h = std::max(0.0f, dest_rect.h - borders.top - borders.bottom);

            // Top-Left Corner
            dst_rects[0] = {dest_rect.x, dest_rect.y, borders.left, borders.top};
            // Top Edge
            dst_rects[1] = {dest_rect.x + borders.left, dest_rect.y, center_w, borders.top};
            // Top-Right Corner
            dst_rects[2] = {dest_rect.x + borders.left + center_w, dest_rect.y, borders.right, borders.top};

            // Middle-Left Edge
            dst_rects[3] = {dest_rect.x, dest_rect.y + borders.top, borders.left, center_h};
            // Center
            dst_rects[4] = {dest_rect.x + borders.left, dest_rect.y + borders.top, center_w, center_h};
            // Middle-Right Edge
            dst_rects[5] = {dest_rect.x + borders.left + center_w, dest_rect.y + borders.top, borders.right, center_h};

            // Bottom-Left Corner
            dst_rects[6] = {dest_rect.x, dest_rect.y + borders.top + center_h, borders.left, borders.bottom};
            // Bottom Edge
            dst_rects[7] = {dest_rect.x + borders.left, dest_rect.y + borders.top + center_h, center_w, borders.bottom};
            // Bottom-Right Corner
            dst_rects[8] = {dest_rect.x + borders.left + center_w, dest_rect.y + borders.top + center_h, borders.right, borders.bottom};

            // --- Render the 9 Slices ---
            for (int i = 0; i < 9; ++i) {
                // Skip rendering if source or destination slice has zero width or height
                if (src_rects[i].w <= 0 || src_rects[i].h <= 0 || dst_rects[i].w <= 0 || dst_rects[i].h <= 0) {
                    continue;
                }
                SDL_RenderTexture(renderer, texture, &src_rects[i], &dst_rects[i]);
            }
        }


        UIManager::UIManager(SDL_Renderer* renderer, SDL_Window* window, Core::AssetManager& assetManager)
            : renderer_(renderer), window_(window), assetManager_(assetManager) {}

		

        void UIManager::renderAll(const CommandLine& cmdLine,
								  Core::Game* game) {
            if (!renderer_ || !window_) return;
			
            // Main background clear is done by Game class

            //float commandLineActualTopY = renderCommandLinePanel(cmdLine);
            //renderCommandLogPanel(cmdLine, commandLineActualTopY);
            //renderPlayingFieldPanel(tileMap, camera, allUnits, game);
            //renderSelectionInfoPanel(selManager, allUnits, allBuildings, game);
			renderTopBar(game);
        }

		void UIManager::renderTopBar(Core::Game* game) {
			
		}
		
		
		void UIManager::renderMap(Core::Game *game) {
			int winW, winH;
			SDL_GetWindowSizeInPixels(game->get_window(), &winW, &winH);
			
			ImGui::SetNextWindowPos(ImVec2(winW-(winW*0.95), winH-(winH*0.95)), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(winW*0.9, winH*0.9), ImGuiCond_FirstUseEver);
			
			if (!ImGui::Begin("systems.db", &render_system_list)) {
				
				ImGui::End();
				return;
			} else {
				
				ImGui::End();
			}
		}

		// Helper (add to UIManager or a Utils class)
		void UIManager::renderText(const std::string& text, float x, float y, TTF_Font* font, SDL_Color color) {
			if (text.empty() || !font) return;
			SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), 0, color);
			if (!surf) { std::cerr << "TTF_RenderText_Blended Error: " << SDL_GetError() << std::endl; return; }
			SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surf);
			SDL_FRect dest = {x, y, (float)surf->w, (float)surf->h};
			SDL_DestroySurface(surf);
			if (tex) {
				SDL_RenderTexture(renderer_, tex, nullptr, &dest);
				SDL_DestroyTexture(tex);
			} else { std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl; }
		}

		static char cmdinput[64] = {'\0'};
        void UIManager::renderCommandLogPanel(Core::Game* game) {
			int winW, winH;
			SDL_GetWindowSizeInPixels(game->get_window(), &winW, &winH);
			
			ImGui::SetNextWindowPos(ImVec2(winW/2.0f - 366, winH/2-180), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(632, 360), ImGuiCond_FirstUseEver);
			
			// Begin the window. Note that ImGui::End() should always be called.
			if (!ImGui::Begin("cmd.exe", &render_command_line)) {
				ImGui::End();
				return;
			}

			// --- Font Application Start ---
			// We'll use the description font for the entire panel (input and history) for a consistent look.
			// If the font is not available, it will gracefully use the default font.
			if (game->font_description) {
				ImGui::PushFont(game->font_description);
			}
			
			const auto& history = game->getMutableCommandLine().getCommandHistory();
			
			ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
			
			// --- History Log Child Window ---
			// Calculate available height for the history, leaving space for the input box at the bottom.
			float inputTextHeight = ImGui::GetFrameHeightWithSpacing(); 
			ImVec2 historySize = ImVec2(0, contentRegionAvail.y - inputTextHeight - ImGui::GetStyle().ItemSpacing.y);
			
			ImGui::BeginChild("history", historySize, true);
			for (const auto& entry : history) {
				// Your existing logic for displaying lines is fine.
				std::istringstream outputStream(entry.output);
				std::string tempLine;
				while (std::getline(outputStream, tempLine, '\n')) {
					ImGui::TextWrapped("%s", tempLine.c_str()); // Use TextWrapped for long lines
				}
			}
			// Auto-scroll to the bottom if the window is focused
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
			}
			ImGui::EndChild();

			// --- Input Text Box ---
			ImGui::PushItemWidth(-1); // Use full available width
			if (ImGui::InputText("##Input", cmdinput, sizeof(cmdinput), ImGuiInputTextFlags_EnterReturnsTrue)) {
				std::string lower_command_name = std::string(cmdinput);
				std::string command_name;
				std::vector<std::string> args = game->getMutableCommandLine().parseFullCommand(lower_command_name, command_name);
				
				if (command_name == "systems.db") {
					render_system_list = !render_system_list;
				} else {
					std::string output = game->getMutableCommandLine().interpretCommand(command_name, args, lower_command_name, game);
				}
				// Clear the input buffer safely
				memset(cmdinput, 0, sizeof(cmdinput));
				ImGui::SetKeyboardFocusHere(-1); // Refocus the input
			}
			ImGui::PopItemWidth();

			// --- Font Application End ---
			// Pop the font after all elements in this panel have been rendered.
			if (game->font_description) {
				ImGui::PopFont();
			}
			
			ImGui::End();
		}

		void UIManager::renderMainMenu(const std::vector<std::string>& options, int selectedOption, Core::Game* game) {
			int winW, winH;
			SDL_GetWindowSizeInPixels(window_, &winW, &winH);
			
			// Main Menu Bar (generally uses default font, but you could change it if desired)
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("Server Selection")) {
					if (ImGui::MenuItem("Comodore - Development")) {
						
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			
			// Login Window
			ImGui::SetNextWindowPos(ImVec2(winW/2.0f - 260, winH/2-90), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(500, 180));
			
			// As before, always call ImGui::End() after a Begin()
			if (!ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::End();
				return;
			}

			static char userBuffer[64] = {'\0'};
			static char passwordBuffer[64] = {'\0'};

			// --- Username Field ---
			// Push subtitle font for the label
			if (game->font_subtitle) ImGui::PushFont(game->font_subtitle);
			ImGui::Text("Username:");
			if (game->font_subtitle) ImGui::PopFont();
			
			ImGui::SameLine();
			ImGui::InputText("##Username", userBuffer, sizeof(userBuffer));

			// --- Password Field ---
			// Push subtitle font for the label
			if (game->font_subtitle) ImGui::PushFont(game->font_subtitle);
			ImGui::Text("Password:");
			if (game->font_subtitle) ImGui::PopFont();
			
			ImGui::SameLine();
			ImGui::InputText("##Password", passwordBuffer, sizeof(passwordBuffer), ImGuiInputTextFlags_Password);
			
			// --- Login Button ---
			// Push subtitle font for the button text to make it stand out
			if (game->font_subtitle) ImGui::PushFont(game->font_subtitle);
			if (ImGui::Button("Login", ImVec2(120, 0))) { // Example of setting a button size
				game->clientInitiateConnection(userBuffer, passwordBuffer);
				memset(passwordBuffer, 0, sizeof(passwordBuffer));
				memset(userBuffer, 0, sizeof(userBuffer));
			}
			if (game->font_subtitle) ImGui::PopFont();
			
			ImGui::End();
			
			
		}

		void UIManager::renderConnectingScreen(const std::string& statusMessage) {
			// Similar: Render "Connecting..." and statusMessage centered.
			TTF_Font* font = assetManager_.getFont("commandFont"); // Ensure font is loaded
			if (!font || !renderer_ || !window_) return;
			int winW, winH;
			SDL_GetWindowSizeInPixels(window_, &winW, &winH);

			SDL_Surface* surface = TTF_RenderText_Blended(font, statusMessage.c_str(), 0, DEFAULT_TEXT_COLOR);
			if (surface) {
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
				if (texture) {
					SDL_FRect dest = {(winW - surface->w) / 2.0f, (winH - surface->h) / 2.0f, (float)surface->w, (float)surface->h};
					SDL_RenderTexture(renderer_, texture, nullptr, &dest);
					SDL_DestroyTexture(texture);
				}
				SDL_DestroySurface(surface);
			}
		}

		void render_ingame_system_menu(Core::Game* game) {
		}
		void UIManager::renderLobbyScreen(const std::map<int, Core::LobbyPlayerInfo>& players, int myId, int maxPlayers, const CommandLine& cmdLine, Core::Game* game) {

			int winW, winH;
			SDL_GetWindowSizeInPixels(game->get_window(), &winW, &winH);
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Main")) {
					if (ImGui::MenuItem("logout.cmd")) {
						render_logout_confirm = true;
					}
					if (ImGui::MenuItem("exit.cmd")) {
						render_quit_confirm = true;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Commands")) {
					if (ImGui::MenuItem("cmd.exe")) {
						render_command_line = !render_command_line;
					}
					ImGui::EndMenu();
				}
				
				if (ImGui::BeginMenu("Info")) {
					if (ImGui::MenuItem("systems.db")) {
						render_system_list = !render_system_list;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			
			if (render_logout_confirm) {
				ImGui::SetNextWindowPos(ImVec2(winW/2.0f - 260, winH/2-90), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(500, 180));
				ImGui::OpenPopup("Logout?");
				if (ImGui::BeginPopupModal("Logout?", &render_logout_confirm, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("You can always come back later.");
					ImGui::Separator();

					if (ImGui::Button("Yes", ImVec2(120, 0)))
					{
						game->handleDisconnection("Logged Out!");
						ImGui::CloseCurrentPopup(); // Close the dialog
						render_logout_confirm = false;
						render_disconnect_confirm = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(120, 0)))
					{
						ImGui::CloseCurrentPopup(); // Close the dialog
						render_logout_confirm = false;
					}
					ImGui::EndPopup();
				}
			}
			if (render_quit_confirm) {
				ImGui::SetNextWindowPos(ImVec2(winW/2.0f - 260, winH/2-90), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(500, 180));
				ImGui::OpenPopup("Exit to Desktop?");
				if (ImGui::BeginPopupModal("Exit to Desktop?", &render_quit_confirm, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("You can always come back later.");
					ImGui::Separator();

					if (ImGui::Button("Yes", ImVec2(120, 0)))
					{
						game->quitGame();
						ImGui::CloseCurrentPopup(); // Close the dialog
						render_quit_confirm = false;
					}
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(120, 0)))
					{
						ImGui::CloseCurrentPopup(); // Close the dialog
						render_quit_confirm = false;
					}
					ImGui::EndPopup();
				}
			}
			if (render_command_line) {
				renderCommandLogPanel(game);
			}
			if (render_system_list) {
				renderMap(game);
			}
		}

		void UIManager::renderLoadingScreen(const std::string& message) {
			
			TTF_Font* font = assetManager_.getFont("commandFont"); // Ensure font is loaded
			if (!font || !renderer_ || !window_) return;
			int winW, winH;
			SDL_GetWindowSizeInPixels(window_, &winW, &winH);

			SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), 0, DEFAULT_TEXT_COLOR);
			if (surface) {
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
				if (texture) {
					SDL_FRect dest = {(winW - surface->w) / 2.0f, (winH - surface->h) / 2.0f, (float)surface->w, (float)surface->h};
					SDL_RenderTexture(renderer_, texture, nullptr, &dest);
					SDL_DestroyTexture(texture);
				}
				SDL_DestroySurface(surface);
			}
			
		}
		void UIManager::renderDisconnectedScreen(const std::string& message, Core::Game* game) {
			int winW, winH;
			SDL_GetWindowSizeInPixels(window_, &winW, &winH);
						
			ImGui::SetNextWindowPos(ImVec2(winW/2.0f - 260, winH/2-90), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(500, 180));
			
			// As before, always call ImGui::End() after a Begin()
			if (ImGui::Begin("Disconnected", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::Text(message.c_str());
				ImGui::Separator();
				if (ImGui::Button("Ok", ImVec2(120, 0)))
				{
					game->transitionToState(Core::ClientGameState::MAIN_MENU);
				}
				
				ImGui::End();
			}		
		}

    } // namespace UI
} // namespace RTSEngine