#include "GemCollectible.h"
#include "../Player.h"

const uint GemCollectible::gemValues[4] = {
    100,
    500,
    1000,
    100
};

const sf::Vector3i GemCollectible::gemColors[4] = {
    { 512, 0, 0 },
    { 0, 512, 0 },
    { 0, 0, 512 },
    { 256, 0, 512 }
};

GemCollectible::GemCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, GemType type, bool fromEventMap)
    : Collectible(root, x, y, fromEventMap), gemType(type) {
    scoreValue = gemValues[(uint)type % 4];
    setFacingDirection();

    AnimationUser::setAnimation("PICKUP_GEM");
    currentAnimation->setColor(gemColors[(uint)type % 4]);
}

void GemCollectible::collect(std::shared_ptr<Player> player) {
    player->addGems(gemType, 1);
    Collectible::collect(player);
}
