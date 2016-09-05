#pragma once
#include <memory>
#include "SolidObject.h"
#include <SFML/Graphics.hpp>

class ActorAPI;

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
    MovingPlatform(std::shared_ptr<ActorAPI> api, double x, double y, PlatformType type,
        quint16 length, qint16 speed, ushort sync, bool swing);
    ~MovingPlatform();
    void tickEvent() override;
    void updateHitbox() override;
    void drawUpdate(std::shared_ptr<GameView>& view) override;
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
    std::shared_ptr<AnimationInstance> chainAnimation;
};
