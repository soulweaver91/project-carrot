#include "TileMap.h"
#include "EventMap.h"
#include "CarrotQt5.h"
#include "AnimatedTile.h"
#include "CommonActor.h"

TileMap::TileMap(CarrotQt5* game_root, const QString& tileset_file, const QString& mask_file, const QString& spr_layer_file) :
    level_width(1), level_height(1), root(game_root), spr_layer(0) {
    // Reserve textures for tileset and its mask counterpart
    level_tileset.tiles = new sf::Texture();
    
    // Read the tileset
    readTileset(tileset_file,mask_file);

    // initialize the trigger store
    for (int i = 0; i < 256; ++i) {
        trigger_state[i] = false;
    }

    // initialize the render texture and fade vertices for textured background
    int w = root->getViewWidth() / 2;

    tex_back = new sf::RenderTexture();
    tex_back->create(256,256);
    tex_back->setRepeated(true);
    tex_fade.resize(12);
    tex_fade.setPrimitiveType(sf::PrimitiveType::Quads);
    // top part of fade
    tex_fade[0].position = sf::Vector2f(-w,-200);
    tex_fade[1].position = sf::Vector2f( w,-200);
    tex_fade[2].position = sf::Vector2f( w, -25);
    tex_fade[3].position = sf::Vector2f(-w, -25);
    // bottom part of fade
    tex_fade[4].position = sf::Vector2f(-w,  25);
    tex_fade[5].position = sf::Vector2f( w,  25);
    tex_fade[6].position = sf::Vector2f( w, 200);
    tex_fade[7].position = sf::Vector2f(-w, 200);
    // middle solid color part
    tex_fade[8].position = sf::Vector2f(-w, -25);
    tex_fade[9].position = sf::Vector2f( w, -25);
    tex_fade[10].position = sf::Vector2f( w, 25);
    tex_fade[11].position = sf::Vector2f(-w, 25);

    readLayerConfiguration(LayerType::LAYER_SPRITE_LAYER,spr_layer_file);
    level_height = level_layout.at(0).tile_layout.size() - 1;
    level_width = level_layout.at(0).tile_layout.at(0).size() - 1;
}

TileMap::~TileMap() {
    // TODO: might've missed some pointers to be freed
    delete tex_back;
}

void TileMap::readTileset(const QString& file_tiles, const QString& file_mask) {
    level_tileset.tiles->loadFromFile(file_tiles.toUtf8().data());
    
    sf::Image maskfile;
    maskfile.loadFromFile(file_mask.toUtf8().data());
    if (level_tileset.tiles->getSize() != maskfile.getSize()) {
        // mask doesn't match the tiles in size
        return;
    }
    /*if (level_tileset.tiles->getSize().x == 0) {
        // TODO: loading the tileset failed for some reason, texture is empty
        throw -1;
    }*/
    int width  = level_tileset.tiles->getSize().x / 32;
    int height = level_tileset.tiles->getSize().y / 32;
    level_tileset.tiles_col = width;
    level_tileset.tile_amount = width * height;

    // define colors to compare pixels against
    sf::Color white_alpha(255,255,255,0);
    sf::Color white(255,255,255,255);
    for (unsigned i = 0; i < height; i++) {
        for (unsigned j = 0; j < width; j++) {
            QBitArray tilemask(1024);
            bool maskempty = true;
            bool maskfull = true;
            for (unsigned x = 0; x < 32; ++x) {
                for (unsigned y = 0; y < 32; ++y) {
                    sf::Color px = maskfile.getPixel(j*32 + x,i*32 + y);
                    // Consider any fully white or fully transparent pixel in the masks as non-solid and all others as solid
                    bool masked = ((px != white) && (px.a > 0));
                    tilemask.setBit(x + 32*y, masked);
                    maskempty &= !masked;
                    maskfull  &=  masked;
                }
            }
            level_tileset.masks << tilemask;
            level_tileset.mask_empty << maskempty;
            level_tileset.mask_full << maskfull;
        }
    }
}

