#include "TileMap.h"

#include <cmath>
#include <exception>
#include "EventMap.h"
#include "GameView.h"
#include "LevelManager.h"
#include "ActorAPI.h"
#include "../graphics/AnimatedTile.h"
#include "../sound/SoundSystem.h"
#include "../struct/Constants.h"
#include "../struct/DebugConfig.h"
#include "../actor/FrozenBlock.h"
#include "../graphics/ShaderSource.h"

TileMap::TileMap(LevelManager* root, const QString& tilesetFilename, 
    const QString& maskFilename, const QString& sprLayerFilename) 
    : root(root), sprLayerIdx(0), levelWidth(1), levelHeight(1) {
    // Reserve textures for tileset and its mask counterpart
    levelTileset = std::make_unique<Tileset>(tilesetFilename, maskFilename);
    if (!levelTileset->getIsValid()) {
        throw std::runtime_error(QString("Unknown error loading the tileset!").toStdString());
    }

    // initialize the trigger store
    for (int i = 0; i < 256; ++i) {
        triggerState[i] = false;
    }

    texturedBackgroundTexture = std::make_unique<sf::RenderTexture>();
    texturedBackgroundTexture->create(256, 256);
    texturedBackgroundTexture->setRepeated(true);
    texturedBackgroundTexture->setSmooth(true);
    texturedBackgroundSprite = std::make_unique<sf::Sprite>(texturedBackgroundTexture->getTexture());
    
    // The sprite layer has no settings to apply to it, so just pass an empty set instead.
    QSettings dummySettings;
    readLayerConfiguration(LayerType::LAYER_SPRITE_LAYER, sprLayerFilename, dummySettings);
    levelHeight = levelLayout.at(0).tileLayout.size();
    levelWidth = levelLayout.at(0).tileLayout.at(0).size();

    sceneryResources = root->getActorAPI()->loadActorTypeResources("Common/Scenery");
}

TileMap::~TileMap() {

}

void TileMap::drawLowerLevels(std::shared_ptr<GameView>& view) {
    // lower levels is everything below the sprite layer and the sprite layer itself
    for (TileMapLayer& layer : levelLayout) {
        if (layer.type == LAYER_FOREGROUND_LAYER) { continue; }
        drawLayer(layer, view);
    }
}

void TileMap::drawHigherLevels(std::shared_ptr<GameView>& view) {
    // higher levels is only the foreground layers
    for (TileMapLayer& layer : levelLayout) {
        if (layer.type != LAYER_FOREGROUND_LAYER) { continue; }
        drawLayer(layer, view);
    }
}

void TileMap::initializeBackgroundTexture(TileMapLayer& background) {
    // basically just draw all tiles onto the background texture once
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = background.tileLayout.at(j).at(i)->tileId;
            bool ani = background.tileLayout.at(j).at(i)->isAnimated;

            std::shared_ptr<sf::Sprite> sprite = nullptr;
            if (ani) {
                if (idx < animatedTiles.size()) {
                    auto anim = animatedTiles.at(idx);
                    sprite = anim->getCurrentTile()->sprite;
                }
            } else {
                sprite = background.tileLayout[j][i]->sprite;
            }
                
            sprite->setPosition(i * 32, j * 32);
            texturedBackgroundTexture->draw(*sprite);
        }
    }
}

void TileMap::drawTexturedBackground(TileMapLayer& layer, const double& x, const double& y, std::shared_ptr<GameView>& view) {
    auto target = view->getCanvas().lock();
    if (target == nullptr) {
        return;
    }

    // update animated tiles on the cache texture
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = layer.tileLayout.at(j).at(i)->tileId;
            bool animated = layer.tileLayout.at(j).at(i)->isAnimated;

            if (animated) {
                if (idx < animatedTiles.size()) {
                    auto anim = animatedTiles.at(idx);
                    auto sprite = anim->getCurrentTile()->sprite;
                    sprite->setPosition(i * 32,j * 32);
                    texturedBackgroundTexture->draw(*sprite);
                }
            }
        }
    }

    texturedBackgroundTexture->display();

    sf::RenderStates states;
    auto shader = ShaderSource::getShader("TexturedBackgroundShader");
    shader->setParameter("horizonColor", texturedBackgroundColor);
    shader->setParameter("shift", sf::Vector2f(x, y));
    shader->setParameter("canvasDimensions", sf::Vector2f(view->getViewWidth(), view->getViewHeight()));
    states.shader = shader.get();

    texturedBackgroundSprite->setPosition(view->getViewCenter().x - view->getViewWidth() / 2, view->getViewCenter().y - view->getViewHeight() / 2);
    target->draw(*texturedBackgroundSprite, states);
    return;
}

