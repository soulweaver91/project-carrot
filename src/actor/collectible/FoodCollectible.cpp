#include "FoodCollectible.h"
#include "../Player.h"

FoodCollectible::FoodCollectible(std::shared_ptr<ActorAPI> api, double x, double y, PCEvent type, bool fromEventMap)
    : Collectible(api, x, y, fromEventMap) {
    scoreValue = 50;
    switch (type) {
        case PC_FOOD_APPLE:       AnimationUser::setAnimation("PICKUP_FOOD_APPLE"); break;
        case PC_FOOD_BANANA:      AnimationUser::setAnimation("PICKUP_FOOD_BANANA"); break;
        case PC_FOOD_CHERRY:      AnimationUser::setAnimation("PICKUP_FOOD_CHERRY"); break;
        case PC_FOOD_ORANGE:      AnimationUser::setAnimation("PICKUP_FOOD_ORANGE"); break;
        case PC_FOOD_PEAR:        AnimationUser::setAnimation("PICKUP_FOOD_PEAR"); break;
        case PC_FOOD_PRETZEL:     AnimationUser::setAnimation("PICKUP_FOOD_PRETZEL"); break;
        case PC_FOOD_STRAWBERRY:  AnimationUser::setAnimation("PICKUP_FOOD_STRAWBERRY"); break;
        case PC_FOOD_LEMON:       AnimationUser::setAnimation("PICKUP_FOOD_LEMON"); break;
        case PC_FOOD_LIME:        AnimationUser::setAnimation("PICKUP_FOOD_LIME"); break;
        case PC_FOOD_THING:       AnimationUser::setAnimation("PICKUP_FOOD_THING"); break;
        case PC_FOOD_WATERMELON:  AnimationUser::setAnimation("PICKUP_FOOD_WATERMELON"); break;
        case PC_FOOD_PEACH:       AnimationUser::setAnimation("PICKUP_FOOD_PEACH"); break;
        case PC_FOOD_GRAPES:      AnimationUser::setAnimation("PICKUP_FOOD_GRAPES"); break;
        case PC_FOOD_LETTUCE:     AnimationUser::setAnimation("PICKUP_FOOD_LETTUCE"); break;
        case PC_FOOD_EGGPLANT:    AnimationUser::setAnimation("PICKUP_FOOD_EGGPLANT"); break;
        case PC_FOOD_CUCUMBER:    AnimationUser::setAnimation("PICKUP_FOOD_CUCUMBER"); break;
        case PC_FOOD_PEPSI:       AnimationUser::setAnimation("PICKUP_FOOD_SODA"); break;
        case PC_FOOD_COKE:        AnimationUser::setAnimation("PICKUP_FOOD_COLA"); break;
        case PC_FOOD_MILK:        AnimationUser::setAnimation("PICKUP_FOOD_MILK"); break;
        case PC_FOOD_PIE:         AnimationUser::setAnimation("PICKUP_FOOD_PIE"); break;
        case PC_FOOD_CAKE:        AnimationUser::setAnimation("PICKUP_FOOD_CAKE"); break;
        case PC_FOOD_DONUT:       AnimationUser::setAnimation("PICKUP_FOOD_DONUT"); break;
        case PC_FOOD_CUPCAKE:     AnimationUser::setAnimation("PICKUP_FOOD_CUPCAKE"); break;
        case PC_FOOD_CHIPS:       AnimationUser::setAnimation("PICKUP_FOOD_CHIPS"); break;
        case PC_FOOD_CANDY:       AnimationUser::setAnimation("PICKUP_FOOD_CANDY"); break;
        case PC_FOOD_CHOCOLATE:   AnimationUser::setAnimation("PICKUP_FOOD_CHOCOLATE"); break;
        case PC_FOOD_ICE_CREAM:   AnimationUser::setAnimation("PICKUP_FOOD_ICECREAM"); break;
        case PC_FOOD_BURGER:      AnimationUser::setAnimation("PICKUP_FOOD_BURGER"); break;
        case PC_FOOD_PIZZA:       AnimationUser::setAnimation("PICKUP_FOOD_PIZZA"); break;
        case PC_FOOD_FRIES:       AnimationUser::setAnimation("PICKUP_FOOD_FRIES"); break;
        case PC_FOOD_CHICKEN_LEG: AnimationUser::setAnimation("PICKUP_FOOD_CHICKEN"); break;
        case PC_FOOD_SANDWICH:    AnimationUser::setAnimation("PICKUP_FOOD_SANDWICH"); break;
        case PC_FOOD_TACO:        AnimationUser::setAnimation("PICKUP_FOOD_TACO"); break;
        case PC_FOOD_HOT_DOG:     AnimationUser::setAnimation("PICKUP_FOOD_HOTDOG"); break;
        case PC_FOOD_HAM:         AnimationUser::setAnimation("PICKUP_FOOD_HAM"); break;
        case PC_FOOD_CHEESE:      AnimationUser::setAnimation("PICKUP_FOOD_CHEESE"); break;
        default:
            break;
    }

    switch (type) {
        case PC_FOOD_PEPSI:
        case PC_FOOD_COKE:
        case PC_FOOD_MILK:
            isDrinkable = true;
            break;
        default:
            isDrinkable = false;
            break;
    }

    setFacingDirection();
}

void FoodCollectible::collect(std::shared_ptr<Player> player) {
    player->consumeFood(isDrinkable);
    Collectible::collect(player);
}
