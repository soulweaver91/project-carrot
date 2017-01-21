#pragma once

#include "../../struct/CoordinatePair.h"

#include <memory>
#include <SFML/Graphics.hpp>

class LightSource {
public:
    LightSource(const CoordinatePair lightLocation, sf::Color lightColor);
    ~LightSource();
    virtual bool applyBackgroundEffectToViewTexture(const CoordinatePair& viewPosition, const sf::Sprite& prev, sf::RenderTexture* next);
    virtual bool applyLightingToViewTexture(const CoordinatePair& viewPosition, const sf::Sprite& prev, sf::RenderTexture* next);

protected:
    CoordinatePair lightLocation;
    sf::Color lightColor;
};