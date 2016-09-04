#pragma once

#include <QVector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "TimerUser.h"
#include "../struct/CoordinatePair.h"

class LevelManager;
class Player;
class LightSource;

class GameView : public TimerUser {
public:
    GameView(LevelManager* root, const uint& playerID, const sf::Vector2f& dimensions);
    unsigned getViewWidth();
    unsigned getViewHeight();
    CoordinatePair getViewCenter();
    void centerView(const CoordinatePair& pos);
    void centerView(const double& x, const double& y);
    void setLighting(int target, bool immediate);
    std::weak_ptr<sf::RenderTexture> getCanvas();
    int getLightingLevel();
    void drawView(sf::RenderTarget* windowCanvas);
    void drawUiElements(const QVector<LightSource*>& lightingSources);
    void centerToPlayer();
    void setSize(const sf::Vector2f& dimensions);
    std::weak_ptr<Player> getViewPlayer();
private:
    void setLightingStep();

    LevelManager* root;
    std::shared_ptr<sf::RenderTexture> canvas;
    std::shared_ptr<sf::RenderTexture> lightingAuxiliaryCanvas;
    std::shared_ptr<sf::RenderTexture> renderAuxiliaryCanvas;
    std::unique_ptr<sf::View> playerView;
    std::unique_ptr<sf::View> uiView;
    std::unique_ptr<sf::Sprite> viewSprite;
    int lightingLevel;
    int targetLightingLevel;
    uint playerID;
};