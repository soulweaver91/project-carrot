#include "Tileset.h"
#include "../struct/Layers.h"
#include "../struct/Constants.h"
#include <exception>

Tileset::Tileset(const QString& tilesFilename, const QString& maskFilename) : isValid(false) {
    texture = std::make_shared<sf::Texture>();

    texture->loadFromFile(tilesFilename.toUtf8().data());

    if (texture->getSize().x == 0) {
        // Loading the tileset failed for some reason, texture is empty.
        throw std::runtime_error(QString("Tileset texture could not be loaded!").toStdString());
    }

    sf::Image maskfile;
    maskfile.loadFromFile(maskFilename.toUtf8().data());
    if (texture->getSize() != maskfile.getSize()) {
        // Mask could not be loaded or it didn't match the texture.
        // As a fallback, get mask from texture image itself.
        maskfile = texture->copyToImage();
    }

    uint width = texture->getSize().x / TILE_WIDTH;
    uint height = texture->getSize().y / TILE_HEIGHT;
    tilesPerRow = width;
    tileCount = width * height;

    // define colors to compare pixels against
    sf::Color white_alpha(255, 255, 255, 0);
    sf::Color white(255, 255, 255, 255);
    for (unsigned i = 0; i < height; i++) {
        for (unsigned j = 0; j < width; j++) {
            QBitArray tileMask(TILE_WIDTH * TILE_HEIGHT);
            bool maskEmpty = true;
            bool maskFilled = true;
            for (unsigned x = 0; x < TILE_WIDTH; ++x) {
                for (unsigned y = 0; y < TILE_HEIGHT; ++y) {
                    sf::Color px = maskfile.getPixel(j * TILE_WIDTH + x, i * TILE_HEIGHT + y);
                    // Consider any fully white or fully transparent pixel in the masks as non-solid and all others as solid
                    bool masked = ((px != white) && (px.a > 0));
                    tileMask.setBit(x + TILE_WIDTH * y, masked);
                    maskEmpty &= !masked;
                    maskFilled &= masked;
                }
            }
            masks << tileMask;
            isMaskEmpty << maskEmpty;
            isMaskFilled << maskFilled;

            auto sprite = std::make_shared<sf::Sprite>();
            sprite->setTexture(*(texture));
            sprite->setPosition(0, 0);
            sprite->setTextureRect(
                sf::IntRect(TILE_WIDTH * ((i * width + j) % tilesPerRow),
                            TILE_HEIGHT * ((i * width + j) / tilesPerRow),
                            TILE_WIDTH, TILE_HEIGHT));

            auto defaultLayerTile = std::make_shared<LayerTile>();
            defaultLayerTile->tilesetDefault = true;
            defaultLayerTile->isAnimated = false;
            defaultLayerTile->destructType = TileDestructType::DESTRUCT_NONE;
            defaultLayerTile->destructAnimation = 0;
            defaultLayerTile->extraByte = 0;
            defaultLayerTile->isFlippedX = false;
            defaultLayerTile->isFlippedY = false;
            defaultLayerTile->isOneWay = false;
            defaultLayerTile->destructFrameIndex = 0;
            defaultLayerTile->sprite = sprite;
            defaultLayerTile->texture = texture;
            defaultLayerTile->tileId = i * width + j;
            defaultLayerTile->suspendType = SuspendType::SUSPEND_NONE;

            defaultLayerTiles.append(defaultLayerTile);
        }
    }

    isValid = true;
}

bool Tileset::getIsValid() {
    return isValid;
}

const QBitArray& Tileset::getTileMask(const uint& tileID) {
    if (tileID < tileCount) {
        return masks.at(tileID);
    }

    return DEFAULT_MASK_BITARRAY;
}

std::shared_ptr<LayerTile> Tileset::getDefaultTile(const uint& tileID) {
    if (tileID < tileCount) {
        return defaultLayerTiles.at(tileID);
    }

    return defaultLayerTiles.at(0);
}

sf::IntRect Tileset::getTileTextureRect(const uint& tileID) {
    if (tileID < tileCount) {
        return sf::IntRect(TILE_WIDTH * (tileID % tilesPerRow),
                           TILE_HEIGHT * (tileID / tilesPerRow),
                           TILE_WIDTH, TILE_HEIGHT);
    }

    return sf::IntRect(0, 0, TILE_WIDTH, TILE_HEIGHT);
}

bool Tileset::isTileMaskEmpty(const uint& tileID) {
    if (tileID < tileCount) {
        return isMaskEmpty.at(tileID);
    }

    return true;
}

bool Tileset::isTileMaskFilled(const uint& tileID) {
    if (tileID < tileCount) {
        return isMaskFilled.at(tileID);
    }

    return false;
}

std::shared_ptr<sf::Texture> Tileset::getTexture() {
    return texture;
}

uint Tileset::getSize() {
    return tileCount;
}

QString Tileset::getName() {
    return name;
}

const QBitArray Tileset::DEFAULT_MASK_BITARRAY = QBitArray(TILE_WIDTH * TILE_HEIGHT);