#include "GameView.h"

#include <cmath>
#include "LevelManager.h"
#include "../actor/Player.h"
#include "../graphics/ShaderSource.h"

GameView::GameView(LevelManager* root, const uint& playerID, const sf::Vector2f& dimensions) 
    : root(root), playerID(playerID) {
    canvas = std::make_shared<sf::RenderTexture>();
    canvas->create(dimensions.x, dimensions.y);

    lightingAuxiliaryCanvas = std::make_shared<sf::RenderTexture>();
    lightingAuxiliaryCanvas->create(dimensions.x, dimensions.y);

    renderAuxiliaryCanvas = std::make_shared<sf::RenderTexture>();
    renderAuxiliaryCanvas->create(dimensions.x, dimensions.y);

    playerView = std::make_unique<sf::View>(sf::FloatRect(0, 0, dimensions.x, dimensions.y));
    uiView = std::make_unique<sf::View>(sf::FloatRect(0, 0, dimensions.x, dimensions.y));
    viewSprite = std::make_unique<sf::Sprite>(canvas->getTexture());
}

void GameView::setLighting(int target, bool immediate) {
    if (target == targetLightingLevel) {
        return;
    }

    targetLightingLevel = target;
    if (immediate) {
        lightingLevel = target;
    } else {
        addTimer(3.0, false, [this]() {
            setLightingStep();
        });
    }
}

std::weak_ptr<sf::RenderTexture> GameView::getCanvas() {
    return canvas;
}

void GameView::setLightingStep() {
    if (targetLightingLevel == lightingLevel) {
        return;
    }
    short dir = (targetLightingLevel < lightingLevel) ? -1 : 1;
    lightingLevel += dir;
    addTimer(3.0, false, [this]() {
        setLightingStep();
    });
}

CoordinatePair GameView::getViewCenter() {
    return CoordinatePair(playerView->getCenter());
}

unsigned GameView::getViewWidth() {
    return playerView->getSize().x;
}

unsigned GameView::getViewHeight() {
    return playerView->getSize().y;
}

int GameView::getLightingLevel() {
    return lightingLevel;
}

void GameView::drawView(sf::RenderTarget* windowCanvas) {
    canvas->display();
    windowCanvas->draw(*viewSprite.get());

    // Clear the drawing surface; we don't want to do this if we emulate the JJ2 behavior
    canvas->clear();
    advanceTimers();
}

void GameView::drawBackgroundEffects(const QVector<LightSource*>& lightSources) {
    lightingAuxiliaryCanvas->clear(sf::Color(255, 255, 255, 0));
    lightingAuxiliaryCanvas->display();

    // Apply each light source onto the lighting canvas
    CoordinatePair viewOffset = {
        playerView->getCenter().x - playerView->getSize().x / 2,
        playerView->getCenter().y - playerView->getSize().y / 2
    };
    renderAuxiliaryCanvas->clear();

    for (auto actor : lightSources) {
        bool updated = actor->applyBackgroundEffectToViewTexture(
            viewOffset,
            sf::Sprite(lightingAuxiliaryCanvas->getTexture()),
            renderAuxiliaryCanvas.get()
        );

        if (updated) {
            renderAuxiliaryCanvas->display();
            std::swap(lightingAuxiliaryCanvas, renderAuxiliaryCanvas);
            renderAuxiliaryCanvas->clear();
        }
    }

    auto effectSprite = sf::Sprite(lightingAuxiliaryCanvas->getTexture());
    effectSprite.setPosition(viewOffset.x, viewOffset.y);
    canvas->draw(effectSprite);
}

void GameView::drawLighting(const QVector<LightSource*>& lightSources) {
    canvas->setView(*uiView.get());
    canvas->display();

    // Set up the initial lighting canvas with black at the opacity of the current lighting level
    lightingAuxiliaryCanvas->clear(sf::Color(0, 0, 0, 255 - lightingLevel * 255.0 / 100.0));
    lightingAuxiliaryCanvas->display();

    // Apply each light source onto the lighting canvas
    CoordinatePair viewOffset = {
        playerView->getCenter().x - playerView->getSize().x / 2,
        playerView->getCenter().y - playerView->getSize().y / 2
    };
    renderAuxiliaryCanvas->clear();

    for (auto actor : lightSources) {
        bool updated = actor->applyLightingToViewTexture(
            viewOffset,
            sf::Sprite(lightingAuxiliaryCanvas->getTexture()),
            renderAuxiliaryCanvas.get()
        );

        if (updated) {
            renderAuxiliaryCanvas->display();
            std::swap(lightingAuxiliaryCanvas, renderAuxiliaryCanvas);
            renderAuxiliaryCanvas->clear();
        }
    }

    // Compose the final view picture sans the UI
    renderAuxiliaryCanvas->clear();
    renderAuxiliaryCanvas->draw(*viewSprite);
    renderAuxiliaryCanvas->draw(sf::Sprite(lightingAuxiliaryCanvas->getTexture()));
    renderAuxiliaryCanvas->display();

    std::swap(canvas, renderAuxiliaryCanvas);
    viewSprite->setTexture(canvas->getTexture());
}

void GameView::drawUiElements() {
    // Draw the player's own overlays
    auto player = root->getPlayer(playerID).lock();
    if (player != nullptr) {
        player->drawUIOverlay();
    }
}

void GameView::centerToPlayer() {
    auto player = root->getPlayer(playerID).lock();
    if (player == nullptr) {
        return;
    }

    player->setToOwnViewCenter();
}

void GameView::setSize(const sf::Vector2f& dimensions) {
    canvas->create(dimensions.x, dimensions.y);
    renderAuxiliaryCanvas->create(dimensions.x, dimensions.y);
    lightingAuxiliaryCanvas->create(dimensions.x, dimensions.y);
    playerView->setCenter(dimensions.x / 2, dimensions.y / 2);
    playerView->setSize(dimensions);
    uiView->setCenter(dimensions.x / 2, dimensions.y / 2);
    uiView->setSize(dimensions);
    viewSprite->setTextureRect(sf::IntRect(0, 0, std::ceil(dimensions.x), std::ceil(dimensions.y)));
}

std::weak_ptr<Player> GameView::getViewPlayer() {
    return root->getPlayer(playerID);
}

void GameView::centerView(const double& x, const double& y) {
    playerView->setCenter(x, y);
    canvas->setView(*playerView.get());
}

void GameView::centerView(const CoordinatePair& pair) {
    playerView->setCenter(pair.x, pair.y);
    canvas->setView(*playerView.get());
}
