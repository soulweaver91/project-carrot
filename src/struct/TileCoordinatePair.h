#pragma once
#include <SFML/System.hpp>
#include <QHashFunctions>

typedef sf::Vector2i TileCoordinatePair;

namespace sf {
    template<typename T>
    inline uint qHash(const sf::Vector2<T>& p) {
        return ::qHash(p.x + p.y);
    }
}
