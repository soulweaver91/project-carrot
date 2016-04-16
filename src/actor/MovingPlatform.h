#pragma once
#include <memory>
#include "SolidObject.h"
#include <SFML/Graphics.hpp>

class CarrotQt5;

enum PlatformType {
    CARROTUS_FRUIT = 1,
    BALL = 2,
    CARROTUS_GRASS = 3,
    LAB = 4,
    SONIC = 5,
    SPIKE = 6
};

class MovingPlatform : public SolidObject {
public:
    MovingPlatform(std::shared_ptr<CarrotQt5> root, double x, double y, PlatformType type, 
        quint16 length, qint16 speed, ushort sync, bool swing);
    ~MovingPlatform();
    void tickEvent() override;
    Hitbox getHitbox() override;
    void drawUpdate() override;
    CoordinatePair getLocationDelta();

private:
    CoordinatePair getPhasePosition(bool next, uint distance);
    PlatformType type;
    qint16 speed;
    quint16 length;
    double phase;
    double originX;
    double originY;
    bool isSwing;
    AnimationInstance chainAnimation;
};