void TileMap::drawLowerLevels() {
    // lower levels is everything below the sprite layer and the sprite layer itself
    for (unsigned layer = 0; layer < level_layout.size(); ++layer) {
        if (level_layout.at(layer).type == LAYER_FOREGROUND_LAYER) { continue; }
        drawLayer(&level_layout[layer]);
    }
}

void TileMap::drawHigherLevels() {
    // higher levels is only the foreground layers
    for (unsigned layer = 0; layer < level_layout.size(); ++layer) {
        if (level_layout.at(layer).type != LAYER_FOREGROUND_LAYER) { continue; }
        drawLayer(&level_layout[layer]);
    }
}

void TileMap::initializeBackgroundTexture(TileMapLayer* background) {
    // basically just draw all tiles onto the background texture once
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = background->tile_layout.at(j).at(i).tile_id;
            bool ani = background->tile_layout.at(j).at(i).animated;

            sf::Sprite* spr = nullptr;
            if (ani) {
                if (idx < animated_tiles.size()) {
                    AnimatedTile* anim = animated_tiles.at(idx);
                    spr = &(anim->getCurrentTile()->sprite);
                }
            } else {
                spr = &background->tile_layout[j][i].sprite;
            }
                
            spr->setPosition(i * 32,j * 32);
            tex_back->draw(*spr);
        }
    }
}

