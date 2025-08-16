#ifndef RTSENGINE_AUDIOMANAGER_H
#define RTSENGINE_AUDIOMANAGER_H

#include <SDL3/SDL_mixer.h> // Main SDL_mixer header
#include <string>
#include <map>
#include <iostream> // For error messages

namespace RTSEngine {
    namespace Core {

        class AudioManager {
        public:
            AudioManager();
            ~AudioManager();

            bool init(); // Initialize SDL_mixer
            void quit(); // Quit SDL_mixer

            // Sound Effects (Chunks)
            bool loadSoundEffect(const std::string& id, const std::string& filepath);
            void playSoundEffect(const std::string& id, int loops = 0, int channel = -1); // channel -1 for first free
            void setSoundEffectVolume(const std::string& id, int volume); // 0-128
            void setMasterVolumeSFX(int volume); // 0-128 for all sound effects

            // Music (Not used yet, but good to have placeholders)
            // bool loadMusic(const std::string& id, const std::string& filepath);
            // void playMusic(const std::string& id, int loops = -1); // -1 for infinite
            // void stopMusic();
            // void pauseMusic();
            // void resumeMusic();
            // void setMusicVolume(int volume); // 0-128

        private:
            bool initialized_ = false;
            std::map<std::string, Mix_Chunk*> sound_effects_;
            // std::map<std::string, Mix_Music*> music_tracks_;

            int master_sfx_volume_ = MIX_MAX_VOLUME; // SDL_mixer default is 128
        };

    } // namespace Core
} // namespace RTSEngine

#endif // RTSENGINE_AUDIOMANAGER_H