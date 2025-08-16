      
#include "AudioManager.h"

namespace RTSEngine {
    namespace Core {

        AudioManager::AudioManager() {}

        AudioManager::~AudioManager() {
            quit(); // Ensure cleanup
        }

        bool AudioManager::init() {
            if (initialized_) {
                std::cout << "AudioManager already initialized." << std::endl;
                return true;
            }

            // Initialize SDL_mixer
            // Common flags: MIX_INIT_MP3, MIX_INIT_OGG, MIX_INIT_FLAC, MIX_INIT_MOD
            // You need to link the appropriate decoder libraries if you use specific formats beyond WAV.
            // For simple WAVs, you might not need any MIX_Init flags,
            // but it's good practice if you plan to use other formats.
            // Let's try initializing for OGG and MP3, common formats.
            int flags = MIX_INIT_OGG | MIX_INIT_MP3;
            int initted = Mix_Init(flags);
            if ((initted & flags) != flags) {
                std::cerr << "AudioManager Error: Mix_Init failed to initialize required decoders! Error: " << SDL_GetError() << std::endl;
                // You can still proceed if only WAV is needed, but log this.
                // For now, let's consider it a failure if decoders aren't up.
                // return false; // Strict: if decoders fail, init fails.
            }
            SDL_AudioSpec audioSpec;
			audioSpec.format = MIX_DEFAULT_FORMAT;
			audioSpec.channels = 2;
			audioSpec.freq = 2048;
            // Open audio device: frequency, format, channels, chunksize
            // Common values: 44100 Hz, MIX_DEFAULT_FORMAT (signed 16-bit), 2 (stereo), 2048 bytes per sample frame
            if (Mix_OpenAudio(0, NULL) == false) {
                std::cerr << "AudioManager Error: Mix_OpenAudio failed! Error: " << SDL_GetError() << std::endl;
                Mix_Quit(); // Quit the parts of MIX_Init that succeeded
                return false;
            }

            // Allocate channels (optional, default is 8, can increase if needed)
            // Mix_AllocateChannels(16); 

            std::cout << "AudioManager initialized successfully." << std::endl;
            initialized_ = true;
            return true;
        }

        void AudioManager::quit() {
            if (!initialized_) return;

            std::cout << "AudioManager quitting..." << std::endl;
            // Free all loaded sound effects
            for (auto const& [id, chunk] : sound_effects_) {
                if (chunk) {
                    Mix_FreeChunk(chunk);
                }
            }
            sound_effects_.clear();

            // Free all loaded music (if implemented)
            // for (auto const& [id, music] : music_tracks_) {
            //     if (music) {
            //         Mix_FreeMusic(music);
            //     }
            // }
            // music_tracks_.clear();

            Mix_CloseAudio(); // Close the audio device
            
            // Quit initialized subsystems (from Mix_Init)
            // This loop is important to quit all subsystems Mix_Init might have started.
            while(Mix_Init(0)) { // Call Mix_Init(0) to get current initted flags
                Mix_Quit();      // Quit one subsystem at a time
            }
            
            initialized_ = false;
        }

        bool AudioManager::loadSoundEffect(const std::string& id, const std::string& filepath) {
            if (!initialized_) {
                std::cerr << "AudioManager Error: Cannot load sound effect, AudioManager not initialized." << std::endl;
                return false;
            }
            if (sound_effects_.count(id)) {
                std::cout << "AudioManager Warning: Sound effect with ID '" << id << "' already loaded. Overwriting." << std::endl;
                if (sound_effects_[id]) Mix_FreeChunk(sound_effects_[id]);
            }

            Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
            if (!chunk) {
                std::cerr << "AudioManager Error: Failed to load sound effect '" << filepath << "'. Error: " << SDL_GetError() << std::endl;
                return false;
            }
            sound_effects_[id] = chunk;
            Mix_VolumeChunk(chunk, master_sfx_volume_); // Apply master SFX volume
            std::cout << "AudioManager: Loaded sound effect '" << id << "' from " << filepath << std::endl;
            return true;
        }

        void AudioManager::playSoundEffect(const std::string& id, int loops, int channel) {
            if (!initialized_) return;
            auto it = sound_effects_.find(id);
            if (it != sound_effects_.end() && it->second) {
                // channel -1 means play on the first free channel
                // loops 0 means play once, 1 means play twice, etc. -1 means loop indefinitely (not for chunks usually)
                if (Mix_PlayChannel(channel, it->second, loops) == -1) {
                    // std::cerr << "AudioManager Warning: Mix_PlayChannel failed for '" << id << "'. Error: " << SDL_GetError() << std::endl;
                    // This can happen if all channels are busy. You might want to manage channels more carefully
                    // or increase allocated channels.
                }
            } else {
                std::cerr << "AudioManager Warning: Sound effect with ID '" << id << "' not found or null." << std::endl;
            }
        }
        
        void AudioManager::setMasterVolumeSFX(int volume) {
            if (!initialized_) return;
            master_sfx_volume_ = std::max(0, std::min(volume, MIX_MAX_VOLUME)); // Clamp 0-128
            // Apply to all currently loaded chunks
            for (auto const& [id, chunk] : sound_effects_){
                if(chunk) Mix_VolumeChunk(chunk, master_sfx_volume_);
            }
            std::cout << "AudioManager: Master SFX volume set to " << master_sfx_volume_ << std::endl;
        }


        void AudioManager::setSoundEffectVolume(const std::string& id, int volume) {
            if (!initialized_) return;
             auto it = sound_effects_.find(id);
            if (it != sound_effects_.end() && it->second) {
                Mix_VolumeChunk(it->second, std::max(0, std::min(volume, MIX_MAX_VOLUME)));
            } else {
                 std::cerr << "AudioManager Warning: Sound effect ID '" << id << "' not found for setting volume." << std::endl;
            }
        }


        // Music functions (placeholders)
        // bool AudioManager::loadMusic(const std::string& id, const std::string& filepath) { /* ... Mix_LoadMUS ... */ return false; }
        // void AudioManager::playMusic(const std::string& id, int loops) { /* ... Mix_PlayMusic ... */ }
        // void AudioManager::stopMusic() { /* ... Mix_HaltMusic ... */ }
        // void AudioManager::setMusicVolume(int volume) { /* ... Mix_VolumeMusic ... */ }


    } // namespace Core
} // namespace RTSEngine

    