std::shared_ptr<LayerTile> TileMap::cloneDefaultLayerTile(const TileCoordinatePair& tilePos) {
    auto tile = levelLayout[sprLayerIdx].tileLayout[tilePos.y][tilePos.x];
    if (tile->tilesetDefault) {
        tile = std::make_shared<LayerTile>(*tile);
        tile->tilesetDefault = false;
        levelLayout[sprLayerIdx].tileLayout[tilePos.y][tilePos.x] = tile;
    }
    return tile;
}

double TileMap::translateCoordinate(const double& coordinate, const double& speed, const double& offset, const bool& isY,
    const int& viewHeight, const int& viewWidth) const {
    // Coordinate: the "vanilla" coordinate of the tile on the layer if the layer was fixed to the sprite layer with same
    // speed and no other options. Think of its position in JCS.
    // Speed: the set layer speed; 1 for anything that moves the same speed as the sprite layer (where the objects live),
    // less than 1 for backgrounds that move slower, more than 1 for foregrounds that move faster
    // Offset: any difference to starting coordinates caused by an inherent automatic speed a layer has

    // Literal 70 is the same as in drawLayer, it's the offscreen offset of the first tile to draw.
    // Don't touch unless absolutely necessary.
    return ((coordinate * speed + offset + (70 + (isY ? (viewHeight - 200) : (viewWidth - 320)) / 2) * (speed - 1)));
}

