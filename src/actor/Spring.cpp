#include "Spring.h"
#include "../CarrotQt5.h"
#include "../gamestate/TileMap.h"

Spring::Spring(std::shared_ptr<CarrotQt5> root, double x, double y, SpringType type, unsigned char orient) :
    CommonActor(root, x, y, false), orientation(orient) {
    loadResources("Object/Spring");

    CoordinatePair tileCorner = { (int)(x / 32) * 32.0, (int)(y / 32) * 32.0 };
    if (orientation == 5) {
        // JJ2 horizontal springs held no data about which way they were facing.
        // For compatibility, the level converter sets their orientation to 5, which is interpreted here.
        orientation = !root->getGameTiles().lock()->isTileEmpty(getHitbox().add(16, 0), false) ? 3 : 1;
    }

    int orientationBit = 0;
    switch (orientation) {
        case 0:
            moveInstantly({ tileCorner.x + 16, tileCorner.y + 8 });
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
            isFacingLeft = true;
            break;
    }

    setAnimation((AnimStateT)(((int)type << 10) | orientationBit << 12));
    if (orientation % 2 == 1) {
        // Horizontal springs all seem to have the same strength.
        // This constant strength gives about the correct amount of horizontal push.
        strength = 9.5;
    } else {
        // Vertical springs should work as follows:
        // Red spring lifts the player 9 tiles, green 14, and blue 19.
        // Vertical strength currently works differently from horizontal, that explains
        // the otherwise inexplicable difference of scale between the two types.
        switch (type) {
            case SpringType::SPRING_RED:
                strength = 1.25;
                break;
            case SpringType::SPRING_GREEN:
                strength = 1.50;
                break;
            case SpringType::SPRING_BLUE:
                strength = 1.65;
                break;
        }
    }
}

Spring::~Spring() {
}

Hitbox Spring::getHitbox() {
    switch (orientation) {
        case 1:
            return { posX - 8, posY - 15, posX, posY + 15 };
            break;
        case 3:
            return { posX, posY - 15, posX + 8, posY + 15 };
            break;
        case 0:
        case 2:
        default:
            return { posX - 15, posY, posX + 15, posY + 8};
    }
}

sf::Vector2f Spring::activate() {
    if (frozenFramesLeft > 0) {
        return { 0, 0 };
    }

    setTransition(currentAnimation->getAnimationState() | 0x200, false);
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
