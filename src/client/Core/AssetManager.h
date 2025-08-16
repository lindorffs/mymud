#ifndef RTSENGINE_ASSETMANAGER_H
#define RTSENGINE_ASSETMANAGER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <string>
#include <map>

namespace RTSEngine {
    namespace Core {

        class AssetManager {
        private:
            SDL_Renderer* renderer_; // Needed for texture creation
            std::map<std::string, SDL_Texture*> textures_;
            std::map<std::string, TTF_Font*> fonts_;

        public:
            AssetManager();
            ~AssetManager();

            void init(SDL_Renderer* renderer); // Must be called after renderer is created

            SDL_Texture* loadTexture(const std::string& id, const std::string& path);
            SDL_Texture* getTexture(const std::string& id) const;

            TTF_Font* loadFont(const std::string& id, const std::string& path, int size);
            TTF_Font* getFont(const std::string& id) const;

            void destroyAssets();
        };

    } // namespace Core
} // namespace RTSEngine

#endif // RTSENGINE_ASSETMANAGER_H