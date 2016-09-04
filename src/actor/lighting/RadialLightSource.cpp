#include "RadialLightSource.h"
#include "../../graphics/ShaderSource.h"

RadialLightSource::RadialLightSource(double lightRadiusNear, double lightRadiusFar, sf::Color lightColor, const CoordinatePair lightLocation)
    : LightSource(lightLocation, lightColor), lightRadiusNear(lightRadiusNear), lightRadiusFar(lightRadiusFar) {

}

RadialLightSource::~RadialLightSource() {

}

void RadialLightSource::applyLightingToViewTexture(const CoordinatePair& viewPosition, const sf::Sprite& prev, sf::RenderTexture* next) {
    sf::RenderStates states;
    auto shader = ShaderSource::getShader("LightingShader");
    shader->setParameter("color", lightColor);
    shader->setParameter("radiusNear", lightRadiusNear);
    shader->setParameter("radiusFar", lightRadiusFar);
    sf::Vector2f translatedPosition = {
        static_cast<float>(lightLocation.x - viewPosition.x),
        static_cast<float>(prev.getTextureRect().height - (lightLocation.y - viewPosition.y))
    };

    shader->setParameter("center", translatedPosition);
    states.blendMode = sf::BlendNone;
    states.shader = shader.get();
    next->draw(prev, states);
}
