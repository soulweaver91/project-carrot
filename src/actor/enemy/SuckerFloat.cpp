#include "SuckerFloat.h"

#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"
#include "Sucker.h"

EnemySuckerFloat::EnemySuckerFloat(std::shared_ptr<ActorAPI> api, double x, double y)
    : Enemy(api, x, y), phase(0.0), originX(x), originY(y) {
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
        api->addActor(std::make_shared<EnemySucker>(api, posX, posY, lastHitDir));
        return CommonActor::perish();
    }

    return false;
}
