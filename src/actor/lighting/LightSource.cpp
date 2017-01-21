#include "LightSource.h"

LightSource::LightSource(const CoordinatePair lightLocation, sf::Color lightColor)
    : lightLocation(lightLocation), lightColor(lightColor) {
}

LightSource::~LightSource() {
}

bool LightSource::applyBackgroundEffectToViewTexture(const CoordinatePair&, const sf::Sprite&, sf::RenderTexture*) {
    // Implemented by a subclass.
    // The return value indicates if the actor did anything; if not, the calling view can opt out to not clear surfaces.
    return false;
}

bool LightSource::applyLightingToViewTexture(const CoordinatePair&, const sf::Sprite&, sf::RenderTexture*) {
    // Implemented by a subclass.
    // The return value indicates if the actor did anything; if not, the calling view can opt out to not clear surfaces.
    return false;
}
