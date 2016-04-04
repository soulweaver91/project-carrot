#pragma once

#include <memory>
#include <QSet>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <bass.h>
#include "AnimState.h"

struct GraphicResource {
    QSet<AnimStateT> state;
    std::shared_ptr<sf::Texture> texture;
    sf::Vector2i frameDimensions;
    uint frameDuration;
    uint frameCount;
    uint frameOffset;
    sf::Vector2i hotspot;
    sf::Vector2i coldspot;
    sf::Vector2i gunspot;
    bool hasColdspot;
    bool hasGunspot;
};

struct SoundResource {
    HSAMPLE sound;
};

struct ResourceSet {
    QString identifier;
    QMap<QString, std::shared_ptr<GraphicResource>> graphics;
    QMultiMap<QString, SoundResource> sounds;
};