void TileMap::drawTexturedBackground(TileMapLayer* layer, const double& x, const double& y) {
    // update animated tiles on the cache texture
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = layer->tile_layout.at(j).at(i).tile_id;
            bool ani = layer->tile_layout.at(j).at(i).animated;

            if (ani) {
                if (idx < animated_tiles.size()) {
                    AnimatedTile* anim = animated_tiles.at(idx);
                    sf::Sprite* spr = &(anim->getCurrentTile()->sprite);
                    spr->setPosition(i * 32,j * 32);
                    tex_back->draw(*spr);
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
    sf::VertexArray prim(sf::PrimitiveType::Lines,1100);
    for (int iy = 0; iy < 275; ++iy) {
        // line drawing y offset from the middle of the screen, up and down
        int ly = 25 + iy;

        // line drawing x offset from the middle of the screen on the given y distance,
        // altered by both far and near layer factors
        int lx = (root->getViewWidth() / 2) + (ly / 300.0) * 800.0 * (root->mod_temp[2] - root->mod_temp[1]);

        // texture y coordinates for above and below draw areas,
        // altered by sky copy count in the depth and perspective correction multiplier
        int ty2 = pow(300 - iy,root->mod_temp[0] / 10.0) * root->mod_temp[4] * 256.0/pow(300.0,root->mod_temp[0] / 10.0);
        int ty = 256 * root->mod_temp[4] - ty2;
            
        // set upper half line
        prim[4*iy].position    = sf::Vector2f(-lx, -ly);
        prim[4*iy].texCoords   = sf::Vector2f(0 + x,  ty2 + y);
        prim[4*iy+1].position  = sf::Vector2f(lx, -ly);
        prim[4*iy+1].texCoords = sf::Vector2f(root->mod_temp[2] * 256 + x, ty2 + y);
            
        // set lower half line
        prim[4*iy+2].position  = sf::Vector2f(-lx, ly);
        prim[4*iy+2].texCoords = sf::Vector2f(0 + x,  ty + y);
        prim[4*iy+3].position  = sf::Vector2f(lx, ly);
        prim[4*iy+3].texCoords = sf::Vector2f(root->mod_temp[2] * 256 + x, ty + y);
    }

    // translation to the center of the screen
    sf::Transform transform;
    transform.translate(root->game_view->getCenter());

    // combine translation to texture binding
    sf::RenderStates states;
    states.texture = &tex_back->getTexture();
    states.transform = transform;

    // draw background and the fade effect
    root->window->draw(prim,states);
    root->window->draw(tex_fade,transform);
}

double TileMap::translateCoordinate(const double& coordinate, const double& speed, const double& offset, const bool& is_y) const {
    // Coordinate: the "vanilla" coordinate of the tile on the layer if the layer was fixed to the sprite layer with same
    // speed and no other options. Think of its position in JCS.
    // Speed: the set layer speed; 1 for anything that moves the same speed as the sprite layer (where the objects live),
    // less than 1 for backgrounds that move slower, more than 1 for foregrounds that move faster
    // Offset: any difference to starting coordinates caused by an inherent automatic speed a layer has

    // Literal 70 is the same as in drawLayer, it's the offscreen offset of the first tile to draw.
    // Don't touch unless absolutely necessary.
    return ((coordinate * speed + offset + (70 + (is_y ? (root->getViewHeight() - 200) : (root->getViewWidth() - 320)) / 2) * (speed - 1)));
}

void TileMap::drawLayer(TileMapLayer* layer) {
    if (root->dbgShowMasked && layer->type != LAYER_SPRITE_LAYER) {
        // only draw sprite layer in collision debug mode
        return;
    }

    // Layer dimensions
    int lh = layer->tile_layout.size();
    int lw = layer->tile_layout.at(0).size();
    
    // Update offsets for moving layers
    if (layer->auto_xspeed > 1e-10) {
        layer->xoffset += layer->auto_xspeed * 2;
        while (layer->xrepeat && (std::abs(layer->xoffset) > (lw * 32))) {
            layer->xoffset -= (lw * 32);
        }
    }
    if (layer->auto_yspeed > 1e-10) {
        layer->yoffset += layer->auto_yspeed * 2;
        while (layer->yrepeat && (std::abs(layer->yoffset) > (lh * 32))) {
            layer->yoffset -= (lh * 32);
        }
    }

    // Get current layer offsets and speeds
    double lox = layer->xoffset;
    double loy = layer->yoffset - (layer->inherent_offset ? (root->getViewHeight() - 200) / 2 : 0);
    double sx = layer->xspeed;
    double sy = layer->yspeed;
    
    // Find out coordinates for a tile from outside the boundaries from topleft corner of the screen 
    double x1 = root->game_view->getCenter().x - 70.0 - (root->getViewWidth() / 2);
    double y1 = root->game_view->getCenter().y - 70.0 - (root->getViewHeight() / 2);
    
    // Figure out the floating point offset from the calculated coordinates and the actual tile
    // corner coordinates
    double rem_x = fmod(translateCoordinate(x1,sx,lox,false), 32);
    double rem_y = fmod(translateCoordinate(y1,sy,loy,true), 32);
    
    // Calculate the index (on the layer map) of the first tile that needs to be drawn to the
    // position determined earlier
    int tile_x = 0;
    int tile_y = 0;

    // Determine the actual drawing location on screen
    double xinter = translateCoordinate(x1,sx,lox,false) / 32.0;
    double yinter = translateCoordinate(y1,sy,loy,true) / 32.0;

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

    if (layer->textured && (lh == 8) && (lw == 8)) {
        drawTexturedBackground(layer,
                fmod((x1 + rem_x) * (root->mod_temp[3] / 10.0) + lox, 256.0),
                fmod((y1 + rem_y) * (root->mod_temp[3] / 10.0) + loy, 256.0));
    } else {
        int tile_xo = -1;
        for (double x2 = x1; x2 < x3; x2 += 32) {
            tile_x = (tile_x + 1) % lw;
            tile_xo++;
            if (!layer->xrepeat) {
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

                int idx = layer->tile_layout.at(tile_y).at(tile_x).tile_id;
                bool ani = layer->tile_layout.at(tile_y).at(tile_x).animated;

                if (!layer->yrepeat) {
                    // If the current tile isn't in the first iteration of the layer vertically, don't draw it
                    if (tile_absy + tile_yo + 1 < 0 || tile_absy + tile_yo + 1 >= lh) {
                        continue;
                    }
                }

                if (idx == 0 & !ani) { continue; }
            
                if (root->dbgShowMasked) {
                    // debug code for masks
                    sf::RectangleShape b(sf::Vector2f(32, 32));
                    b.setPosition(x2,y2);
                    if (ani) {
                        idx = animated_tiles.at(idx)->getCurrentTile()->tile_id;
                    }
                    if (level_tileset.mask_full.at(idx)) {
                        b.setFillColor(sf::Color::White);
                        root->window->draw(b);
                    } else {
                        if (!level_tileset.mask_empty.at(idx)) {
                            b.setFillColor(sf::Color(255,255,255,128));
                            root->window->draw(b);
                        }
                    }
                    if (ani || layer->tile_layout.at(tile_y).at(tile_x).dtype != TileDestructType::DESTRUCT_NONE) {
                        b.setFillColor(sf::Color::Transparent);
                        b.setOutlineThickness(-3);
                        b.setOutlineColor(sf::Color(255,0,128,255));
                        root->window->draw(b);
                    }
                    continue;
                }

                sf::Sprite* spr = nullptr;
                if (ani) {
                    if (idx < animated_tiles.size()) {
                        AnimatedTile* anim = animated_tiles.at(idx);
                        spr = &(anim->getCurrentTile()->sprite);
                    }
                } else {
                    spr = &layer->tile_layout[tile_y][tile_x].sprite;
                }

                // rounding to nearest integer is necessary because otherwise there will be tearing
                if (spr != nullptr) {
                    spr->setPosition((float)qRound(x2), (float)qRound(y2));
                    root->window->draw(*spr);
                }
            }
        }
    }
}

void TileMap::readLayerConfiguration(enum LayerType type, const QString& filename, unsigned layer_idx, QSettings* config) {
    // Build a new layer
    TileMapLayer new_layer;

    // Layer index and type are given as parameters
    new_layer.idx = layer_idx;
    new_layer.type = type;

    // Open the layer file
    QFile handle(filename);
    if (handle.open(QIODevice::ReadOnly)) {
        // Uncompress the compressed data
        QByteArray layer_data = qUncompress(handle.readAll());

        if (layer_data.size() > 0) {
            // Create a data stream of the data
            QDataStream outstr(layer_data);

            int row = 0;
            // Read data until no more data available
            while (!outstr.atEnd()) {
                QList< LayerTile > new_row;
                int col = 0;

                while (!outstr.atEnd()) {
                    LayerTile tile;

                    // Read type short from the stream
                    quint16 type;
                    outstr >> type;
                    if (type == 0xFFFF) {
                        // 0xFFFF means a line break
                        break;
                    }
                    // Otherwise it was a tile; read tile flags next
                    quint8 flags;
                    outstr >> flags;

                    tile.tile_id = type;
                    tile.flipped_x = (flags & 0x01) > 0;
                    tile.flipped_y = (flags & 0x02) > 0;
                    tile.animated  = (flags & 0x04) > 0;
                    tile.vine = false;
                    tile.oneway = false;
                    tile.dtype = TileDestructType::DESTRUCT_NONE;
                    tile.extra_byte = 0;

                    if (tile.animated) {
                        int i = 0;
                    }
                    
                    // Invalid tile numbers (higher than tileset tile amount) are silently changed to empty tiles
                    if (tile.tile_id > level_tileset.tile_amount && !tile.animated) {
                        tile.tile_id = 0;
                    }

                    // Set tile texture according to the given tile number, to the tile coordinates
                    tile.sprite.setTexture(*(level_tileset.tiles));
                    tile.sprite.setPosition(col*32.0, row*32.0);
                    tile.sprite.setTextureRect(
                        sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                                    32 * (tile.tile_id / level_tileset.tiles_col),
                                    32,32));
                    if (tile.flipped_x) {
                        tile.sprite.setScale(-1.0,1.0);
                        tile.sprite.setOrigin(32.0,0.0);
                    }

                    new_row.push_back(tile);
                    col++;
                }
                new_layer.tile_layout.push_back(new_row);
                row++;
            }
            
            if (config == nullptr) {
                new_layer.xspeed = 1;
                new_layer.yspeed = 1;
                new_layer.xrepeat = false;
                new_layer.yrepeat = false;
                new_layer.auto_xspeed = 0.0;
                new_layer.auto_yspeed = 0.0;
                new_layer.inherent_offset = false;
                new_layer.textured = false;
                new_layer.textured_stars = false;
                new_layer.textured_color = sf::Color::Black;
            } else {
                config->beginGroup(filename.section('/', -1));
                new_layer.xspeed = config->value("XSpeed",1.0).toDouble();
                new_layer.yspeed = config->value("YSpeed",1.0).toDouble();
                new_layer.xrepeat = config->value("XRepeat",false).toBool();
                new_layer.yrepeat = config->value("YRepeat",false).toBool();
                new_layer.auto_xspeed = config->value("XAutoSpeed",0.0).toDouble();
                new_layer.auto_yspeed = config->value("YAutoSpeed",0.0).toDouble();
                new_layer.inherent_offset = config->value("InherentOffset",false).toBool();
                new_layer.textured = config->value("TexturedModeEnabled",false).toBool();
                new_layer.textured_stars = config->value("ParallaxStarsEnabled",false).toBool();
                QString color = config->value("TexturedModeColor","#000000").toString();
                if (color.length() == 7) {
                    bool foo; // we won't care about invalid data too much here, unrecognized input will be converted to 0
                    new_layer.textured_color = sf::Color(color.mid(1,2).toInt(&foo,16),color.mid(3,2).toInt(&foo,16),color.mid(5,2).toInt(&foo,16));
                } else {
                    new_layer.textured_color = sf::Color::Black;
                }
                config->endGroup();
            }
            new_layer.xoffset = 0.0;
            new_layer.yoffset = 0.0;

            level_layout.push_back(new_layer);

            // Sort the layers by their layer type and number
            std::sort(level_layout.begin(),level_layout.end());
            updateSprLayerIdx();

            if (type == LAYER_SKY_LAYER && new_layer.textured) {
                initializeBackgroundTexture(&new_layer);

                for (int i = 0; i < 12; ++i) {
                    tex_fade[i].color = new_layer.textured_color;
                    if (i < 2 || (i > 5 && i < 8)) {
                        tex_fade[i].color.a = 0;
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
    if ((x >= level_width * 32) || (y >= level_height * 32)) {
        return false;
    }

    for (unsigned layer = 0; layer < level_layout.size(); ++layer) {
        // We need to find the sprite layer, only collisions in that layer matter
        if (!(level_layout.at(layer).type == LAYER_SPRITE_LAYER)) { continue; }

        int idx = level_layout.at(layer).tile_layout.at(y).at(x).tile_id;
        if (level_layout.at(layer).tile_layout.at(y).at(x).animated) {
            idx = animated_tiles.at(idx)->getCurrentTile()->tile_id;
        }

        if (level_tileset.mask_empty.at(idx)) {
            return true;
        } else {
            // TODO: Pixel perfect collision wanted here? probably, so do that later
            return false;
        }
    }

    // Safeguard return value; any valid tile should be caught earlier on already
    return true;
}

bool TileMap::isTileEmpty(const Hitbox& hbox, bool downwards) {
    // Consider out-of-level coordinates as solid walls
    if ((hbox.right >= level_width * 32) || (hbox.bottom >= level_height * 32)
     || (hbox.left <= 0) || (hbox.top <= 0)) {
        return false;
    }

    for (unsigned layer = 0; layer < level_layout.size(); ++layer) {
        // We need to find the sprite layer, only collisions in that layer matter
        if (!(level_layout.at(layer).type == LAYER_SPRITE_LAYER)) { continue; }

        // check all covered tiles for collisions; if all are empty, no need to do pixel level collision checking
        bool all_empty = true;
        int hx1 = floor(hbox.left);
        int hx2 = ceil(hbox.right);
        int hy1 = floor(hbox.top);
        int hy2 = ceil(hbox.bottom);
        if (root->dbgShowMasked) {
            // debug code
            sf::RectangleShape b(sf::Vector2f((hx2 / 32 - hx1 / 32) * 32 + 32,(hy2 / 32 - hy1 / 32) * 32 + 32));
            b.setPosition(hx1 / 32 * 32,hy1 / 32 * 32);
            b.setFillColor(sf::Color::Green);
            root->window->draw(b);
            
            sf::RectangleShape a(sf::Vector2f(hbox.right-hbox.left, hbox.bottom-hbox.top));
            a.setPosition(hbox.left,hbox.top);
            a.setFillColor(sf::Color::Blue);
            root->window->draw(a);
        }
        for (int x = hx1 / 32; x <= hx2 / 32; ++x) {
            for (int y = hy1 / 32; y <= hy2 / 32; ++y) {
                int idx = level_layout.at(layer).tile_layout.at(y).at(x).tile_id;
                if (level_layout.at(layer).tile_layout.at(y).at(x).animated) {
                    idx = animated_tiles.at(idx)->getCurrentTile()->tile_id;
                }

                if (!level_tileset.mask_empty.at(idx) &&
                    !(level_layout.at(layer).tile_layout.at(y).at(x).oneway && !downwards) &&
                    !(level_layout.at(layer).tile_layout.at(y).at(x).vine)) {
                    all_empty = false;
                    if (root->dbgShowMasked) {
                        sf::RectangleShape a(sf::Vector2f(32,32));
                        a.setPosition(x*32,y*32);
                        a.setFillColor(sf::Color::Red);
                        root->window->draw(a);
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
                bool fx = level_layout.at(layer).tile_layout.at(y).at(x).flipped_x;
                bool fy = level_layout.at(layer).tile_layout.at(y).at(x).flipped_y;
                int idx = level_layout.at(layer).tile_layout.at(y).at(x).tile_id;
                if (level_layout.at(layer).tile_layout.at(y).at(x).animated) {
                    idx = animated_tiles.at(idx)->getCurrentTile()->tile_id;
                }

                if ((level_layout.at(layer).tile_layout.at(y).at(x).oneway
                    && !downwards && (hy2 < ((y + 1) * 32)))
                    || level_layout.at(layer).tile_layout.at(y).at(x).vine) {
                    continue;
                }
                QBitArray mask = level_tileset.masks.at(idx);
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
                    if (mask[px_idx]) { return false; }
                }
            }
        }
        return true;
    }

    // Safeguard return value; any valid tile should be caught earlier on already
    return true;
}

void TileMap::updateSprLayerIdx() {
    for (unsigned layer = 0; layer < level_layout.size(); ++layer) {
        if (!(level_layout.at(layer).type == LAYER_SPRITE_LAYER)) {
            continue;
        }
        spr_layer = layer;
        return;
    }
}

void TileMap::readAnimatedTiles(const QString& filename) {
    // Open the given file
    QFile handle(filename);
    if (handle.open(QIODevice::ReadOnly)) {
        // Uncompress the compressed data
        QByteArray layer_data = qUncompress(handle.readAll());

        if (layer_data.size() > 0) {
            // Create a data stream of the data
            QDataStream outstr(layer_data);

            // Read data until no more data available
            while (!outstr.atEnd()) {
                QList< unsigned short > frames;
                // Read type short from the stream
                for (int i = 0; i < 64; ++i) {
                    quint16 tile;
                    outstr >> tile;
                    if (tile != 0xFFFF) {
                        frames << tile;
                    }
                }

                quint16 delay, delay_jitter, reverse_delay;
                quint8 speed, reverse;
                outstr >> speed >> delay >> delay_jitter >> reverse >> reverse_delay;

                if (frames.size() > 0) {
                    AnimatedTile* ani = new AnimatedTile(this, frames, speed, delay, delay_jitter, (reverse > 0), reverse_delay);
                    animated_tiles.push_back(ani);
                }
            }
            
        } else {
            // TODO: uncompress fail, what do?
        }
    } else {
        // TODO: could not open the file, do something here
    }
}

const sf::Texture* TileMap::getTilesetTexture() {
    return level_tileset.tiles;
}


QList< QList< LayerTile > > TileMap::prepareSavePointLayer() {
    /*
    TODO: make use of this improved algorithm as an option

    std::vector< std::vector< LayerTile > > layer = level_layout.at(spr_layer).tile_layout;
    for (int i = 0; i < layer.size(); ++i) {
        for (int j = 0; j < layer.at(i).size(); ++j) {
            layer.at(i).at(j).event_active = false;
        }
    }
    return layer;*/
    return initial_spr_layer_copy;
}

void TileMap::loadSavePointLayer(const QList< QList< LayerTile > >& layer) {
    level_layout[spr_layer].tile_layout = layer;
}

bool TileMap::checkWeaponDestructible(double x, double y, WeaponType weapon) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;

    if (ty >= level_height || tx >= level_width) {
        return false;
    }
    LayerTile& tile = level_layout[spr_layer].tile_layout[ty][tx];
    if (tile.dtype == DESTRUCT_WEAPON && (tile.extra_byte == 0 || tile.extra_byte == (weapon + 1))) {
        if ((animated_tiles.at(tile.d_animation)->getAnimationLength() - 2) > tile.scenery_frame_idx) {
            // tile not destroyed yet, advance counter by one
            tile.scenery_frame_idx++;
            tile.tile_id = animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(tile.scenery_frame_idx);
            tile.sprite.setTextureRect(
                sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                            32 * (tile.tile_id / level_tileset.tiles_col),
                            32,32));
            if (!((animated_tiles.at(tile.d_animation)->getAnimationLength() - 2) > tile.scenery_frame_idx)) {
                // the tile was destroyed, create debris
                root->sfxsys->playSFX(SFX_BLOCK_DESTRUCT);
                root->createDebris(animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(animated_tiles.at(tile.d_animation)->getAnimationLength() - 1),tx,ty);
            }
            return true;
        }
    }
    return false;
}

bool TileMap::checkSpecialDestructible(double x, double y) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;

    if (ty >= level_height || tx >= level_width) {
        return false;
    }
    LayerTile& tile = level_layout[spr_layer].tile_layout[ty][tx];
    if (tile.dtype == DESTRUCT_SPECIAL) {
        if ((animated_tiles.at(tile.d_animation)->getAnimationLength() - 2) > tile.scenery_frame_idx) {
            // tile not destroyed yet, advance counter by one
            tile.scenery_frame_idx++;
            tile.tile_id = animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(tile.scenery_frame_idx);
            tile.sprite.setTextureRect(
                sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                            32 * (tile.tile_id / level_tileset.tiles_col),
                            32,32));
            if (!((animated_tiles.at(tile.d_animation)->getAnimationLength() - 2) > tile.scenery_frame_idx)) {
                // the tile was destroyed, create debris
                root->sfxsys->playSFX(SFX_BLOCK_DESTRUCT);
                root->createDebris(animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(animated_tiles.at(tile.d_animation)->getAnimationLength() - 1),tx,ty);
            }
            return true;
        }
    }
    return false;
}

void TileMap::saveInitialSpriteLayer() {
    initial_spr_layer_copy = level_layout.at(spr_layer).tile_layout;
}

void TileMap::setTrigger(unsigned char trigger_id, bool new_state) {
    if (trigger_state[trigger_id] == new_state) {
        return;
    }

    trigger_state[trigger_id] = new_state;

    // go through all tiles and update any that are influenced by this trigger
    for (int tx = 0; tx < level_width; ++tx) {
        for (int ty = 0; ty < level_height; ++ty) {
            LayerTile& tile = level_layout[spr_layer].tile_layout[ty][tx];
            if (tile.dtype == DESTRUCT_TRIGGER) {
                if (tile.extra_byte == trigger_id) {
                if (animated_tiles.at(tile.d_animation)->getAnimationLength() > 1) {
                    tile.scenery_frame_idx = (new_state ? 1 : 0);
                    tile.tile_id = animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(tile.scenery_frame_idx);
                    tile.sprite.setTextureRect(
                        sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                                    32 * (tile.tile_id / level_tileset.tiles_col),
                                    32,32));
                }
            }}
        }
    }
}

bool TileMap::getTrigger(unsigned char trigger_id) {
    return trigger_state[trigger_id];
}

unsigned TileMap::getLevelWidth() {
    return level_width;
}
unsigned TileMap::getLevelHeight() {
    return level_height;
}

void TileMap::setTileEventFlag(int x, int y, PCEvent e) {
    LayerTile& tile = level_layout[spr_layer].tile_layout[y][x];
    quint16 p[8];
    root->game_events->getPositionParams(x*32,y*32,p);

    switch (e) {
        case PC_MODIFIER_ONE_WAY:
            tile.oneway = true;
            break;
        case PC_MODIFIER_VINE:
            tile.vine = true;
            break;
        case PC_SCENERY_DESTRUCT:
            if (tile.animated) {
                tile.dtype = DESTRUCT_WEAPON;
                tile.animated = false;
                tile.d_animation = tile.tile_id;
                tile.tile_id = animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(0);
                tile.scenery_frame_idx = 0;
                tile.sprite.setTextureRect(
                    sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                                32 * (tile.tile_id / level_tileset.tiles_col),
                                32,32));
                tile.extra_byte = p[0];
            }
            break;
        case PC_SCENERY_BUTTSTOMP:
            if (tile.animated) {
                tile.dtype = DESTRUCT_SPECIAL;
                tile.animated = false;
                tile.d_animation = tile.tile_id;
                tile.tile_id = animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(0);
                tile.scenery_frame_idx = 0;
                tile.sprite.setTextureRect(
                    sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                                32 * (tile.tile_id / level_tileset.tiles_col),
                                32,32));
                tile.extra_byte = p[0];
            }
            break;
        case PC_TRIGGER_AREA:
            if (tile.animated) {
                tile.dtype = DESTRUCT_TRIGGER;
                tile.animated = false;
                tile.d_animation = tile.tile_id;
                tile.tile_id = animated_tiles.at(tile.d_animation)->getFrameCanonicalIndex(0);
                tile.scenery_frame_idx = 0;
                tile.sprite.setTextureRect(
                    sf::IntRect(32 * (tile.tile_id % level_tileset.tiles_col),
                                32 * (tile.tile_id / level_tileset.tiles_col),
                                32,32));
                tile.extra_byte = p[0];
            }
            break;
    }
}

