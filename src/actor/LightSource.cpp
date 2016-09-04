#include "LightSource.h"

LightSource::LightSource(const CoordinatePair lightLocation, sf::Color lightColor)
    : lightLocation(lightLocation), lightColor(lightColor) {
}

LightSource::~LightSource() {
}

void LightSource::applyLightingToViewTexture(const CoordinatePair&, const sf::Sprite&, sf::RenderTexture*) {
    // Implemented by a subclass.
}
