#pragma once

#include "../EngineState.h"

#include <memory>
#include <QString>
#include <SFML/Graphics.hpp>

#include "../graphics/BitmapString.h"

class CarrotQt5;

class PauseScreen : public EngineState {
public:
    PauseScreen(CarrotQt5* root, bool useText);
    ~PauseScreen();

    void logicTick(const ControlEventList& events) override;
    void renderTick(bool topmost) override;
    QString getType() override;

private:
    CarrotQt5* root;
    bool useText;

    std::unique_ptr<BitmapString> pausedText;
    sf::RectangleShape overlay;
};