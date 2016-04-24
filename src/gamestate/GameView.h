#pragma once

#include <memory>
#include <SFML/Graphics.hpp>
#include "TimerUser.h"
#include "../struct/CoordinatePair.h"

class CarrotQt5;

class GameView : public TimerUser {
public:
    GameView(std::shared_ptr<CarrotQt5> root, const uint& playerID, const sf::Vector2f& dimensions);
    unsigned getViewWidth();
    unsigned getViewHeight();
    CoordinatePair getViewCenter();
    void centerView(const CoordinatePair& pos);
    void centerView(const double& x, const double& y);
    void setLighting(int target, bool immediate);
    std::weak_ptr<sf::RenderTexture> getCanvas();
    int getLightingLevel();
    void drawView(std::shared_ptr<sf::RenderTarget> windowCanvas);
    void drawUiElements();
    void centerToPlayer();
private:
    void setLightingStep();

    std::shared_ptr<CarrotQt5> root;
    std::shared_ptr<sf::RenderTexture> canvas;
    std::unique_ptr<sf::View> playerView;
    std::unique_ptr<sf::View> uiView;
    std::unique_ptr<sf::Sprite> viewSprite;
    std::shared_ptr<sf::RenderTexture> lightTexture;
    int lightingLevel;
    int targetLightingLevel;
    uint playerID;
};