bool TileMap::isPosVine(double x, double y) {
    int ax = static_cast<int>(x) / 32;
    int ay = static_cast<int>(y) / 32;
    int rx = static_cast<int>(x) - 32 * ax;
    int ry = static_cast<int>(y) - 32 * ay;

    LayerTile& tile = level_layout[spr_layer].tile_layout[ay][ax];

    bool fx = tile.flipped_x;
    bool fy = tile.flipped_y;
    if (!tile.vine) {
        return false;
    }

    QBitArray mask(1024, false);
    if (tile.animated) {
        if (tile.tile_id < animated_tiles.size()) {
            mask = level_tileset.masks.at(animated_tiles.at(tile.tile_id)->getCurrentTile()->tile_id);
        }
    } else {
        mask = level_tileset.masks.at(tile.tile_id);
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
    for (int i = 0; i < animated_tiles.size(); ++i) {
        animated_tiles.at(i)->advanceTimer();
    }
}

DestructibleDebris::DestructibleDebris(const sf::Texture* tex, sf::RenderTarget* win, int x, int y, unsigned tx, unsigned ty, unsigned short quarter) :
    pos_x(x * 32 + (quarter % 2) * 16), pos_y(y * 32 + (quarter / 2) * 16), h_speed(0), v_speed(0), wint(win) {
    // different quarters of the debris fly out with different x speeds
    int speed_multi[4] = {-2, 2, -1, 1};

    spr = new sf::Sprite(*tex);
    spr->setTextureRect(sf::IntRect(tx * 32 + (quarter % 2) * 16, ty * 32 + (quarter / 2) * 16,16,16));
    h_speed = speed_multi[quarter];
}

DestructibleDebris::~DestructibleDebris() {
    delete spr;
}

void DestructibleDebris::TickUpdate() {
    pos_x += h_speed;
    pos_y += v_speed;
    v_speed += 0.2;
    v_speed = std::min(5.0,v_speed);

    spr->setPosition(pos_x,pos_y);
    wint->draw(*spr);
}

double DestructibleDebris::GetY() {
    return pos_y;
}

