#include "TileMap.h"
#include "EventMap.h"
#include "../CarrotQt5.h"
#include "../graphics/AnimatedTile.h"




TileMap::TileMap(std::shared_ptr<CarrotQt5> gameRoot, const QString& tilesetFilename, const QString& maskFilename,
    const QString& sprLayerFilename) : levelWidth(1), levelHeight(1), root(gameRoot), sprLayerIdx(0) {
    // Reserve textures for tileset and its mask counterpart
    levelTileset.tiles = std::make_shared<sf::Texture>();
    
    // Read the tileset
    readTileset(tilesetFilename, maskFilename);

    // initialize the trigger store
    for (int i = 0; i < 256; ++i) {
        triggerState[i] = false;
    }

    // initialize the render texture and fade vertices for textured background
    int width = root->getViewWidth() / 2;

    texturedBackgroundTexture = std::make_unique<sf::RenderTexture>();
    texturedBackgroundTexture->create(256, 256);
    texturedBackgroundTexture->setRepeated(true);

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

    readLayerConfiguration(LayerType::LAYER_SPRITE_LAYER, sprLayerFilename);
    levelHeight = levelLayout.at(0).tileLayout.size() - 1;
    levelWidth = levelLayout.at(0).tileLayout.at(0).size() - 1;

    sceneryResources = root->loadActorTypeResources("Common/Scenery");
}

TileMap::~TileMap() {

}

void TileMap::readTileset(const QString& tilesFilename, const QString& maskFilename) {
    levelTileset.tiles->loadFromFile(tilesFilename.toUtf8().data());
    
    sf::Image maskfile;
    maskfile.loadFromFile(maskFilename.toUtf8().data());
    if (levelTileset.tiles->getSize() != maskfile.getSize()) {
        // mask doesn't match the tiles in size
        return;
    }
    /*if (levelTileset.tiles->getSize().x == 0) {
        // TODO: loading the tileset failed for some reason, texture is empty
        throw -1;
    }*/
    int width  = levelTileset.tiles->getSize().x / 32;
    int height = levelTileset.tiles->getSize().y / 32;
    levelTileset.tilesPerRow = width;
    levelTileset.tileCount = width * height;

    // define colors to compare pixels against
    sf::Color white_alpha(255, 255, 255, 0);
    sf::Color white(255, 255, 255, 255);
    for (unsigned i = 0; i < height; i++) {
        for (unsigned j = 0; j < width; j++) {
            QBitArray tileMask(1024);
            bool maskEmpty = true;
            bool maskFilled = true;
            for (unsigned x = 0; x < 32; ++x) {
                for (unsigned y = 0; y < 32; ++y) {
                    sf::Color px = maskfile.getPixel(j*32 + x,i*32 + y);
                    // Consider any fully white or fully transparent pixel in the masks as non-solid and all others as solid
                    bool masked = ((px != white) && (px.a > 0));
                    tileMask.setBit(x + 32*y, masked);
                    maskEmpty  &= !masked;
                    maskFilled &=  masked;
                }
            }
            levelTileset.masks << tileMask;
            levelTileset.isMaskEmpty << maskEmpty;
            levelTileset.isMaskFilled << maskFilled;

            auto sprite = std::make_shared<sf::Sprite>();
            sprite->setTexture(*(levelTileset.tiles));
            sprite->setPosition(0, 0);
            sprite->setTextureRect(
                sf::IntRect(32 * ((i * width + j) % levelTileset.tilesPerRow),
                            32 * ((i * width + j) / levelTileset.tilesPerRow),
                            32, 32));

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
            defaultLayerTile->texture = getTilesetTexture();
            defaultLayerTile->tileId = i * width + j;
            defaultLayerTile->isVine = false;

            defaultLayerTiles.append(defaultLayerTile);
        }
    }
}

void TileMap::drawLowerLevels() {
    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    // lower levels is everything below the sprite layer and the sprite layer itself
    for (unsigned layer = 0; layer < levelLayout.size(); ++layer) {
        if (levelLayout.at(layer).type == LAYER_FOREGROUND_LAYER) { continue; }
        drawLayer(levelLayout[layer], canvas);
    }
}

