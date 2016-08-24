#pragma once

#include <memory>

#include "SolidObject.h"

class ActorAPI;
class Player;

class PowerUpMonitor : public SolidObject {
public:
    PowerUpMonitor(std::shared_ptr<ActorAPI> api, double x, double y, WeaponType type);
    ~PowerUpMonitor();
    void handleCollision(std::shared_ptr<CommonActor> other) override;
    bool perish() override;
    void destroyAndApplyToPlayer(std::shared_ptr<Player> player, int strength);

private:
    WeaponType type;
};