#pragma once

#include <memory>
#include <QSet>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QVector>
#include <QBitArray>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <bass.h>
#include "AnimState.h"
#include "Hitbox.h"

struct GraphicResource {
    QSet<AnimStateT> state;
    std::shared_ptr<sf::Texture> texture;
    sf::Vector2i frameDimensions;
    sf::Vector2i frameConfiguration;
    uint frameDuration;
    uint frameCount;
    uint frameOffset;
    sf::Vector2i hotspot;
    sf::Vector2i coldspot;
    sf::Vector2i gunspot;
    bool hasColdspot;
    bool hasGunspot;
    QVector<std::shared_ptr<QBitArray>> bitmasks;
};

struct SoundResource {
    HSAMPLE sound;
};

struct ResourceSet {
    QString identifier;
    QMap<QString, std::shared_ptr<GraphicResource>> graphics;
    QMultiMap<QString, SoundResource> sounds;
};

struct ActorGraphicState {
    std::shared_ptr<QBitArray> mask;
    sf::Vector2i hotspot;
    sf::Vector2i dimensions;
    sf::FloatRect boundingBox;
    sf::Vector2f scale;
    sf::Vector2f origin;
    float angle;
};
