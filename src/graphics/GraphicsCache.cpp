#include "GraphicsCache.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

const GraphicResource GraphicsCache::defaultResource = {
    {},
    nullptr,
    sf::Vector2i(0, 0),
    sf::Vector2i(0, 0),
    0,
    0,
    0,
    sf::Vector2i(0, 0),
    sf::Vector2i(0, 0),
    sf::Vector2i(0, 0),
    false,
    false
};

GraphicsCache::GraphicsCache() {
}

GraphicsCache::~GraphicsCache() {
}

std::shared_ptr<GraphicResource> GraphicsCache::request(const QString& filename) {
    if (!cache.contains(filename)) {
        QString absolutePath = QDir::current().absoluteFilePath("Data/Assets/" + filename);
        // try to add the texture to the cache
        auto t = std::make_shared<sf::Texture>();
        if (!t->loadFromFile(absolutePath.toStdString())) {
            return nullptr;
        }
        auto res = std::make_shared<GraphicResource>(defaultResource);
        res->texture = t;
        cache.insert(filename, res);

        QFile manifestFile(absolutePath + ".json");

        if (!manifestFile.exists() || !manifestFile.open(QIODevice::ReadOnly)) {
            return res;
        }

        QJsonDocument manifest;
        QJsonParseError err;

        manifest = QJsonDocument::fromJson(manifestFile.readAll(), &err);
        if (err.error != QJsonParseError::NoError || !manifest.isObject()) {
            return res;
        }

        QJsonObject manifestRoot = manifest.object();

        if (manifestRoot.contains("coldspot") && manifestRoot.value("coldspot").isArray()) {
            QJsonArray array = manifestRoot.value("coldspot").toArray();
            if (array.size() == 2 && array.at(0).isDouble() && array.at(1).isDouble()) {
                res->coldspot = { array.at(0).toInt(), array.at(1).toInt() };
                res->hasColdspot = true;
            }
        }

        if (manifestRoot.contains("gunspot") && manifestRoot.value("gunspot").isArray()) {
            QJsonArray array = manifestRoot.value("gunspot").toArray();
            if (array.size() == 2 && array.at(0).isDouble() && array.at(1).isDouble()) {
                res->gunspot = { array.at(0).toInt(), array.at(1).toInt() };
                res->hasGunspot = true;
            }
        }

        if (manifestRoot.contains("hotspot") && manifestRoot.value("hotspot").isArray()) {
            QJsonArray array = manifestRoot.value("hotspot").toArray();
            if (array.size() == 2 && array.at(0).isDouble() && array.at(1).isDouble()) {
                res->hotspot = { array.at(0).toInt(), array.at(1).toInt() };
            }
        }

        if (manifestRoot.contains("frameCount") && manifestRoot.value("frameCount").isDouble()) {
            res->frameCount = manifestRoot.value("frameCount").toInt();
        }

        if (manifestRoot.contains("framesPerRow") && manifestRoot.value("framesPerRow").isDouble()) {
            res->frameConfiguration.x = manifestRoot.value("framesPerRow").toInt();
        } else {
            res->frameConfiguration.x = res->frameCount;
        }

        if (manifestRoot.contains("framesPerCol") && manifestRoot.value("framesPerCol").isDouble()) {
            res->frameConfiguration.y = manifestRoot.value("framesPerCol").toInt();
        } else {
            res->frameConfiguration.y = 1;
        }

        if (manifestRoot.contains("width") && manifestRoot.value("width").isDouble()) {
            res->frameDimensions.x = manifestRoot.value("width").toInt();
        } else {
            res->frameDimensions.x = res->texture->getSize().x;
            res->frameConfiguration.x = 1;
        }

        if (manifestRoot.contains("height") && manifestRoot.value("height").isDouble()) {
            res->frameDimensions.y = manifestRoot.value("height").toInt();
        } else {
            res->frameDimensions.y = res->texture->getSize().y;
            res->frameConfiguration.y = 1;
        }

        res->frameCount = std::min(res->frameCount, (uint)(res->frameConfiguration.x * res->frameConfiguration.y));

        if (manifestRoot.contains("fps") && manifestRoot.value("fps").isDouble()) {
            int value = manifestRoot.value("fps").toInt();
            if (value > 0) {
                res->frameDuration = 1000 / value;
            }
        }

        res->frameOffset = 0;

        return res;
    }
    return cache.value(filename);
}

bool GraphicsCache::flush() {
    cache.clear();
    return true;
}

