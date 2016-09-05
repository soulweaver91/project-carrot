#include "TileMap.h"

#include <cmath>
#include "EventMap.h"
#include "GameView.h"
#include "LevelManager.h"
#include "ActorAPI.h"
#include "../graphics/AnimatedTile.h"
#include "../sound/SoundSystem.h"
#include "../struct/Constants.h"
#include "../struct/DebugConfig.h"
#include "../actor/FrozenBlock.h"

TileMap::TileMap(LevelManager* root, const QString& tilesetFilename, 
    const QString& maskFilename, const QString& sprLayerFilename) 
    : root(root), sprLayerIdx(0), levelWidth(1), levelHeight(1) {
    // Reserve textures for tileset and its mask counterpart
    levelTileset = std::make_unique<Tileset>(tilesetFilename, maskFilename);
    if (!levelTileset->getIsValid()) {
        return;
    }

    // initialize the trigger store
    for (int i = 0; i < 256; ++i) {
        triggerState[i] = false;
    }

    texturedBackgroundTexture = std::make_unique<sf::RenderTexture>();
    texturedBackgroundTexture->create(256, 256);
    texturedBackgroundTexture->setRepeated(true);
    
    // The sprite layer has no settings to apply to it, so just pass an empty set instead.
    QSettings dummySettings;
    readLayerConfiguration(LayerType::LAYER_SPRITE_LAYER, sprLayerFilename, dummySettings);
    levelHeight = levelLayout.at(0).tileLayout.size() - 1;
    levelWidth = levelLayout.at(0).tileLayout.at(0).size() - 1;

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
                
            sprite->setPosition(i * 32,j * 32);
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

    // not the prettiest algorithm but the SFML drawing surface we're using doesn't seem to work with 3D OpenGL so...
    // at least this works, worry about performance later

    // TODO: set fitting constant values here and remove them from the runtime debug variable array
        
    // initialize 550 lines that we're going to texturize and draw on the screen
    // from (400, 300), first 25 lines up and down are obscured by the fade effect,
    // but the 275 other lines are at least partially visible
    double viewHeightHalf = view->getViewHeight() / 2;
    double viewWidth = view->getViewWidth();
    sf::VertexArray primitiveLines(sf::PrimitiveType::Lines, viewHeightHalf * 4 - 50);

#ifdef CARROT_DEBUG
    auto debugConfig = root->getActorAPI()->getDebugConfig();
    float perspectiveCurve        = debugConfig.tempModifier[0] / 10.0;
    int perspectiveMultiplierNear = debugConfig.tempModifier[1];
    int perspectiveMultiplierFar  = debugConfig.tempModifier[2];
    int skyDepth                  = debugConfig.tempModifier[4];
#else
    float perspectiveCurve        = 2.0;
    int perspectiveMultiplierNear = 1;
    int perspectiveMultiplierFar  = 3;
    int skyDepth                  = 1;
#endif

    for (int iy = 0; iy < viewHeightHalf - 25; ++iy) {
        // line drawing y offset from the middle of the screen, up and down
        int ly = 25 + iy;

        // line drawing x offset from the middle of the screen on the given y distance,
        // altered by both far and near layer factors
        int lx = (viewWidth / 2) + (ly / viewHeightHalf) * 800.0 * (perspectiveMultiplierFar - perspectiveMultiplierNear);

        // texture y coordinates for above and below draw areas,
        // altered by sky copy count in the depth and perspective correction multiplier
        int ty2 = pow(viewHeightHalf - iy, perspectiveCurve) * skyDepth * 256.0 / pow(viewHeightHalf, perspectiveCurve);
        int ty = 256 * skyDepth - ty2;
            
        // set upper half line
        primitiveLines[4 * iy].position      = sf::Vector2f(-lx, -ly);
        primitiveLines[4 * iy].texCoords     = sf::Vector2f(0 + x, ty2 + y);
        primitiveLines[4 * iy + 1].position  = sf::Vector2f(lx, -ly);
        primitiveLines[4 * iy + 1].texCoords = sf::Vector2f(perspectiveMultiplierFar * 256 + x, ty2 + y);
            
        // set lower half line
        primitiveLines[4 * iy + 2].position  = sf::Vector2f(-lx, ly);
        primitiveLines[4 * iy + 2].texCoords = sf::Vector2f(0 + x, ty + y);
        primitiveLines[4 * iy + 3].position  = sf::Vector2f(lx, ly);
        primitiveLines[4 * iy + 3].texCoords = sf::Vector2f(perspectiveMultiplierFar * 256 + x, ty + y);
    }

    // translation to the center of the screen
    sf::Transform transform;
    auto viewCenter = view->getViewCenter();
    transform.translate(viewCenter.x, viewCenter.y);

    // combine translation to texture binding
    sf::RenderStates states;
    states.texture = &texturedBackgroundTexture->getTexture();
    states.transform = transform;

    // draw background and the fade effect
    target->draw(primitiveLines, states);
    target->draw(*texturedBackgroundFadeArray.get(), transform);
}

