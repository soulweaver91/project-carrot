#include "Spring.h"
#include "../CarrotQt5.h"
#include "../gamestate/TileMap.h"

Spring::Spring(std::shared_ptr<CarrotQt5> root, double x, double y, SpringType type, byte orient) :
    CommonActor(root, x, y, false), orientation(orient), type(type) {
    loadResources("Object/Spring");

    CoordinatePair tileCorner = { (int)(x / 32) * 32.0, (int)(y / 32) * 32.0 };
    if (orientation == 5) {
        // JJ2 horizontal springs held no data about which way they were facing.
        // For compatibility, the level converter sets their orientation to 5, which is interpreted here.
        orientation = !root->getGameTiles().lock()->isTileEmpty(root->calcHitbox(getHitbox(), 16, 0), false) ? 3 : 1;
    }

    int orientationBit = 0;
    switch (orientation) {
        case 0:
            moveInstantly({ tileCorner.x + 16, tileCorner.y + 24 });
            break;
        case 1:
            moveInstantly({ tileCorner.x + 16, tileCorner.y + 16 });
            orientationBit = 1;
            isGravityAffected = false;
            break;
        case 2:
            moveInstantly({ tileCorner.x + 16, tileCorner.y + 8 });
            orientationBit = 2;
            isGravityAffected = false;
            break;
        case 3:
            moveInstantly({ tileCorner.x + 16, tileCorner.y + 16 });
            orientationBit = 1;
            isGravityAffected = false;
            facingLeft = true;
            break;
    }

    setAnimation((AnimStateT)(((int)type << 10) | orientationBit << 12));
    strength = (int)type * 4 + 5;
}

Spring::~Spring() {
}

Hitbox Spring::getHitbox() {
    switch (orientation) {
        case 1:
            return { pos_x - 8, pos_y - 15, pos_x, pos_y + 15 };
            break;
        case 3:
            return { pos_x + 8, pos_y - 15, pos_x + 16, pos_y + 15 };
            break;
        case 0:
        case 2:
        default:
            return { pos_x - 15, pos_y, pos_x + 15, pos_y + 8};
    }
}

sf::Vector2f Spring::activate() {
    setTransition(currentState | 0x200, false);
    switch (orientation) {
        case 0:
            playSound("OBJECT_SPRING_VERTICAL");
            return { 0, -static_cast<float>(strength) };
            break;
        case 2:
            playSound("OBJECT_SPRING_VERTICAL_REVERSED");
            return { 0, static_cast<float>(strength) };
            break;
        case 1:
        case 3:
            playSound("OBJECT_SPRING_HORIZONTAL");
            return { static_cast<float>(strength) * (orientation == 1 ? 1 : -1), 0 };
            break;
        default:
            return { 0, 0 };

    }
}
