#include "SuckerFloat.h"

#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"
#include "Sucker.h"

EnemySuckerFloat::EnemySuckerFloat(const ActorInstantiationDetails& initData)
    : Enemy(initData), phase(0.0), originPos(pos) {
    isGravityAffected = false;

    loadResources("Enemy/SuckerFloat");
    setAnimation(AnimState::IDLE);
}

EnemySuckerFloat::~EnemySuckerFloat() {

}

void EnemySuckerFloat::tickEvent() {
    if (frozenFramesLeft == 0) {
        phase = std::fmod(phase + 0.05, 2 * PI);
        moveInstantly(originPos + CoordinatePair(CIRCLING_RADIUS * std::cos(phase), CIRCLING_RADIUS * std::sin(phase)), true, true);

        isFacingLeft = phase < PI / 2 || phase > 3 * PI / 2;
    }

    CommonActor::tickEvent();
}

bool EnemySuckerFloat::perish() {
    if (health == 0) {
        api->addActor(std::make_shared<EnemySucker>(ActorInstantiationDetails(api, originPos), lastHitDir));
        return CommonActor::perish();
    }

    return false;
}

const double EnemySuckerFloat::CIRCLING_RADIUS = 10;