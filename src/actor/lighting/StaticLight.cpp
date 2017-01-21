#include "StaticLight.h"

StaticLight::StaticLight(const ActorInstantiationDetails& initData, quint16 alpha) : CommonActor(initData),
    RadialLightSource(100.0, 150.0, sf::Color(0, 0, 0, alpha), initData.coords) {
    isCollidable = false;
}

StaticLight::~StaticLight() {
}
