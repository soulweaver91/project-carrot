#pragma once

#include <memory>
#include <QString>
#include <QBitArray>
#include <QVector>
#include <SFML/Graphics.hpp>

struct LayerTile;

class Tileset {
public:
    Tileset(const QString& tilesFilename, const QString& maskFilename);
    bool getIsValid();
    const QBitArray& getTileMask(const uint& tileID);
    std::shared_ptr<LayerTile> getDefaultTile(const uint& tileID);
    sf::IntRect getTileTextureRect(const uint& tileID);
    bool isTileMaskEmpty(const uint& tileID);
    bool isTileMaskFilled(const uint& tileID);
    std::shared_ptr<sf::Texture> getTexture();
    uint getSize();
    QString getName();

private:
    bool isValid;
    QString name;
    uint tileCount;
    uint tilesPerRow;
    std::shared_ptr<sf::Texture> texture;
    QVector<QBitArray> masks;
    QVector<std::shared_ptr<LayerTile>> defaultLayerTiles;
    
    // to speed up collision checking so that not every tile needs to be pixel perfect checked
    QVector<bool> isMaskEmpty;
    QVector<bool> isMaskFilled;

    static const QBitArray DEFAULT_MASK_BITARRAY;
};
