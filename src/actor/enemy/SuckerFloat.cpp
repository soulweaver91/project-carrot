#include "SuckerFloat.h"

#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"
#include "Sucker.h"

EnemySuckerFloat::EnemySuckerFloat(const ActorInstantiationDetails& initData)
    : Enemy(initData), phase(0.0), originX(posX), originY(posY) {
    isGravityAffected = false;

    loadResources("Enemy/SuckerFloat");
    setAnimation(AnimState::IDLE);
}

EnemySuckerFloat::~EnemySuckerFloat() {

}

void EnemySuckerFloat::tickEvent() {
    if (frozenFramesLeft == 0) {
        phase = std::fmod(phase + 0.05, 2 * PI);
        moveInstantly({ originX + 10 * std::cos(phase), originY + 10 * std::sin(phase) }, true, true);

        isFacingLeft = phase < PI / 2 || phase > 3 * PI / 2;
    }

    CommonActor::tickEvent();
}

bool EnemySuckerFloat::perish() {
    if (health == 0) {
        api->addActor(std::make_shared<EnemySucker>(ActorInstantiationDetails(api, { posX, posY }), lastHitDir));
        return CommonActor::perish();
    }

    return false;
}