void TileMap::drawLayer(TileMapLayer& layer, std::shared_ptr<GameView>& view) {
    auto target = view->getCanvas().lock();
    if (target == nullptr) {
        return;
    }

#ifdef CARROT_DEBUG
    auto debugConfig = root->getActorAPI()->getDebugConfig();

    if (debugConfig->dbgShowMasked && layer.type != LAYER_SPRITE_LAYER) {
        // only draw sprite layer in collision debug mode
        return;
    }
#endif

    // Layer dimensions
    int layerHeight = layer.tileLayout.size();
    int layerWidth = layer.tileLayout.at(0).size();
    
    // Update offsets for moving layers
    if (layer.autoSpeedX > EPSILON) {
        layer.offsetX += layer.autoSpeedX * 2;
        while (layer.repeatX && (std::abs(layer.offsetX) > (layerWidth * TILE_WIDTH))) {
            layer.offsetX -= (layerWidth * TILE_WIDTH);
        }
    }
    if (layer.autoSpeedY > EPSILON) {
        layer.offsetY += layer.autoSpeedY * 2;
        while (layer.repeatY && (std::abs(layer.offsetY) > (layerHeight * TILE_HEIGHT))) {
            layer.offsetY -= (layerHeight * TILE_HEIGHT);
        }
    }

    // Get current layer offsets and speeds
    double layerOffsetX = layer.offsetX;
    double layerOffsetY = layer.offsetY - (layer.useInherentOffset ? (view->getViewHeight() - 200) / 2 : 0);
    double layerSpeedX = layer.speedX;
    double layerSpeedY = layer.speedY;
    
    // Find out coordinates for a tile from outside the boundaries from topleft corner of the screen 
    auto viewCenter = view->getViewCenter();
    double xStart = viewCenter.x - 70.0 - (view->getViewWidth() / 2);
    double yStart = viewCenter.y - 70.0 - (view->getViewHeight() / 2);

    // Get view dimensions
    uint viewHeight = view->getViewHeight();
    uint viewWidth = view->getViewWidth();
    
    // Figure out the floating point offset from the calculated coordinates and the actual tile
    // corner coordinates
    double remainderX = fmod(translateCoordinate(xStart, layerSpeedX, layerOffsetX, false, viewHeight, viewWidth), TILE_WIDTH);
    double remainderY = fmod(translateCoordinate(yStart, layerSpeedY, layerOffsetY, true, viewHeight, viewWidth), TILE_HEIGHT);
    
    // Calculate the index (on the layer map) of the first tile that needs to be drawn to the
    // position determined earlier
    int tileX = 0;
    int tileY = 0;

    // Determine the actual drawing location on screen
    double firstTileXIntermediate = translateCoordinate(xStart, layerSpeedX, layerOffsetX, false, viewHeight, viewWidth) / TILE_WIDTH;
    double firstTileYIntermediate = translateCoordinate(yStart, layerSpeedY, layerOffsetY, true, viewHeight, viewWidth) / TILE_HEIGHT;

    int absoluteFirstTileX = 0;
    int absoluteFirstTileY = 0;

    // Get the actual tile coords on the layer layout
    if (firstTileXIntermediate > 0) {
        tileX = static_cast<int>(std::floor(firstTileXIntermediate)) % layerWidth;
        absoluteFirstTileX = static_cast<int>(std::floor(firstTileXIntermediate));
    } else {
        tileX = static_cast<int>(std::ceil(firstTileXIntermediate)) % layerWidth;
        absoluteFirstTileX = static_cast<int>(std::ceil(firstTileXIntermediate));
        while (tileX < 0) {
            tileX += layerWidth;
        }
    }
    
    if (firstTileYIntermediate > 0) {
        tileY = static_cast<int>(std::floor(firstTileYIntermediate)) % layerHeight;
        absoluteFirstTileY = static_cast<int>(std::floor(firstTileYIntermediate));
    } else {
        tileY = static_cast<int>(std::ceil(firstTileYIntermediate)) % layerHeight;
        absoluteFirstTileY = static_cast<int>(std::ceil(firstTileYIntermediate));
        while (tileY < 0) {
            tileY += layerHeight;
        }
    }

    // Save the tile Y at the left border so that we can roll back to it at the start of
    // every row iteration
    unsigned tileYStart = tileY;

    // update xStart and yStart with the remainder so that we start at the tile boundary
    // minus 1 because indices are updated in the beginning of the loops
    xStart -= remainderX - TILE_WIDTH;
    yStart -= remainderY - TILE_HEIGHT;
    
    // Calculate the last coordinates we want to draw to
    double xEnd = xStart + 100 + view->getViewWidth();
    double yEnd = yStart + 100 + view->getViewHeight();

    if (layer.isTextured && (layerHeight == 8) && (layerWidth == 8)) {
#ifdef CARROT_DEBUG
        float perspectiveSpeed = debugConfig->tempModifier[3] / 10.0;
#else
        float perspectiveSpeed = 0.4;
#endif
        drawTexturedBackground(layer,
                fmod((xStart + remainderX) * perspectiveSpeed + layerOffsetX, 256.0),
                fmod((yStart + remainderY) * perspectiveSpeed + layerOffsetY, 256.0), view);
    } else {
        int tileCounterX = -1;
        for (double x = xStart; x < xEnd; x += TILE_WIDTH) {
            tileX = (tileX + 1) % layerWidth;
            tileCounterX++;
            if (!layer.repeatX) {
                // If the current tile isn't in the first iteration of the layer horizontally, don't draw this column
                if (absoluteFirstTileX + tileCounterX + 1 < 0 || absoluteFirstTileX + tileCounterX + 1 >= layerWidth) {
                    continue;
                }
            }
            tileY = tileYStart;
            int tileCounterY = -1;
            for (double y = yStart; y < yEnd; y += TILE_HEIGHT) {
                tileY = (tileY + 1) % layerHeight;
                tileCounterY++;

                auto tile = layer.tileLayout.at(tileY).at(tileX);

                int idx = tile->tileId;
                bool ani = tile->isAnimated;

                if (!layer.repeatY) {
                    // If the current tile isn't in the first iteration of the layer vertically, don't draw it
                    if (absoluteFirstTileY + tileCounterY + 1 < 0 || absoluteFirstTileY + tileCounterY + 1 >= layerHeight) {
                        continue;
                    }
                }

                if (idx == 0 && !ani) { continue; }

#ifdef CARROT_DEBUG
                if (debugConfig->dbgShowMasked) {
                    // debug code for masks
                    sf::RectangleShape b(sf::Vector2f(TILE_WIDTH, TILE_HEIGHT));
                    b.setPosition(x, y);
                    if (ani) {
                        idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
                    }
                    if (levelTileset->isTileMaskFilled(idx)) {
                        b.setFillColor(sf::Color::White);
                        target->draw(b);
                    } else {
                        if (!levelTileset->isTileMaskEmpty(idx)) {
                            b.setFillColor(sf::Color(255, 255, 255, 128));
                            target->draw(b);
                        }
                    }
                    if (ani || tile->destructType != TileDestructType::DESTRUCT_NONE) {
                        b.setFillColor(sf::Color::Transparent);
                        b.setOutlineThickness(-3);
                        b.setOutlineColor(sf::Color(255, 0, 128, 255));
                        target->draw(b);
                    }
                    continue;
                }
#endif

                std::shared_ptr<sf::Sprite> sprite = nullptr;
                if (ani) {
                    if (idx < animatedTiles.size()) {
                        auto anim = animatedTiles.at(idx);
                        sprite = anim->getCurrentTile()->sprite;
                        sprite->setScale(
                            tile->isFlippedX ? -1.0 : 1.0,
                            tile->isFlippedY ? -1.0 : 1.0
                        );
                        sprite->setOrigin(
                            tile->isFlippedX ? TILE_WIDTH : 0.0,
                            tile->isFlippedY ? TILE_HEIGHT : 0.0
                        );
                    }
                } else {
                    sprite = tile->sprite;
                }

                // rounding to nearest integer is necessary because otherwise there will be tearing
                if (sprite != nullptr) {
                    sprite->setPosition((float)qRound(x), (float)qRound(y));
                    target->draw(*sprite);
                }
            }
        }
    }
}

