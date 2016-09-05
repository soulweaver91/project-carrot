#pragma once

#include "../../struct/CoordinatePair.h"
#include "../LightSource.h"

#include <memory>
#include <SFML/Graphics.hpp>

class RadialLightSource : public LightSource {
public:
    RadialLightSource(double lightRadiusNear = 100.0, double lightRadiusFar = 200.0, sf::Color lightColor = sf::Color(0, 0, 0, 0), const CoordinatePair lightLocation = { 0, 0 });
    ~RadialLightSource();
    virtual bool applyLightingToViewTexture(const CoordinatePair& viewPosition, const sf::Sprite& prev, sf::RenderTexture* next) override;

protected:
    double lightRadiusNear;
    double lightRadiusFar;
};