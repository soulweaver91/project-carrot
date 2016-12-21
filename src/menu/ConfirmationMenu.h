#pragma once

#include "MenuScreen.h"

#include <functional>

class CarrotQt5;

class ConfirmationMenu : public MenuScreen {
public:
    ConfirmationMenu(CarrotQt5* root, std::function<void(bool)> callback, const QString& text, const QString& yesLabel = "Yes", const QString& noLabel = "No");
    ~ConfirmationMenu();

    void renderTick(bool topmost, bool topmostAfterPause) override;
    void processControlDownEvent(const ControlEvent& e) override;
private:
    const QString text;
};
