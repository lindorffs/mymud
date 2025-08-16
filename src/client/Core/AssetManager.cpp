#include "AssetManager.h"
#include <SDL3/SDL_image.h>
#include <iostream>

namespace RTSEngine {
    namespace Core {

        AssetManager::AssetManager() : renderer_(nullptr) {}

        AssetManager::~AssetManager() {
            destroyAssets();
        }

        void AssetManager::init(SDL_Renderer* renderer) {
            renderer_ = renderer;
        }

        SDL_Texture* AssetManager::loadTexture(const std::string& id, const std::string& path) {
            if (!renderer_) {
                std::cerr << "AssetManager Error: Renderer not initialized before loading texture " << id << std::endl;
                return nullptr;
            }
            if (textures_.count(id)) {
                std::cerr << "AssetManager Warning: Texture with id '" << id << "' already loaded. Returning existing." << std::endl;
                return textures_[id];
            }

            SDL_Surface* surface = IMG_Load(path.c_str());
            if (!surface) {
                std::cerr << "AssetManager Error: Failed to load image " << path << ": " << SDL_GetError() << std::endl;
                return nullptr;
            }

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            SDL_DestroySurface(surface);

            if (!texture) {
                std::cerr << "AssetManager Error: Failed to create texture from " << path << ": " << SDL_GetError() << std::endl;
                return nullptr;
            }
            textures_[id] = texture;
            return texture;
        }

        SDL_Texture* AssetManager::getTexture(const std::string& id) const {
            auto it = textures_.find(id);
            if (it != textures_.end()) {
                return it->second;
            }
            std::cerr << "AssetManager Error: Texture with id '" << id << "' not found." << std::endl;
            return nullptr;
        }

        TTF_Font* AssetManager::loadFont(const std::string& id, const std::string& path, int size) {
             if (fonts_.count(id)) {
                std::cerr << "AssetManager Warning: Font with id '" << id << "' already loaded. Returning existing." << std::endl;
                return fonts_[id];
            }
            TTF_Font* font = TTF_OpenFont(path.c_str(), size);
            if (!font) {
                std::cerr << "AssetManager Error: Failed to load font " << path << ": " << SDL_GetError() << std::endl;
                return nullptr;
            }
            fonts_[id] = font;
            return font;
        }

        TTF_Font* AssetManager::getFont(const std::string& id) const {
            auto it = fonts_.find(id);
            if (it != fonts_.end()) {
                return it->second;
            }
            std::cerr << "AssetManager Error: Font with id '" << id << "' not found." << std::endl;
            return nullptr;
        }

        void AssetManager::destroyAssets() {
            for (auto const& [id, texture] : textures_) {
                SDL_DestroyTexture(texture);
            }
            textures_.clear();

            for (auto const& [id, font] : fonts_) {
                TTF_CloseFont(font);
            }
            fonts_.clear();
        }

    } // namespace Core
} // namespace RTSEngine