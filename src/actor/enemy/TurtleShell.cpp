#include "TurtleShell.h"

#include "../../CarrotQt5.h"

TurtleShell::TurtleShell(std::shared_ptr<CarrotQt5> root, double x, double y, double initSpeedX,
    double initSpeedY, bool fromEventMap) : CommonActor(root, x, y, fromEventMap) {
    loadResources("Enemy/TurtleShell");
    setAnimation(AnimState::IDLE);

    speedX = initSpeedX;
    speedY = initSpeedY;
    friction = root->gravity / 100;
    elasticity = 0.5;
}

TurtleShell::~TurtleShell() {
}

void TurtleShell::tickEvent() {
    speedX = std::max(std::abs(speedX) - friction, 0.0) * (speedX > 1e-6 ? 1 : -1);
    CommonActor::tickEvent();
}

void TurtleShell::impact(double speed) {
    speedX += speed / 2;
}