void TileMap::readLayerConfiguration(enum LayerType type, const QString& filename, QSettings& config, unsigned layerIdx) {
    // Build a new layer
    TileMapLayer newLayer;

    // Layer index and type are given as parameters
    newLayer.idx = layerIdx;
    newLayer.type = type;

    // Open the layer file
    QFile layerHandle(filename);
    if (layerHandle.open(QIODevice::ReadOnly)) {
        // Uncompress the compressed data
        QByteArray layerData = qUncompress(layerHandle.readAll());

        if (layerData.size() > 0) {
            // Create a data stream of the data
            QDataStream layerStream(layerData);

            int row = 0;
            // Read data until no more data available
            while (!layerStream.atEnd()) {
                QVector<std::shared_ptr<LayerTile>> newTileRow;
                int col = 0;

                while (!layerStream.atEnd()) {
                    std::shared_ptr<LayerTile> tile;

                    // Read type short from the stream
                    quint16 type;
                    layerStream >> type;
                    if (type == 0xFFFF) {
                        // 0xFFFF means a line break
                        break;
                    }
                    // Otherwise it was a tile; read tile flags next
                    quint8 flags;
                    layerStream >> flags;

                    if (flags == 0) {
                        tile = levelTileset->getDefaultTile(type);
                        newTileRow << tile;
                        col++;
                        continue;
                    }

                    bool isFlippedX = (flags & 0x01) > 0;
                    bool isFlippedY = (flags & 0x02) > 0;
                    bool isAnimated = (flags & 0x04) > 0;
                    bool legacyTranslucent = (flags & 0x80) > 0;

                    // Invalid tile numbers (higher than tileset tile amount) are silently changed to empty tiles
                    if (type > levelTileset->getSize() && !isAnimated) {
                        type = 0;
                    }

                    // Copy the default tile and do stuff with it
                    if (!isAnimated) {
                        tile = std::make_shared<LayerTile>(*levelTileset->getDefaultTile(type));
                    } else {
                        // Copy the template for isAnimated tiles from the first tile, then fix the tile ID.
                        // Cannot rely on copying the same tile as its own isAnimated tile ID, because it is
                        // possible that there are more isAnimated tiles than regular ones.
                        tile = std::make_shared<LayerTile>(*levelTileset->getDefaultTile(0));
                        tile->tileId = type;
                    }

                    tile->isFlippedX = isFlippedX;
                    tile->isFlippedY = isFlippedY;
                    tile->isAnimated = isAnimated;

                    if (tile->isFlippedX || tile->isAnimated || legacyTranslucent) {
                        tile->sprite = std::make_shared<sf::Sprite>(*tile->sprite);
                    }

                    // Set tile texture according to the given tile number, to the tile coordinates
                    if (tile->isFlippedX) {
                        tile->sprite = std::make_shared<sf::Sprite>(*tile->sprite);
                        tile->sprite->setScale(-1.0, 1.0);
                        tile->sprite->setOrigin(TILE_WIDTH, 0.0);
                    }

                    if (legacyTranslucent) {
                        tile->sprite->setColor(sf::Color(255, 255, 255, 127));
                    }

                    newTileRow.push_back(tile);
                    col++;
                }
                newLayer.tileLayout.push_back(newTileRow);
                row++;
            }
            
            config.beginGroup(filename.section('/', -1));
            newLayer.speedX = config.value("XSpeed", 1.0).toDouble();
            newLayer.speedY = config.value("YSpeed", 1.0).toDouble();
            newLayer.repeatX = config.value("XRepeat", false).toBool();
            newLayer.repeatY = config.value("YRepeat", false).toBool();
            newLayer.autoSpeedX = config.value("XAutoSpeed", 0.0).toDouble();
            newLayer.autoSpeedY = config.value("YAutoSpeed", 0.0).toDouble();
            newLayer.useInherentOffset = config.value("InherentOffset", false).toBool();
            newLayer.isTextured = config.value("TexturedModeEnabled", false).toBool();
            newLayer.useStarsTextured = config.value("ParallaxStarsEnabled", false).toBool();
            QString color = config.value("TexturedModeColor", "#000000").toString();
            if (color.length() == 7) {
                bool foo; // we won't care about invalid data too much here, unrecognized input will be converted to 0
                newLayer.texturedBackgroundColor = sf::Color(
                    color.mid(1, 2).toInt(&foo, 16),
                    color.mid(3, 2).toInt(&foo, 16),
                    color.mid(5, 2).toInt(&foo, 16)
                );
            } else {
                newLayer.texturedBackgroundColor = sf::Color::Black;
            }
            config.endGroup();

            newLayer.offsetX = 0.0;
            newLayer.offsetY = 0.0;

            levelLayout << newLayer;

            // Sort the layers by their layer type and number
            std::sort(levelLayout.begin(),levelLayout.end());
            updateSprLayerIdx();

            if (type == LAYER_SKY_LAYER && newLayer.isTextured) {
                texturedBackgroundColor = newLayer.texturedBackgroundColor;
                initializeBackgroundTexture(newLayer);
            }
        } else {
            // TODO: uncompress fail, what do?
        }
    } else {
        // TODO: could not open the file, do something here
    }
}

