#include "StaticLight.h"

StaticLight::StaticLight(std::shared_ptr<ActorAPI> api, double x, double y, quint16 alpha) : CommonActor(api, x, y),
    RadialLightSource(100.0, 150.0, sf::Color(0, 0, 0, alpha), { x, y }) {
    isCollidable = false;
}

StaticLight::~StaticLight() {
}
