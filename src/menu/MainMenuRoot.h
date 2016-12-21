#pragma once

#include "../EngineState.h"

class CarrotQt5;

class MainMenuRoot : public EngineState {
public:
    MainMenuRoot(CarrotQt5* root);
    ~MainMenuRoot();

    void logicTick(const ControlEventList& events) override;
    void renderTick(bool topmost, bool topmostAfterPause) override;
    QString getType() override;

private:
    CarrotQt5* root;

    sf::Texture mainMenuCircularGlowTexture;
    sf::Sprite mainMenuCircularGlowSprite;
    sf::Texture mainMenuConicGlowTexture;
    sf::Sprite mainMenuConicGlowSprite[4];
    sf::Texture projectCarrotLogoTexture;
    sf::Sprite projectCarrotLogoSprite;
};