bool TileMap::isTileEmpty(const TileCoordinatePair& tilePos) const {
    // Consider out-of-level coordinates as solid walls
    if (tilePos.x >= static_cast<int>(levelWidth) || tilePos.y >= static_cast<int>(levelHeight)) {
        return false;
    }

    int idx = levelLayout.at(sprLayerIdx).tileLayout.at(tilePos.y).at(tilePos.x)->tileId;
    if (levelLayout.at(sprLayerIdx).tileLayout.at(tilePos.y).at(tilePos.x)->isAnimated) {
        idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
    }

    if (levelTileset->isTileMaskEmpty(idx)) {
        return true;
    } else {
        // TODO: Pixel perfect collision wanted here? probably, so do that later
        return false;
    }
}

bool TileMap::isTileEmpty(const Hitbox& hitbox, bool downwards) const {
    // Consider out-of-level coordinates as solid walls
    if ((hitbox.right >= levelWidth * TILE_WIDTH) || (hitbox.bottom >= levelHeight * TILE_HEIGHT)
     || (hitbox.left <= 0) || (hitbox.top <= 0)) {
        return false;
    }

    // check all covered tiles for collisions; if all are empty, no need to do pixel level collision checking
    bool all_empty = true;
    int hx1 = floor(hitbox.left);
    int hx2 = std::min(ceil(hitbox.right), levelWidth * TILE_WIDTH - 1.0);
    int hy1 = floor(hitbox.top);
    int hy2 = std::min(ceil(hitbox.bottom), levelHeight * TILE_HEIGHT - 1.0);

    const auto& sprLayerLayout = levelLayout.at(sprLayerIdx).tileLayout;

    for (int x = hx1 / TILE_WIDTH; x <= hx2 / TILE_WIDTH; ++x) {
        for (int y = hy1 / TILE_HEIGHT; y <= hy2 / TILE_HEIGHT; ++y) {
            auto tile = sprLayerLayout.at(y).at(x);
            int idx = tile->tileId;
            if (tile->isAnimated) {
                idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
            }

            if (!levelTileset->isTileMaskEmpty(idx) &&
                !(tile->isOneWay && !downwards) &&
                !(tile->suspendType != SuspendType::SUSPEND_NONE)) {
                all_empty = false;
                break;
            }
        }
        if (!all_empty) {
            break;
        }
    }

    if (all_empty) {
        return true;
    }

    // check each tile pixel perfectly for collisions
    for (int x = hx1 / TILE_WIDTH; x <= hx2 / TILE_WIDTH; ++x) {
        for (int y = hy1 / TILE_HEIGHT; y <= hy2 / TILE_HEIGHT; ++y) {
            auto tile = sprLayerLayout.at(y).at(x);
            bool fx = tile->isFlippedX;
            // bool fy = sprLayerLayout.at(y).at(x)->isFlippedY;
            int idx = tile->tileId;
            if (tile->isAnimated) {
                idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
            }

            if ((tile->isOneWay && !downwards && (hy2 < ((y + 1) * TILE_HEIGHT)))
                || tile->suspendType != SuspendType::SUSPEND_NONE) {
                continue;
            }
            auto& mask = levelTileset->getTileMask(idx);
            int px_idx;
            for (int i = 0; i < TILE_WIDTH * TILE_HEIGHT; ++i) {
                int nowx = (TILE_WIDTH  * x + i % TILE_WIDTH);
                int nowy = (TILE_HEIGHT * y + i / TILE_WIDTH);
                if (hx2 < nowx || hx1 >= nowx) {
                    continue;
                }
                if (hy2 < nowy || hy1 >= nowy) {
                    continue;
                }
                px_idx = i;
                if (fx) { px_idx =      (i / TILE_WIDTH)  * TILE_WIDTH + ((TILE_WIDTH - 1) - (i % TILE_WIDTH)); }
                //if (fy) { px_idx = (31 -(i / 32)) * 32 +        i % 32  ; }
                // TODO: fix so that both flags work simultaneously
                if (mask[px_idx]) {
                    return false;
                }
            }
        }
    }
    return true;
}

void TileMap::resizeTexturedBackgroundSprite(int width, int height) {
    texturedBackgroundSprite->setTextureRect(sf::IntRect(0, 0, width, height));
}

