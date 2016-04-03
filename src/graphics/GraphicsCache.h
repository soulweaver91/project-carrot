#pragma once

#include <memory>
#include <QMap>
#include <SFML/Graphics.hpp>

class GraphicsCache {
public:
    GraphicsCache();
    ~GraphicsCache();
    std::shared_ptr<sf::Texture> request(const QString& filename);
    bool flush();
private:
    bool loadGraphics(const QString& filename);
    QMap<QString, std::shared_ptr<sf::Texture>> cache;
};
