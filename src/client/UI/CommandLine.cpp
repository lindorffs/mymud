#include "CommandLine.h"
#include "../Core/Game.h"
#include "../Config.h" // For MAX_COMMAND_HISTORY
#include <sstream>
#include <iostream> // For placeholder output, should be routed to command log
#include <algorithm> // For std::transform
#include <cstdio> // For sscanf


namespace RTSEngine {
    namespace UI {

        CommandLine::CommandLine() : showCommandLine_(true) {}

        void CommandLine::handleTextInput(const char* text) {
            if (showCommandLine_) {
                commandBuffer_ += text;
            }
        }

        std::vector<std::string> CommandLine::parseFullCommand(const std::string& input, std::string& command_name_out) {
            std::istringstream stream(input);
            std::string word;
            std::vector<std::string> args_out;

            stream >> command_name_out; 
            std::transform(command_name_out.begin(), command_name_out.end(), command_name_out.begin(), ::tolower);
            
            while (stream >> word) {    
                args_out.push_back(word);
            }
            return args_out;
        }

        std::string CommandLine::interpretCommand(const std::string& lower_command_name, const std::vector<std::string>& args, const std::string& full_command,
											 Core::Game* game_instance) {
            std::ostringstream output; // Capture output for the log

            if (lower_command_name == "clearlog"  || lower_command_name == "cllg") { // Local command
				commandHistory_.clear();
			} else if (lower_command_name == "help" || lower_command_name == "hlp") { // Local command
				 std::string helpText = "Available commands:\n";
				addLogEntry(full_command, helpText);
			} else if (lower_command_name == "say" || lower_command_name == "s") { // New chat command
				if (args.empty()) {
					addLogEntry(lower_command_name, "Usage: say <message> or s <message>");
				} else {
					// Reconstruct the full message from args
					std::string chat_message_text;
					for (size_t i = 0; i < args.size(); ++i) {
						chat_message_text += args[i] + (i == args.size() - 1 ? "" : " ");
					}
					// Pass the reconstructed message as a single argument to the server command "chat"
					game_instance->client_package_and_send_command("chat_message", {chat_message_text});
				}	
			}  else {
				game_instance->client_package_and_send_command(lower_command_name, args);
			}
            
            return output.str();
        }


        void CommandLine::handleKeyDown(SDL_Keycode key,
										Core::Game& game) {

        }
        
        void CommandLine::addLogEntry(const std::string& command, const std::string& output) {
            commandHistory_.push_back({"",command + ": " + output});
            if (commandHistory_.size() > MAX_COMMAND_HISTORY) {
                commandHistory_.pop_front();
            }
        }

        const std::string& CommandLine::getCommandBuffer() const { return commandBuffer_; }
        const CommandHistory& CommandLine::getCommandHistory() const { return commandHistory_; }
        bool CommandLine::isVisible() const { return showCommandLine_; }

    } // namespace UI
} // namespace RTSEngine