void TileMap::drawHigherLevels() {
    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    // higher levels is only the foreground layers
    for (unsigned layer = 0; layer < levelLayout.size(); ++layer) {
        if (levelLayout.at(layer).type != LAYER_FOREGROUND_LAYER) { continue; }
        drawLayer(levelLayout[layer], canvas);
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

void TileMap::drawTexturedBackground(TileMapLayer& layer, const double& x, const double& y, std::shared_ptr<sf::RenderWindow> target) {
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
    sf::VertexArray primitiveLines(sf::PrimitiveType::Lines,1100);
    for (int iy = 0; iy < 275; ++iy) {
        // line drawing y offset from the middle of the screen, up and down
        int ly = 25 + iy;

        // line drawing x offset from the middle of the screen on the given y distance,
        // altered by both far and near layer factors
        int lx = (root->getViewWidth() / 2) + (ly / 300.0) * 800.0 * (root->tempModifier[2] - root->tempModifier[1]);

        // texture y coordinates for above and below draw areas,
        // altered by sky copy count in the depth and perspective correction multiplier
        int ty2 = pow(300 - iy,root->tempModifier[0] / 10.0) * root->tempModifier[4] * 256.0 / pow(300.0,root->tempModifier[0] / 10.0);
        int ty = 256 * root->tempModifier[4] - ty2;
            
        // set upper half line
        primitiveLines[4 * iy].position      = sf::Vector2f(-lx, -ly);
        primitiveLines[4 * iy].texCoords     = sf::Vector2f(0 + x, ty2 + y);
        primitiveLines[4 * iy + 1].position  = sf::Vector2f(lx, -ly);
        primitiveLines[4 * iy + 1].texCoords = sf::Vector2f(root->tempModifier[2] * 256 + x, ty2 + y);
            
        // set lower half line
        primitiveLines[4 * iy + 2].position  = sf::Vector2f(-lx, ly);
        primitiveLines[4 * iy + 2].texCoords = sf::Vector2f(0 + x, ty + y);
        primitiveLines[4 * iy + 3].position  = sf::Vector2f(lx, ly);
        primitiveLines[4 * iy + 3].texCoords = sf::Vector2f(root->tempModifier[2] * 256 + x, ty + y);
    }

    // translation to the center of the screen
    sf::Transform transform;
    auto viewCenter = root->getViewCenter();
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

double TileMap::translateCoordinate(const double& coordinate, const double& speed, const double& offset, const bool& isY) const {
    // Coordinate: the "vanilla" coordinate of the tile on the layer if the layer was fixed to the sprite layer with same
    // speed and no other options. Think of its position in JCS.
    // Speed: the set layer speed; 1 for anything that moves the same speed as the sprite layer (where the objects live),
    // less than 1 for backgrounds that move slower, more than 1 for foregrounds that move faster
    // Offset: any difference to starting coordinates caused by an inherent automatic speed a layer has

    // Literal 70 is the same as in drawLayer, it's the offscreen offset of the first tile to draw.
    // Don't touch unless absolutely necessary.
    return ((coordinate * speed + offset + (70 + (isY ? (root->getViewHeight() - 200) : (root->getViewWidth() - 320)) / 2) * (speed - 1)));
}

void TileMap::drawLayer(TileMapLayer& layer, std::shared_ptr<sf::RenderWindow> target) {
    if (root->dbgShowMasked && layer.type != LAYER_SPRITE_LAYER) {
        // only draw sprite layer in collision debug mode
        return;
    }

    // Layer dimensions
    int lh = layer.tileLayout.size();
    int lw = layer.tileLayout.at(0).size();
    
    // Update offsets for moving layers
    if (layer.autoSpeedX > 1e-10) {
        layer.offsetX += layer.autoSpeedX * 2;
        while (layer.repeatX && (std::abs(layer.offsetX) > (lw * 32))) {
            layer.offsetX -= (lw * 32);
        }
    }
    if (layer.autoSpeedY > 1e-10) {
        layer.offsetY += layer.autoSpeedY * 2;
        while (layer.repeatY && (std::abs(layer.offsetY) > (lh * 32))) {
            layer.offsetY -= (lh * 32);
        }
    }

    // Get current layer offsets and speeds
    double lox = layer.offsetX;
    double loy = layer.offsetY - (layer.useInherentOffset ? (root->getViewHeight() - 200) / 2 : 0);
    double sx = layer.speedX;
    double sy = layer.speedY;
    
    // Find out coordinates for a tile from outside the boundaries from topleft corner of the screen 
    auto viewCenter = root->getViewCenter();
    double x1 = viewCenter.x - 70.0 - (root->getViewWidth() / 2);
    double y1 = viewCenter.y - 70.0 - (root->getViewHeight() / 2);
    
    // Figure out the floating point offset from the calculated coordinates and the actual tile
    // corner coordinates
    double rem_x = fmod(translateCoordinate(x1, sx, lox, false), 32);
    double rem_y = fmod(translateCoordinate(y1, sy, loy, true), 32);
    
    // Calculate the index (on the layer map) of the first tile that needs to be drawn to the
    // position determined earlier
    int tile_x = 0;
    int tile_y = 0;

    // Determine the actual drawing location on screen
    double xinter = translateCoordinate(x1, sx, lox, false) / 32.0;
    double yinter = translateCoordinate(y1, sy, loy, true) / 32.0;

    int tile_absx = 0;
    int tile_absy = 0;

    // Get the actual tile coords on the layer layout
    if (xinter > 0) {
        tile_x = static_cast<int>(floor(xinter)) % lw;
        tile_absx = static_cast<int>(floor(xinter));
    } else {
        tile_x = static_cast<int>(ceil(xinter)) % lw;
        tile_absx = static_cast<int>(ceil(xinter));
        while (tile_x < 0) {
            tile_x += lw;
        }
    }
    
    if (yinter > 0) {
        tile_y = static_cast<int>(floor(yinter)) % lh;
        tile_absy = static_cast<int>(floor(yinter));
    } else {
        tile_y = static_cast<int>(ceil(yinter)) % lh;
        tile_absy = static_cast<int>(ceil(yinter));
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
    double xstart = x1;
    double ystart = y1;
    
    // Calculate the last coordinates we want to draw to
    double x3 = x1 + 100 + root->getViewWidth();
    double y3 = y1 + 100 + root->getViewHeight();

    if (layer.isTextured && (lh == 8) && (lw == 8)) {
        drawTexturedBackground(layer,
                fmod((x1 + rem_x) * (root->tempModifier[3] / 10.0) + lox, 256.0),
                fmod((y1 + rem_y) * (root->tempModifier[3] / 10.0) + loy, 256.0), target);
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

                if (idx == 0 & !ani) { continue; }
            
                if (root->dbgShowMasked) {
                    // debug code for masks
                    sf::RectangleShape b(sf::Vector2f(32, 32));
                    b.setPosition(x2, y2);
                    if (ani) {
                        idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
                    }
                    if (levelTileset.isMaskFilled.at(idx)) {
                        b.setFillColor(sf::Color::White);
                        target->draw(b);
                    } else {
                        if (!levelTileset.isMaskEmpty.at(idx)) {
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

                std::shared_ptr<sf::Sprite> sprite = nullptr;
                if (ani) {
                    if (idx < animatedTiles.size()) {
                        auto anim = animatedTiles.at(idx);
                        sprite = anim->getCurrentTile()->sprite;
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

void TileMap::readLayerConfiguration(enum LayerType type, const QString& filename, unsigned layerIdx, QSettings& config) {
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
                        tile = defaultLayerTiles.at(type);
                        newTileRow << tile;
                        col++;
                        continue;
                    }

                    bool isFlippedX = (flags & 0x01) > 0;
                    bool isFlippedY = (flags & 0x02) > 0;
                    bool isAnimated = (flags & 0x04) > 0;

                    // Invalid tile numbers (higher than tileset tile amount) are silently changed to empty tiles
                    if (type > levelTileset.tileCount && !isAnimated) {
                        type = 0;
                    }

                    // Copy the default tile and do stuff with it
                    if (!isAnimated) {
                        tile = std::make_shared<LayerTile>(*defaultLayerTiles.at(type));
                    } else {
                        // Copy the template for isAnimated tiles from the first tile, then fix the tile ID.
                        // Cannot rely on copying the same tile as its own isAnimated tile ID, because it is
                        // possible that there are more isAnimated tiles than regular ones.
                        tile = std::make_shared<LayerTile>(*defaultLayerTiles.at(0));
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
                initializeBackgroundTexture(newLayer);

                for (int i = 0; i < 12; ++i) {
                    (*texturedBackgroundFadeArray)[i].color = newLayer.texturedBackgroundColor;
                    if (i < 2 || (i > 5 && i < 8)) {
                        (*texturedBackgroundFadeArray)[i].color.a = 0;
                    }
                }
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

    for (unsigned layer = 0; layer < levelLayout.size(); ++layer) {
        // We need to find the sprite layer, only collisions in that layer matter
        if (!(levelLayout.at(layer).type == LAYER_SPRITE_LAYER)) { continue; }

        int idx = levelLayout.at(layer).tileLayout.at(y).at(x)->tileId;
        if (levelLayout.at(layer).tileLayout.at(y).at(x)->isAnimated) {
            idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
        }

        if (levelTileset.isMaskEmpty.at(idx)) {
            return true;
        } else {
            // TODO: Pixel perfect collision wanted here? probably, so do that later
            return false;
        }
    }

    // Safeguard return value; any valid tile should be caught earlier on already
    return true;
}

bool TileMap::isTileEmpty(const Hitbox& hitbox, bool downwards) {
    // Consider out-of-level coordinates as solid walls
    if ((hitbox.right >= levelWidth * 32) || (hitbox.bottom >= levelHeight * 32)
     || (hitbox.left <= 0) || (hitbox.top <= 0)) {
        return false;
    }

    for (unsigned layer = 0; layer < levelLayout.size(); ++layer) {
        // We need to find the sprite layer, only collisions in that layer matter
        if (!(levelLayout.at(layer).type == LAYER_SPRITE_LAYER)) { continue; }

        // check all covered tiles for collisions; if all are empty, no need to do pixel level collision checking
        bool all_empty = true;
        int hx1 = floor(hitbox.left);
        int hx2 = ceil(hitbox.right);
        int hy1 = floor(hitbox.top);
        int hy2 = ceil(hitbox.bottom);
        if (root->dbgShowMasked) {
            auto target = root->getCanvas().lock();
            if (target != nullptr) {
                // debug code
                sf::RectangleShape b(sf::Vector2f((hx2 / 32 - hx1 / 32) * 32 + 32,(hy2 / 32 - hy1 / 32) * 32 + 32));
                b.setPosition(hx1 / 32 * 32,hy1 / 32 * 32);
                b.setFillColor(sf::Color::Green);
                target->draw(b);
            
                sf::RectangleShape a(sf::Vector2f(hitbox.right-hitbox.left, hitbox.bottom-hitbox.top));
                a.setPosition(hitbox.left,hitbox.top);
                a.setFillColor(sf::Color::Blue);
                target->draw(a);
            }
        }
        for (int x = hx1 / 32; x <= hx2 / 32; ++x) {
            for (int y = hy1 / 32; y <= hy2 / 32; ++y) {
                int idx = levelLayout.at(layer).tileLayout.at(y).at(x)->tileId;
                if (levelLayout.at(layer).tileLayout.at(y).at(x)->isAnimated) {
                    idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
                }

                if (!levelTileset.isMaskEmpty.at(idx) &&
                    !(levelLayout.at(layer).tileLayout.at(y).at(x)->isOneWay && !downwards) &&
                    !(levelLayout.at(layer).tileLayout.at(y).at(x)->isVine)) {
                    all_empty = false;
                    if (root->dbgShowMasked) {
                        sf::RectangleShape a(sf::Vector2f(32, 32));
                        a.setPosition(x*32, y*32);
                        a.setFillColor(sf::Color::Red);
                        auto target = root->getCanvas().lock();
                        if (target != nullptr) {
                            target->draw(a);
                        }
                    }
                    break;
                }
            }
            if (!all_empty && !root->dbgShowMasked) {
                break;
            }
        }

        if (all_empty) {
            return true;
        }

        // check each tile pixel perfectly for collisions
        for (int x = hx1 / 32; x <= hx2 / 32; ++x) {
            for (int y = hy1 / 32; y <= hy2 / 32; ++y) {
                bool fx = levelLayout.at(layer).tileLayout.at(y).at(x)->isFlippedX;
                bool fy = levelLayout.at(layer).tileLayout.at(y).at(x)->isFlippedY;
                int idx = levelLayout.at(layer).tileLayout.at(y).at(x)->tileId;
                if (levelLayout.at(layer).tileLayout.at(y).at(x)->isAnimated) {
                    idx = animatedTiles.at(idx)->getCurrentTile()->tileId;
                }

                if ((levelLayout.at(layer).tileLayout.at(y).at(x)->isOneWay
                    && !downwards && (hy2 < ((y + 1) * 32)))
                    || levelLayout.at(layer).tileLayout.at(y).at(x)->isVine) {
                    continue;
                }
                QBitArray mask = levelTileset.masks.at(idx);
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

    // Safeguard return value; any valid tile should be caught earlier on already
    return true;
}

void TileMap::updateSprLayerIdx() {
    for (unsigned layer = 0; layer < levelLayout.size(); ++layer) {
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
    return levelTileset.tiles;
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

    if (ty >= levelHeight || tx >= levelWidth) {
        return false;
    }
    auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
    if (tile->destructType == DESTRUCT_WEAPON && (tile->extraByte == 0 || tile->extraByte == (weapon + 1))) {
        if ((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex) {
            // tile not destroyed yet, advance counter by one
            tile->destructFrameIndex++;
            tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
            tile->sprite->setTextureRect(
                sf::IntRect(32 * (tile->tileId % levelTileset.tilesPerRow),
                            32 * (tile->tileId / levelTileset.tilesPerRow),
                            32, 32));
            if (!((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex)) {
                // the tile was destroyed, create debris
                auto soundSystem = root->getSoundSystem().lock();
                if (soundSystem != nullptr && sceneryResources->sounds.contains("COMMON_SCENERY_DESTRUCT")) {
                    soundSystem->playSFX(sceneryResources->sounds.value("COMMON_SCENERY_DESTRUCT").sound);
                }
                root->createDebris(animatedTiles.at(tile->destructAnimation)
                    ->getFrameCanonicalIndex(animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 1),
                    tx, ty);
            }
            return true;
        }
    }
    return false;
}

bool TileMap::checkSpecialDestructible(double x, double y) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;

    if (ty >= levelHeight || tx >= levelWidth) {
        return false;
    }
    auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
    if (tile->destructType == DESTRUCT_SPECIAL) {
        if ((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex) {
            // tile not destroyed yet, advance counter by one
            tile->destructFrameIndex++;
            tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
            tile->sprite->setTextureRect(
                sf::IntRect(32 * (tile->tileId % levelTileset.tilesPerRow),
                            32 * (tile->tileId / levelTileset.tilesPerRow),
                            32, 32));
            if (!((animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 2) > tile->destructFrameIndex)) {
                // the tile was destroyed, create debris
                auto soundSystem = root->getSoundSystem().lock();
                if (soundSystem != nullptr && sceneryResources->sounds.contains("COMMON_SCENERY_DESTRUCT")) {
                    soundSystem->playSFX(sceneryResources->sounds.value("COMMON_SCENERY_DESTRUCT").sound);
                }
                root->createDebris(animatedTiles.at(tile->destructAnimation)
                    ->getFrameCanonicalIndex(animatedTiles.at(tile->destructAnimation)->getAnimationLength() - 1),
                    tx, ty);
            }
            return true;
        }
    }
    return false;
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
    for (int tx = 0; tx < levelWidth; ++tx) {
        for (int ty = 0; ty < levelHeight; ++ty) {
            auto tile = levelLayout[sprLayerIdx].tileLayout[ty][tx];
            if (tile->destructType == DESTRUCT_TRIGGER) {
                if (tile->extraByte == triggerID) {
                if (animatedTiles.at(tile->destructAnimation)->getAnimationLength() > 1) {
                    tile->destructFrameIndex = (newState ? 1 : 0);
                    tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(tile->destructFrameIndex);
                    tile->sprite->setTextureRect(
                        sf::IntRect(32 * (tile->tileId % levelTileset.tilesPerRow),
                                    32 * (tile->tileId / levelTileset.tilesPerRow),
                                    32, 32));
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
            tile->isVine = true;
            break;
        case PC_SCENERY_DESTRUCT:
            if (tile->isAnimated) {
                tile = cloneDefaultLayerTile(x, y);
                tile->destructType = DESTRUCT_WEAPON;
                tile->isAnimated = false;
                tile->destructAnimation = tile->tileId;
                tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(0);
                tile->destructFrameIndex = 0;
                tile->sprite->setTextureRect(
                    sf::IntRect(32 * (tile->tileId % levelTileset.tilesPerRow),
                                32 * (tile->tileId / levelTileset.tilesPerRow),
                                32, 32));
                tile->extraByte = p[0];
            }
            break;
        case PC_SCENERY_BUTTSTOMP:
            if (tile->isAnimated) {
                tile = cloneDefaultLayerTile(x, y);
                tile->destructType = DESTRUCT_SPECIAL;
                tile->isAnimated = false;
                tile->destructAnimation = tile->tileId;
                tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(0);
                tile->destructFrameIndex = 0;
                tile->sprite->setTextureRect(
                    sf::IntRect(32 * (tile->tileId % levelTileset.tilesPerRow),
                                32 * (tile->tileId / levelTileset.tilesPerRow),
                                32, 32));
                tile->extraByte = p[0];
            }
            break;
        case PC_TRIGGER_AREA:
            if (tile->isAnimated) {
                tile = cloneDefaultLayerTile(x, y);
                tile->destructType = DESTRUCT_TRIGGER;
                tile->isAnimated = false;
                tile->destructAnimation = tile->tileId;
                tile->tileId = animatedTiles.at(tile->destructAnimation)->getFrameCanonicalIndex(0);
                tile->destructFrameIndex = 0;
                tile->sprite->setTextureRect(
                    sf::IntRect(32 * (tile->tileId % levelTileset.tilesPerRow),
                                32 * (tile->tileId / levelTileset.tilesPerRow),
                                32, 32));
                tile->extraByte = p[0];
            }
            break;
    }
}

bool TileMap::isPosVine(double x, double y) {
    int ax = static_cast<int>(x) / 32;
    int ay = static_cast<int>(y) / 32;
    int rx = static_cast<int>(x) - 32 * ax;
    int ry = static_cast<int>(y) - 32 * ay;

    auto tile = levelLayout[sprLayerIdx].tileLayout[ay][ax];

    bool fx = tile->isFlippedX;
    bool fy = tile->isFlippedY;
    if (!tile->isVine) {
        return false;
    }

    QBitArray mask(1024, false);
    if (tile->isAnimated) {
        if (tile->tileId < animatedTiles.size()) {
            mask = levelTileset.masks.at(animatedTiles.at(tile->tileId)->getCurrentTile()->tileId);
        }
    } else {
        mask = levelTileset.masks.at(tile->tileId);
    }
    for (int ty = ry - 4; ty < ry + 4; ty++) {
        if (ty < 0 || ty >= 32) {
            continue;
        }
        int i = rx + 32 * ty;
        int idx = i;
        if (fx) { idx =      (i / 32)  * 32 + (31 - (i % 32)); }
        //if (fy) { idx = (31 -(i / 32)) * 32 +        i % 32  ; }
        if (mask[idx]) { return true; }
    }
    return false;
}

void TileMap::advanceAnimatedTileTimers() {
    for (int i = 0; i < animatedTiles.size(); ++i) {
        animatedTiles.at(i)->advanceTimer();
    }
}

// different quarters of the debris fly out with different x speeds
const int DestructibleDebris::speedMultiplier[4] = { -2, 2, -1, 1 };

DestructibleDebris::DestructibleDebris(std::shared_ptr<sf::Texture> texture, std::weak_ptr<sf::RenderTarget> window,
    int x, int y, unsigned tx, unsigned ty, unsigned short quarter)
    : posX(x * 32 + (quarter % 2) * 16), posY(y * 32 + (quarter / 2) * 16), speedX(0), speedY(0), window(window) {

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

    auto canvas = window.lock();
    if (canvas != nullptr) {
        canvas->draw(*sprite);
    }
}

double DestructibleDebris::getY() {
    return posY;
}

