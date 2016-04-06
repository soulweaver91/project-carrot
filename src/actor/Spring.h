#pragma once
#include <memory>
#include "SolidObject.h"
#include "../struct/Hitbox.h"

class CarrotQt5;

enum SpringType {
    SPRING_RED = 1,
    SPRING_GREEN = 2,
    SPRING_BLUE = 3
};

class Spring : public CommonActor {
public:
    Spring(std::shared_ptr<CarrotQt5> root, double x, double y, SpringType type, byte orient);
    ~Spring();
    Hitbox getHitbox() override;
    sf::Vector2f activate();

private:
    byte orientation;
    byte strength;
    SpringType type;
};
