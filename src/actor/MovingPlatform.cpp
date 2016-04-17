#include "MovingPlatform.h"
#include "../struct/Constants.h"
#include "../CarrotQt5.h"
#include "Player.h"

#define BASE_CYCLE_FRAMES 700

MovingPlatform::MovingPlatform(std::shared_ptr<CarrotQt5> root, double x, double y,
    PlatformType type, quint16 length, qint16 speed, ushort sync, bool swing)
    : SolidObject(root, x, y), type(type), length(length), speed(speed), phase(0.0),
    originX(x), originY(y), isSwing(swing), chainAnimation(this) {
    loadResources("Object/MovingPlatform");
    isGravityAffected = false;
    isOneWay = true;

    phase = fmod(BASE_CYCLE_FRAMES - (root->getFrame() % BASE_CYCLE_FRAMES + sync * 175) * speed,
        BASE_CYCLE_FRAMES);

    auto candidates = findAnimationCandidates((int)(type << 10) + 16);
    if (candidates.size() > 0) {
        chainAnimation.setAnimation(candidates.at(0), (int)(type << 10) + 16);
    }

    setAnimation((int)(type << 10));
}

MovingPlatform::~MovingPlatform() {
}

void MovingPlatform::tickEvent() {
    phase = fmod(phase - speed, BASE_CYCLE_FRAMES);
    if (phase < 0) {
        phase += BASE_CYCLE_FRAMES;
    }

    moveInstantly(getPhasePosition(false, length));

    Hitbox hitbox = getHitbox();
    hitbox.top -= 2;

    auto players = root->getCollidingPlayer(hitbox);
    foreach(auto p, players) {
        p.lock()->setCarryingPlatform(std::dynamic_pointer_cast<MovingPlatform>(shared_from_this()));
    }

    if (type == PlatformType::SPIKE) {
        hitbox.top += 40;
        hitbox.bottom += 40;

        auto players = root->getCollidingPlayer(hitbox);
        foreach(auto p, players) {
            p.lock()->takeDamage(2);
        }
    }

    CommonActor::tickEvent();
}

Hitbox MovingPlatform::getHitbox() {
    Hitbox hitbox = CommonActor::getHitbox();
    hitbox.bottom = hitbox.top + 10;
    return hitbox;
}

void MovingPlatform::drawUpdate() {
    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    for (uint i = 0; i < length; ++i) {
        CoordinatePair pos = getPhasePosition(false, i);
        chainAnimation.setSpritePosition({ (float)pos.x, (float)pos.y });
        chainAnimation.drawCurrentFrame(*canvas);
    }

    CommonActor::drawUpdate();
}

CoordinatePair MovingPlatform::getLocationDelta() {
    CoordinatePair pos1 = getPhasePosition(false, length);
    CoordinatePair pos2 = getPhasePosition(true, length);
    return {pos2.x - pos1.x, pos2.y - pos1.y};
}

CoordinatePair MovingPlatform::getPhasePosition(bool next, uint distance) {
    double effectivePhase = phase - (next ? speed : 0);
    if (isSwing) {
        // Mirror the upper half of the circle motion
        if (effectivePhase > BASE_CYCLE_FRAMES / 2) {
            effectivePhase = BASE_CYCLE_FRAMES - effectivePhase;
        }

        // Blinn-Wyvill approximation to the raised inverted cosine,
        // easing curve with slower ends and faster middle part
        double i = effectivePhase / BASE_CYCLE_FRAMES * 2;
        effectivePhase = ((4.0 / 9.0) * pow(i, 6) - (17.0 / 9.0) * pow(i, 4) + (22.0 / 9.0) * pow(i, 2)) * BASE_CYCLE_FRAMES / 2;
    }

    double multiX = cos(effectivePhase / BASE_CYCLE_FRAMES * 2 * PI);
    double multiY = sin(effectivePhase / BASE_CYCLE_FRAMES * 2 * PI);

    return {
        originX + multiX * distance * 12,
        originY + multiY * distance * 12
    };
}
