#ifndef RTSENGINE_COMMANDLOG_H
#define RTSENGINE_COMMANDLOG_H

#include <string>
#include <deque>

namespace RTSEngine {
    namespace UI {

        struct CommandLogEntry {
            std::string command;
            std::string output;
        };

        using CommandHistory = std::deque<CommandLogEntry>;

    } // namespace UI
} // namespace RTSEngine

#endif // RTSENGINE_COMMANDLOG_H