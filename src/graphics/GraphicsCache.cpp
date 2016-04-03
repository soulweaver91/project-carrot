#include "GraphicsCache.h"

GraphicsCache::GraphicsCache() {
}

GraphicsCache::~GraphicsCache() {
}

std::shared_ptr<sf::Texture> GraphicsCache::request(const QString& filename) {
    if (!cache.contains(filename)) {
        // try to add the texture to the cache
        auto t = std::make_shared<sf::Texture>();
        if (t->loadFromFile(filename.toStdString())) {
            cache.insert(filename, t);
            return t;
        } else {
            return nullptr;
        }
    }
    return cache.value(filename);
}

bool GraphicsCache::flush() {
    cache.clear();
    return true;
}

bool GraphicsCache::loadGraphics(const QString& filename) {
    return false;
}
