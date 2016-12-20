#pragma once

#include "../EngineState.h"

class CarrotQt5;

class InGameMenuRoot : public EngineState {
public:
    InGameMenuRoot(CarrotQt5* root);
    ~InGameMenuRoot();

    void logicTick(const ControlEventList& events) override;
    void renderTick(bool topmost, bool topmostAfterPause) override;
    QString getType() override;

private:
    CarrotQt5* root;
    sf::RectangleShape overlay;
};
