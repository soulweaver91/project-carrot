#include "GameView.h"
#include "../CarrotQt5.h"
#include "../actor/Player.h"
#include "../graphics/ShaderSource.h"

GameView::GameView(std::shared_ptr<CarrotQt5> root, const uint& playerID, const sf::Vector2f& dimensions) 
    : playerID(playerID), root(root) {
    canvas = std::make_shared<sf::RenderTexture>();
    canvas->create(dimensions.x, dimensions.y);
    renderAuxiliaryCanvas = std::make_shared<sf::RenderTexture>();
    renderAuxiliaryCanvas->create(dimensions.x, dimensions.y);
    playerView = std::make_unique<sf::View>(sf::FloatRect(0, 0, dimensions.x, dimensions.y));
    uiView = std::make_unique<sf::View>(sf::FloatRect(0, 0, dimensions.x, dimensions.y));
    viewSprite = std::make_unique<sf::Sprite>(canvas->getTexture());
}

void GameView::setLighting(int target, bool immediate) {
    targetLightingLevel = target;
    if (target == lightingLevel) {
        return;
    }
    if (immediate) {
        lightingLevel = target;
    } else {
        addTimer(1.0, false, static_cast<TimerCallbackFunc>(&GameView::setLightingStep));
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
    addTimer(1.0, false, static_cast<TimerCallbackFunc>(&GameView::setLightingStep));
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

void GameView::drawView(std::shared_ptr<sf::RenderTarget> windowCanvas) {
    canvas->display();
    windowCanvas->draw(*viewSprite.get());

    // Clear the drawing surface; we don't want to do this if we emulate the JJ2 behavior
    canvas->clear();
}

void GameView::drawUiElements() {
    auto player = root->getPlayer(playerID).lock();
    if (player == nullptr) {
        return;
    }
    canvas->setView(*uiView.get());

    sf::RenderStates states;
    auto shader = ShaderSource::getShader("LightingShader");
    if (shader != nullptr) {
        shader->setParameter("color", sf::Color::Black);
        shader->setParameter("lightingLevel", lightingLevel / 100.0);

        shader->setParameter("center", sf::Vector2f(
            player->getPosition().x - playerView->getCenter().x + playerView->getSize().x / 2,
            // Reverse the vertical coordinate, as the frame buffer is "upside down"
            playerView->getSize().y - ((player->getPosition().y - 15) - playerView->getCenter().y + playerView->getSize().y / 2)
        ));
        states.shader = shader.get();
        renderAuxiliaryCanvas->clear();
        renderAuxiliaryCanvas->draw(*viewSprite.get(), states);
        renderAuxiliaryCanvas->display();
        std::swap(canvas, renderAuxiliaryCanvas);
        viewSprite->setTexture(canvas->getTexture());
    }

    // Draw the character icon; managed by the player object
    player->drawUIOverlay();

}

void GameView::centerToPlayer() {
    auto player = root->getPlayer(playerID).lock();
    if (player == nullptr) {
        return;
    }

    player->setToViewCenter();
}

void GameView::setSize(const sf::Vector2f& dimensions) {
    canvas->create(dimensions.x, dimensions.y);
    renderAuxiliaryCanvas->create(dimensions.x, dimensions.y);
    playerView->setCenter(dimensions.x / 2, dimensions.y / 2);
    playerView->setSize(dimensions);
    uiView->setCenter(dimensions.x / 2, dimensions.y / 2);
    uiView->setSize(dimensions);
    viewSprite->setTextureRect(sf::IntRect(0, 0, std::ceil(dimensions.x), std::ceil(dimensions.y)));
}

void GameView::centerView(const double& x, const double& y) {
    playerView->setCenter(x, y);
    canvas->setView(*playerView.get());
}

void GameView::centerView(const CoordinatePair& pair) {
    playerView->setCenter(pair.x, pair.y);
    canvas->setView(*playerView.get());
}