std::shared_ptr<LayerTile> TileMap::cloneDefaultLayerTile(int x, int y) {
    auto tile = levelLayout[sprLayerIdx].tileLayout[y][x];
    if (tile->tilesetDefault) {
        tile = std::make_shared<LayerTile>(*tile);
        tile->tilesetDefault = false;
        levelLayout[sprLayerIdx].tileLayout[y][x] = tile;
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

    if (debugConfig.dbgShowMasked && layer.type != LAYER_SPRITE_LAYER) {
        // only draw sprite layer in collision debug mode
        return;
    }
#endif

    // Layer dimensions
    int lh = layer.tileLayout.size();
    int lw = layer.tileLayout.at(0).size();
    
    // Update offsets for moving layers
    if (layer.autoSpeedX > EPSILON) {
        layer.offsetX += layer.autoSpeedX * 2;
        while (layer.repeatX && (std::abs(layer.offsetX) > (lw * 32))) {
            layer.offsetX -= (lw * 32);
        }
    }
    if (layer.autoSpeedY > EPSILON) {
        layer.offsetY += layer.autoSpeedY * 2;
        while (layer.repeatY && (std::abs(layer.offsetY) > (lh * 32))) {
            layer.offsetY -= (lh * 32);
        }
    }

    // Get current layer offsets and speeds
    double lox = layer.offsetX;
    double loy = layer.offsetY - (layer.useInherentOffset ? (view->getViewHeight() - 200) / 2 : 0);
    double sx = layer.speedX;
    double sy = layer.speedY;
    
    // Find out coordinates for a tile from outside the boundaries from topleft corner of the screen 
    auto viewCenter = view->getViewCenter();
    double x1 = viewCenter.x - 70.0 - (view->getViewWidth() / 2);
    double y1 = viewCenter.y - 70.0 - (view->getViewHeight() / 2);

    // Get view dimensions
    uint viewHeight = view->getViewHeight();
    uint viewWidth = view->getViewWidth();
    
    // Figure out the floating point offset from the calculated coordinates and the actual tile
    // corner coordinates
    double rem_x = fmod(translateCoordinate(x1, sx, lox, false, viewHeight, viewWidth), 32);
    double rem_y = fmod(translateCoordinate(y1, sy, loy, true, viewHeight, viewWidth), 32);
    
    // Calculate the index (on the layer map) of the first tile that needs to be drawn to the
    // position determined earlier
    int tile_x = 0;
    int tile_y = 0;

    // Determine the actual drawing location on screen
    double xinter = translateCoordinate(x1, sx, lox, false, viewHeight, viewWidth) / 32.0;
    double yinter = translateCoordinate(y1, sy, loy, true, viewHeight, viewWidth) / 32.0;

    int tile_absx = 0;
    int tile_absy = 0;

    // Get the actual tile coords on the layer layout
    if (xinter > 0) {
        tile_x = static_cast<int>(std::floor(xinter)) % lw;
        tile_absx = static_cast<int>(std::floor(xinter));
    } else {
        tile_x = static_cast<int>(std::ceil(xinter)) % lw;
        tile_absx = static_cast<int>(std::ceil(xinter));
        while (tile_x < 0) {
            tile_x += lw;
        }
    }
    
    if (yinter > 0) {
        tile_y = static_cast<int>(std::floor(yinter)) % lh;
        tile_absy = static_cast<int>(std::floor(yinter));
    } else {
        tile_y = static_cast<int>(std::ceil(yinter)) % lh;
        tile_absy = static_cast<int>(std::ceil(yinter));
        while (tile_y < 0) {
            tile_y += lh;
        }
    }

    // Save the tile Y at the left border so that we can roll back to it at the start of
    // every row iteration
    unsigned tile_ys = tile_y;

    // update x1 and y1 with the remainder so that we start at the tile boundary
    // minus 1 because indices are updated in the beginning of the loops
    x1 -= rem_x - 32.0;
    y1 -= rem_y - 32.0;
    
    // Calculate the last coordinates we want to draw to
    double x3 = x1 + 100 + view->getViewWidth();
    double y3 = y1 + 100 + view->getViewHeight();

    if (layer.isTextured && (lh == 8) && (lw == 8)) {
#ifdef CARROT_DEBUG
        float perspectiveSpeed = debugConfig.tempModifier[3] / 10.0;
#else
        float perspectiveSpeed = 0.4;
#endif
        drawTexturedBackground(layer,
                fmod((x1 + rem_x) * perspectiveSpeed + lox, 256.0),
                fmod((y1 + rem_y) * perspectiveSpeed + loy, 256.0), view);
    } else {
        int tile_xo = -1;
        for (double x2 = x1; x2 < x3; x2 += 32) {
            tile_x = (tile_x + 1) % lw;
            tile_xo++;
            if (!layer.repeatX) {
                // If the current tile isn't in the first iteration of the layer horizontally, don't draw this column
                if (tile_absx + tile_xo + 1 < 0 || tile_absx + tile_xo + 1 >= lw) {
                    continue;
                }
            }
            tile_y = tile_ys;
            int tile_yo = -1;
            for (double y2 = y1; y2 < y3; y2 += 32) {
                tile_y = (tile_y + 1) % lh;
                tile_yo++;

                int idx = layer.tileLayout.at(tile_y).at(tile_x)->tileId;
                bool ani = layer.tileLayout.at(tile_y).at(tile_x)->isAnimated;

                if (!layer.repeatY) {
                    // If the current tile isn't in the first iteration of the layer vertically, don't draw it
                    if (tile_absy + tile_yo + 1 < 0 || tile_absy + tile_yo + 1 >= lh) {
                        continue;
                    }
                }

                if (idx == 0 && !ani) { continue; }

#ifdef CARROT_DEBUG
                if (debugConfig.dbgShowMasked) {
                    // debug code for masks
                    sf::RectangleShape b(sf::Vector2f(32, 32));
                    b.setPosition(x2, y2);
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
                    if (ani || layer.tileLayout.at(tile_y).at(tile_x)->destructType != TileDestructType::DESTRUCT_NONE) {
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
                            layer.tileLayout[tile_y][tile_x]->isFlippedX ? -1.0 : 1.0,
                            layer.tileLayout[tile_y][tile_x]->isFlippedY ? -1.0 : 1.0
                        );
                        sprite->setOrigin(
                            layer.tileLayout[tile_y][tile_x]->isFlippedX ? 32.0 : 0.0,
                            layer.tileLayout[tile_y][tile_x]->isFlippedY ? 32.0 : 0.0
                        );
                    }
                } else {
                    sprite = layer.tileLayout[tile_y][tile_x]->sprite;
                }

                // rounding to nearest integer is necessary because otherwise there will be tearing
                if (sprite != nullptr) {
                    sprite->setPosition((float)qRound(x2), (float)qRound(y2));
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

                    if (tile->isFlippedX || tile->isAnimated) {
                        tile->sprite = std::make_shared<sf::Sprite>(*tile->sprite);
                    }

                    // Set tile texture according to the given tile number, to the tile coordinates
                    if (tile->isFlippedX) {
                        tile->sprite = std::make_shared<sf::Sprite>(*tile->sprite);
                        tile->sprite->setScale(-1.0, 1.0);
                        tile->sprite->setOrigin(32.0, 0.0);
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

                auto size = root->getActorAPI()->getCanvasSize();
                initializeBackgroundTexture(newLayer);
                initializeTexturedBackgroundFade(size.x, size.y);
            }
        } else {
            // TODO: uncompress fail, what do?
        }
    } else {
        // TODO: could not open the file, do something here
    }
}

bool TileMap::isTileEmpty(unsigned x, unsigned y) {
    // Consider out-of-level coordinates as solid walls
    if ((x >= levelWidth * 32) || (y >= levelHeight * 32)) {
        return false;
    }

    int idx = levelLayout.at(sprLayerIdx).tileLayout.at(y).at(x)->tileId;
    if (levelLayout.at(sprLayerIdx).tileLayout.at(y).at(x)->isAnimated) {
        idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
    }

    if (levelTileset->isTileMaskEmpty(idx)) {
        return true;
    } else {
        // TODO: Pixel perfect collision wanted here? probably, so do that later
        return false;
    }
}

bool TileMap::isTileEmpty(const Hitbox& hitbox, bool downwards) {
    // Consider out-of-level coordinates as solid walls
    if ((hitbox.right >= levelWidth * 32) || (hitbox.bottom >= (levelHeight + 1) * 32)
     || (hitbox.left <= 0) || (hitbox.top <= 0)) {
        return false;
    }

    // check all covered tiles for collisions; if all are empty, no need to do pixel level collision checking
    bool all_empty = true;
    int hx1 = floor(hitbox.left);
    int hx2 = ceil(hitbox.right);
    int hy1 = floor(hitbox.top);
    int hy2 = std::min(ceil(hitbox.bottom), levelHeight * 32.0 + 31.0);

    const auto& sprLayerLayout = levelLayout.at(sprLayerIdx).tileLayout;

    for (int x = hx1 / 32; x <= hx2 / 32; ++x) {
        for (int y = hy1 / 32; y <= hy2 / 32; ++y) {
            int idx = sprLayerLayout.at(y).at(x)->tileId;
            if (sprLayerLayout.at(y).at(x)->isAnimated) {
                idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
            }

            if (!levelTileset->isTileMaskEmpty(idx) &&
                !(sprLayerLayout.at(y).at(x)->isOneWay && !downwards) &&
                !(sprLayerLayout.at(y).at(x)->suspendType != SuspendType::SUSPEND_NONE)) {
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
    for (int x = hx1 / 32; x <= hx2 / 32; ++x) {
        for (int y = hy1 / 32; y <= hy2 / 32; ++y) {
            bool fx = sprLayerLayout.at(y).at(x)->isFlippedX;
            // bool fy = sprLayerLayout.at(y).at(x)->isFlippedY;
            int idx = sprLayerLayout.at(y).at(x)->tileId;
            if (sprLayerLayout.at(y).at(x)->isAnimated) {
                idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
            }

            if ((sprLayerLayout.at(y).at(x)->isOneWay
                && !downwards && (hy2 < ((y + 1) * 32)))
                || sprLayerLayout.at(y).at(x)->suspendType != SuspendType::SUSPEND_NONE) {
                continue;
            }
            QBitArray mask = levelTileset->getTileMask(idx);
            for (int i = 0; i < 1024; ++i) {
                int nowx = (32 * x + i % 32);
                int nowy = (32 * y + i / 32);
                if (hx2 < nowx || hx1 >= nowx) {
                    continue;
                }
                if (hy2 < nowy || hy1 >= nowy) {
                    continue;
                }
                int px_idx = i;
                if (fx) { px_idx =      (i / 32)  * 32 + (31 - (i % 32)); }
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

void TileMap::initializeTexturedBackgroundFade(int width, int) {
    // initialize the render texture and fade vertices for textured background

    texturedBackgroundFadeArray = std::make_unique<sf::VertexArray>();
    texturedBackgroundFadeArray->resize(12);
    texturedBackgroundFadeArray->setPrimitiveType(sf::PrimitiveType::Quads);
    // top part of fade
    (*texturedBackgroundFadeArray)[0].position  = sf::Vector2f(-width, -200);
    (*texturedBackgroundFadeArray)[1].position  = sf::Vector2f( width, -200);
    (*texturedBackgroundFadeArray)[2].position  = sf::Vector2f( width,  -25);
    (*texturedBackgroundFadeArray)[3].position  = sf::Vector2f(-width,  -25);
    // bottom part of fade
    (*texturedBackgroundFadeArray)[4].position  = sf::Vector2f(-width,   25);
    (*texturedBackgroundFadeArray)[5].position  = sf::Vector2f( width,   25);
    (*texturedBackgroundFadeArray)[6].position  = sf::Vector2f( width,  200);
    (*texturedBackgroundFadeArray)[7].position  = sf::Vector2f(-width,  200);
    // middle solid color part
    (*texturedBackgroundFadeArray)[8].position  = sf::Vector2f(-width,  -25);
    (*texturedBackgroundFadeArray)[9].position  = sf::Vector2f( width,  -25);
    (*texturedBackgroundFadeArray)[10].position = sf::Vector2f( width,   25);
    (*texturedBackgroundFadeArray)[11].position = sf::Vector2f(-width,   25);

    for (int i = 0; i < 12; ++i) {
        (*texturedBackgroundFadeArray)[i].color = texturedBackgroundColor;
        if (i < 2 || (i > 5 && i < 8)) {
            (*texturedBackgroundFadeArray)[i].color.a = 0;
        }
    }
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
                QVector<unsigned short> frames;
                // Read type short from the stream
                for (int i = 0; i < 64; ++i) {
                    quint16 tile;
                    animStream >> tile;
                    if (tile != 0xFFFF) {
                        frames << tile;
                    }
                }

                quint16 delay, delayJitter, pingPongDelay;
                quint8 speed, pingPong;
                animStream >> speed >> delay >> delayJitter >> pingPong >> pingPongDelay;

                if (frames.size() > 0) {
                    auto ani = std::make_shared<AnimatedTile>(getTilesetTexture(), frames, speed, 
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

const std::shared_ptr<sf::Texture> TileMap::getTilesetTexture() {
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

bool TileMap::checkWeaponDestructible(double x, double y, WeaponType weapon) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;

    if (ty >= static_cast<int>(levelHeight) || tx >= static_cast<int>(levelWidth)) {
        return false;
    }
    auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
    if (tile->destructType == DESTRUCT_WEAPON) {
        if (weapon == WEAPON_FREEZER && (animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex) {
            auto e = std::make_shared<FrozenBlock>(root->getActorAPI(), 32 * tx + 16.0, 32.0 * ty + 16.0);
            root->addActor(e);

            return true;
        }

        if (tile->extraByte == 0u || tile->extraByte == static_cast<uint>(weapon + 1)) {
            return advanceDestructibleTileAnimation(tile, tx, ty, "COMMON_SCENERY_DESTRUCT");
        }
    }

    return false;
}

uint TileMap::checkSpecialDestructible(const Hitbox& hitbox) {
    int x1 = hitbox.left / 32;
    int x2 = std::min((uint)(hitbox.right / 32), levelWidth);
    int y1 = hitbox.top / 32;
    int y2 = std::min((uint)(hitbox.bottom / 32), levelHeight);
    int hit = 0;
    for (int tx = x1; tx <= x2; ++tx) {
        for (int ty = y1; ty <= y2; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_SPECIAL) {
                if (advanceDestructibleTileAnimation(tile, tx, ty, "COMMON_SCENERY_DESTRUCT")) {
                    hit++;
                }
            }
        }
    }
    return hit;
}

uint TileMap::checkSpecialSpeedDestructible(const Hitbox& hitbox, const double& speed) {
    int x1 = hitbox.left / 32;
    int x2 = std::min((uint)(hitbox.right / 32), levelWidth);
    int y1 = hitbox.top / 32;
    int y2 = std::min((uint)(hitbox.bottom / 32), levelHeight);
    uint hit = 0;
    for (int tx = x1; tx <= x2; ++tx) {
        for (int ty = y1; ty <= y2; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_SPEED && tile->extraByte + 3 <= speed) {
                if (advanceDestructibleTileAnimation(tile, tx, ty, "COMMON_SCENERY_DESTRUCT")) {
                    hit++;
                }
            }
        }
    }
    return hit;
}

uint TileMap::checkCollapseDestructible(const Hitbox& hitbox) {
    int x1 = hitbox.left / 32;
    int x2 = std::min((uint)(hitbox.right / 32), levelWidth);
    int y1 = hitbox.top / 32;
    int y2 = std::min((uint)(hitbox.bottom / 32), levelHeight);
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
            if (tile->destructType == DESTRUCT_TRIGGER) {
                if (tile->extraByte == triggerID) {
                if (animatedTiles.at(tile->destructAnimation)->getAnimationLength() > 1) {
                    tile->destructFrameIndex = (newState ? 1u : 0u);
                    tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
                    tile->sprite->setTextureRect(levelTileset->getTileTextureRect(tile->tileId));
                }
            }}
        }
    }
}

bool TileMap::getTrigger(unsigned char triggerID) {
    return triggerState[triggerID];
}

unsigned TileMap::getLevelWidth() {
    return levelWidth;
}
unsigned TileMap::getLevelHeight() {
    return levelHeight;
}

void TileMap::setTileEventFlag(int x, int y, PCEvent e) {
    auto tile = levelLayout[sprLayerIdx].tileLayout[y][x];
    quint16 p[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    auto tiles = root->getGameEvents().lock();
    if (tiles != nullptr) {
        tiles->getPositionParams(x, y, p);
    }

    switch (e) {
        case PC_MODIFIER_ONE_WAY:
            tile = cloneDefaultLayerTile(x, y);
            tile->isOneWay = true;
            break;
        case PC_MODIFIER_VINE:
            tile = cloneDefaultLayerTile(x, y);
            tile->suspendType = SuspendType::SUSPEND_VINE;
            break;
        case PC_MODIFIER_HOOK:
            tile = cloneDefaultLayerTile(x, y);
            tile->suspendType = SuspendType::SUSPEND_HOOK;
            break;
        case PC_SCENERY_DESTRUCT:
            setTileDestructibleEventFlag(tile, x, y, DESTRUCT_WEAPON, p[0]);
            break;
        case PC_SCENERY_BUTTSTOMP:
            setTileDestructibleEventFlag(tile, x, y, DESTRUCT_SPECIAL, p[0]);
            break;
        case PC_TRIGGER_AREA:
            setTileDestructibleEventFlag(tile, x, y, DESTRUCT_TRIGGER, p[0]);
            break;
        case PC_SCENERY_DESTRUCT_SPD:
            setTileDestructibleEventFlag(tile, x, y, DESTRUCT_SPEED, p[0]);
            break;
        case PC_SCENERY_COLLAPSE:
            setTileDestructibleEventFlag(tile, x, y, DESTRUCT_COLLAPSE, p[0] * 25);
            break;
        default:
            break;
    }
}

void TileMap::setTileDestructibleEventFlag(std::shared_ptr<LayerTile>& tile, const uint& x, const uint& y,
    const TileDestructType& type, const quint16& extraByte) {
    if (tile->isAnimated) {
        tile = cloneDefaultLayerTile(x, y);
        tile->destructType = type;
        tile->isAnimated = false;
        tile->destructAnimation = tile->tileId;
        tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(0);
        tile->destructFrameIndex = 0;
        tile->sprite->setTextureRect(levelTileset->getTileTextureRect(tile->tileId));
        tile->extraByte = extraByte;
    }
}

bool TileMap::advanceDestructibleTileAnimation(std::shared_ptr<LayerTile>& tile, const int& x, const int& y, const QString& soundName) {
    if ((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex) {
        // tile not destroyed yet, advance counter by one
        tile->destructFrameIndex++;
        tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
        tile->sprite->setTextureRect(levelTileset->getTileTextureRect(tile->tileId));
        if (!((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex)) {
            // the tile was destroyed, create debris
            auto soundSystem = root->getActorAPI()->getSoundSystem();
            if (sceneryResources->sounds.contains(soundName)) {
                soundSystem->playSFX(sceneryResources->sounds.value(soundName).sound, { x * 32 + 16, y * 32 + 16 });
            }
            root->createDebris(animatedTiles.at(tile->destructAnimation)
                ->getFrameCanonicalIndex(animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 1),
                x, y);
        }
        return true;
    }
    return false;
}

SuspendType TileMap::getPosSuspendState(double x, double y) {
    int ax = static_cast<int>(x) / 32;
    int ay = static_cast<int>(y) / 32;
    int rx = static_cast<int>(x) - 32 * ax;
    int ry = static_cast<int>(y) - 32 * ay;

    auto tile = levelLayout[sprLayerIdx].tileLayout[ay][ax];

    bool fx = tile->isFlippedX;
    //bool fy = tile->isFlippedY;
    if (tile->suspendType == SuspendType::SUSPEND_NONE) {
        return SuspendType::SUSPEND_NONE;
    }

    QBitArray mask(1024, false);
    if (tile->isAnimated) {
        if (tile->tileId < static_cast<uint>(animatedTiles.size())) {
            mask = levelTileset->getTileMask(animatedTiles.at(tile->tileId)->getCurrentTile()->tileId);
        }
    } else {
        mask = levelTileset->getTileMask(tile->tileId);
    }
    for (int ty = ry - 4; ty < ry + 4; ty++) {
        if (ty < 0 || ty >= 32) {
            continue;
        }
        int i = rx + 32 * ty;
        int idx = i;
        if (fx) { idx =      (i / 32)  * 32 + (31 - (i % 32)); }
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
    foreach(auto& tilePosition, activeCollapsingTiles) {
        auto& tile = levelLayout[sprLayerIdx].tileLayout[tilePosition.second][tilePosition.first];
        if (tile->extraByte == 0) {
            if (!advanceDestructibleTileAnimation(tile, tilePosition.first, tilePosition.second, "COMMON_SCENERY_COLLAPSE")) {
                tile->destructType = DESTRUCT_NONE;
                activeCollapsingTiles.remove(tilePosition);
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
    int x, int y, unsigned tx, unsigned ty, unsigned short quarter)
    : posX(x * 32 + (quarter % 2) * 16), posY(y * 32 + (quarter / 2) * 16), speedX(0), speedY(0) {

    sprite = std::make_unique<sf::Sprite>(*texture);
    sprite->setTextureRect(sf::IntRect(tx * 32 + (quarter % 2) * 16, ty * 32 + (quarter / 2) * 16, 16, 16));
    speedX = speedMultiplier[quarter];
}

DestructibleDebris::~DestructibleDebris() {

}

void DestructibleDebris::tickUpdate() {
    posX += speedX;
    posY += speedY;
    speedY += 0.2;
    speedY = std::min(5.0, speedY);

    sprite->setPosition(posX, posY);

}

void DestructibleDebris::drawUpdate(std::shared_ptr<GameView>& view) {
    auto canvas = view->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    canvas->draw(*sprite);
}

double DestructibleDebris::getY() {
    return posY;
}
