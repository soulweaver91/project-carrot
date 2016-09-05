#pragma once
#include <memory>
#include "SolidObject.h"
#include "../struct/Hitbox.h"

class ActorAPI;

enum SpringType {
    SPRING_RED = 1,
    SPRING_GREEN = 2,
    SPRING_BLUE = 3
};

class Spring : public CommonActor {
public:
    Spring(std::shared_ptr<ActorAPI> api, double x, double y, SpringType type, unsigned char orient);
    ~Spring();
    void updateHitbox() override;
    sf::Vector2f activate();

private:
    unsigned char orientation;
    float strength;
};