void TileMap::updateSprLayerIdx() {
    for (int layer = 0; layer < levelLayout.size(); ++layer) {
        if (!(levelLayout.at(layer).type == LAYER_SPRITE_LAYER)) {
            continue;
        }
        sprLayerIdx = layer;
        return;
    }
}

void TileMap::readAnimatedTiles(const QString& filename) {
    // Open the given file
    QFile animHandle(filename);
    if (animHandle.open(QIODevice::ReadOnly)) {
        // Uncompress the compressed data
        QByteArray animData = qUncompress(animHandle.readAll());

        if (animData.size() > 0) {
            // Create a data stream of the data
            QDataStream animStream(animData);

            // Read data until no more data available
            while (!animStream.atEnd()) {
                QVector<quint16> frames;
                QVector<quint8> flags;
                // Read type short from the stream
                for (int i = 0; i < 64; ++i) {
                    quint16 tile;
                    quint8 tileFlags;
                    animStream >> tile >> tileFlags;
                    if (tile != 0xFFFF) {
                        frames << tile;
                        flags << tileFlags;
                    }
                }

                quint16 delay, delayJitter, pingPongDelay;
                quint8 speed, pingPong;
                animStream >> speed >> delay >> delayJitter >> pingPong >> pingPongDelay;

                if (frames.size() > 0) {
                    auto ani = std::make_shared<AnimatedTile>(getTilesetTexture(), frames, flags, speed,
                        delay, delayJitter, (pingPong > 0), pingPongDelay);
                    animatedTiles << ani;
                }
            }
            
        } else {
            // TODO: uncompress fail, what do?
        }
    } else {
        // TODO: could not open the file, do something here
    }
}

const std::shared_ptr<sf::Texture> TileMap::getTilesetTexture() const {
    return levelTileset->getTexture();
}


QVector<QVector<std::shared_ptr<LayerTile>>> TileMap::prepareSavePointLayer() {
    /*
    TODO: make use of this improved algorithm as an option

    std::vector< std::vector< LayerTile > > layer = level_layout.at(sprLayerIdx).tileLayout;
    for (int i = 0; i < layer.size(); ++i) {
        for (int j = 0; j < layer.at(i).size(); ++j) {
            layer.at(i).at(j).isEventActive = false;
        }
    }
    return layer;*/
    return spriteLayerAtLevelStart;
}

void TileMap::loadSavePointLayer(const QVector<QVector<std::shared_ptr<LayerTile>>>& layer) {
    levelLayout[sprLayerIdx].tileLayout = layer;
}

bool TileMap::checkWeaponDestructible(const CoordinatePair& pos, WeaponType weapon) {
    int tx = pos.tileX();
    int ty = pos.tileY();

    if (ty >= static_cast<int>(levelHeight) || tx >= static_cast<int>(levelWidth) || tx < 0 || ty < 0) {
        return false;
    }
    auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
    if (tile->destructType == DESTRUCT_WEAPON) {
        if (weapon == WEAPON_FREEZER && (animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex) {
            auto e = std::make_shared<FrozenBlock>(ActorInstantiationDetails(root->getActorAPI(), { TILE_WIDTH * (tx + 0.5), TILE_HEIGHT * (ty + 0.5) }));
            root->addActor(e);

            return true;
        }

        if (tile->extraByte == 0u || tile->extraByte == static_cast<uint>(weapon + 1)) {
            return advanceDestructibleTileAnimation(tile, pos.tilePosition(), "COMMON_SCENERY_DESTRUCT");
        }
    }

    return false;
}

uint TileMap::checkSpecialDestructible(const Hitbox& hitbox) {
    int x1 = std::max(0.0, hitbox.left / TILE_WIDTH);
    int x2 = std::min((uint)(hitbox.right / TILE_WIDTH), levelWidth - 1);
    int y1 = std::max(0.0, hitbox.top / TILE_HEIGHT);
    int y2 = std::min((uint)(hitbox.bottom / TILE_HEIGHT), levelHeight - 1);
    int hit = 0;
    for (int tx = x1; tx <= x2; ++tx) {
        for (int ty = y1; ty <= y2; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_SPECIAL) {
                if (advanceDestructibleTileAnimation(tile, TileCoordinatePair(tx, ty), "COMMON_SCENERY_DESTRUCT")) {
                    hit++;
                }
            }
        }
    }
    return hit;
}

uint TileMap::checkSpecialSpeedDestructible(const Hitbox& hitbox, const double& speed) {
    int x1 = std::max(0.0, hitbox.left / TILE_WIDTH);
    int x2 = std::min((uint)(hitbox.right / TILE_WIDTH), levelWidth - 1);
    int y1 = std::max(0.0, hitbox.top / TILE_HEIGHT);
    int y2 = std::min((uint)(hitbox.bottom / TILE_HEIGHT), levelHeight - 1);
    uint hit = 0;
    for (int tx = x1; tx <= x2; ++tx) {
        for (int ty = y1; ty <= y2; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_SPEED && tile->extraByte + 3 <= speed) {
                if (advanceDestructibleTileAnimation(tile, TileCoordinatePair(tx, ty), "COMMON_SCENERY_DESTRUCT")) {
                    hit++;
                }
            }
        }
    }
    return hit;
}

uint TileMap::checkCollapseDestructible(const Hitbox& hitbox) {
    int x1 = std::max(0.0, hitbox.left / TILE_WIDTH);
    int x2 = std::min((uint)(hitbox.right / TILE_WIDTH), levelWidth - 1);
    int y1 = std::max(0.0, hitbox.top / TILE_HEIGHT);
    int y2 = std::min((uint)(hitbox.bottom / TILE_HEIGHT), levelHeight - 1);
    uint hit = 0;
    for (int tx = x1; tx <= x2; ++tx) {
        for (int ty = y1; ty <= y2; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_COLLAPSE && !activeCollapsingTiles.contains({ tx, ty })) {
                activeCollapsingTiles.insert({ tx, ty });
                hit++;
            }
        }
    }
    return hit;
}

void TileMap::saveInitialSpriteLayer() {
    spriteLayerAtLevelStart = levelLayout.at(sprLayerIdx).tileLayout;
}

void TileMap::setTrigger(unsigned char triggerID, bool newState) {
    if (triggerState[triggerID] == newState) {
        return;
    }

    triggerState[triggerID] = newState;

    // go through all tiles and update any that are influenced by this trigger
    for (uint tx = 0; tx < levelWidth; ++tx) {
        for (uint ty = 0; ty < levelHeight; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_TRIGGER && tile->extraByte == triggerID) {
                if (animatedTiles.at(tile->destructAnimation)->getAnimationLength() > 1) {
                    tile->destructFrameIndex = (newState ? 1u : 0u);
                    tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
                    tile->sprite->setTextureRect(levelTileset->getTileTextureRect(tile->tileId));
                }
            }
        }
    }
}

bool TileMap::getTrigger(unsigned char triggerID) const {
    return triggerState[triggerID];
}

unsigned TileMap::getLevelWidth() const {
    return levelWidth;
}
unsigned TileMap::getLevelHeight() const {
    return levelHeight;
}

void TileMap::setTileEventFlag(const EventMap* events, const TileCoordinatePair& tilePos, PCEvent e) {
    auto tile = levelLayout[sprLayerIdx].tileLayout[tilePos.y][tilePos.x];
    quint16 p[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    if (events != nullptr) {
        events->getTileParams(tilePos, p);
    } else {
        auto mainEvents = root->getGameEvents().lock();
        if (mainEvents != nullptr) {
            mainEvents->getTileParams(tilePos, p);
        }
    }

    switch (e) {
        case PC_MODIFIER_ONE_WAY:
            tile = cloneDefaultLayerTile(tilePos);
            tile->isOneWay = true;
            break;
        case PC_MODIFIER_VINE:
            tile = cloneDefaultLayerTile(tilePos);
            tile->suspendType = SuspendType::SUSPEND_VINE;
            break;
        case PC_MODIFIER_HOOK:
            tile = cloneDefaultLayerTile(tilePos);
            tile->suspendType = SuspendType::SUSPEND_HOOK;
            break;
        case PC_SCENERY_DESTRUCT:
            setTileDestructibleEventFlag(tile, tilePos, DESTRUCT_WEAPON, p[0]);
            break;
        case PC_SCENERY_BUTTSTOMP:
            setTileDestructibleEventFlag(tile, tilePos, DESTRUCT_SPECIAL, p[0]);
            break;
        case PC_TRIGGER_AREA:
            setTileDestructibleEventFlag(tile, tilePos, DESTRUCT_TRIGGER, p[0]);
            break;
        case PC_SCENERY_DESTRUCT_SPD:
            setTileDestructibleEventFlag(tile, tilePos, DESTRUCT_SPEED, p[0]);
            break;
        case PC_SCENERY_COLLAPSE:
            setTileDestructibleEventFlag(tile, tilePos, DESTRUCT_COLLAPSE, p[0] * 25);
            break;
        default:
            break;
    }
}

void TileMap::setTileDestructibleEventFlag(std::shared_ptr<LayerTile>& tile, const TileCoordinatePair& tilePos,
    const TileDestructType& type, const quint16& extraByte) {
    if (tile->isAnimated) {
        tile = cloneDefaultLayerTile(tilePos);
        tile->destructType = type;
        tile->isAnimated = false;
        tile->destructAnimation = tile->tileId;
        tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(0);
        tile->destructFrameIndex = 0;
        tile->sprite->setTextureRect(levelTileset->getTileTextureRect(tile->tileId));
        tile->extraByte = extraByte;
    }
}

bool TileMap::advanceDestructibleTileAnimation(std::shared_ptr<LayerTile>& tile, const TileCoordinatePair& tilePos, const QString& soundName) {
    if ((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex) {
        // tile not destroyed yet, advance counter by one
        tile->destructFrameIndex++;
        tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
        tile->sprite->setTextureRect(levelTileset->getTileTextureRect(tile->tileId));
        if (!((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex)) {
            // the tile was destroyed, create debris
            if (sceneryResources->sounds.contains(soundName)) {
                root->getActorAPI()->playSound(sceneryResources->sounds.value(soundName).sound, CoordinatePair({ TILE_WIDTH * (tilePos.x + 0.5), TILE_HEIGHT * (tilePos.y + 0.5) }));
            }
            root->createDebris(animatedTiles.at(tile->destructAnimation)
                ->getFrameCanonicalIndex(animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 1),
                tilePos);
        }
        return true;
    }
    return false;
}

SuspendType TileMap::getPositionSuspendType(const CoordinatePair& pos) const {
    int ax = pos.tileX();
    int ay = pos.tileY();
    int rx = static_cast<int>(pos.x) - ax * TILE_WIDTH;
    int ry = static_cast<int>(pos.y) - ay * TILE_HEIGHT;

    auto tile = levelLayout[sprLayerIdx].tileLayout[ay][ax];

    bool fx = tile->isFlippedX;
    //bool fy = tile->isFlippedY;
    if (tile->suspendType == SuspendType::SUSPEND_NONE) {
        return SuspendType::SUSPEND_NONE;
    }

    QBitArray mask(TILE_WIDTH * TILE_HEIGHT, false);
    if (tile->isAnimated) {
        if (tile->tileId < static_cast<uint>(animatedTiles.size())) {
            mask = levelTileset->getTileMask(animatedTiles.at(tile->tileId)->getCurrentTile()->tileId);
        }
    } else {
        mask = levelTileset->getTileMask(tile->tileId);
    }
    for (int ty = ry - 4; ty < ry + 4; ty++) {
        if (ty < 0 || ty >= TILE_HEIGHT) {
            continue;
        }
        int i = rx + TILE_WIDTH * ty;
        int idx = i;
        if (fx) { idx =      (i / TILE_WIDTH)  * TILE_HEIGHT + ((TILE_WIDTH - 1) - (i % TILE_WIDTH)); }
        //if (fy) { idx = (31 -(i / 32)) * 32 +        i % 32  ; }
        if (mask[idx]) { return tile->suspendType; }
    }
    return SuspendType::SUSPEND_NONE;
}

void TileMap::advanceAnimatedTileTimers() {
    for (auto& tile : animatedTiles) {
        tile->advanceTimer();
    }
}

void TileMap::advanceCollapsingTileTimers() {
    foreach(auto& tilePos, activeCollapsingTiles) {
        auto& tile = levelLayout[sprLayerIdx].tileLayout[tilePos.y][tilePos.x];
        if (tile->extraByte == 0) {
            if (!advanceDestructibleTileAnimation(tile, tilePos, "COMMON_SCENERY_COLLAPSE")) {
                tile->destructType = DESTRUCT_NONE;
                activeCollapsingTiles.remove(tilePos);
            } else {
                tile->extraByte = 4;
            }
            continue;
        }
        tile->extraByte--;
    }
}

// different quarters of the debris fly out with different x speeds
const int DestructibleDebris::speedMultiplier[4] = { -2, 2, -1, 1 };

DestructibleDebris::DestructibleDebris(std::shared_ptr<sf::Texture> texture,
    TileCoordinatePair pos, unsigned tx, unsigned ty, unsigned short quarter)
    : pos(TILE_WIDTH * (pos.x + (quarter % 2) * 0.5), TILE_HEIGHT * (pos.y + (quarter / 2) * 0.5)), speed(0, 0) {

    sprite = std::make_unique<sf::Sprite>(*texture);
    sprite->setTextureRect(sf::IntRect(
        tx * TILE_WIDTH + (quarter % 2) * (TILE_WIDTH / 2),
        ty * TILE_HEIGHT + (quarter / 2) * (TILE_HEIGHT / 2), 
        TILE_WIDTH / 2, 
        TILE_HEIGHT / 2));
    speed.x = speedMultiplier[quarter];
}

DestructibleDebris::~DestructibleDebris() {

}

void DestructibleDebris::tickUpdate() {
    pos.add(speed);
    speed.y += 0.2;
    speed.y = std::min(5.0f, speed.y);

    sprite->setPosition(pos.toSfVector2f());

}

void DestructibleDebris::drawUpdate(std::shared_ptr<GameView>& view) {
    auto canvas = view->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    canvas->draw(*sprite);
}

double DestructibleDebris::getY() {
    return pos.y;
}
