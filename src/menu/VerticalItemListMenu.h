#pragma once

#include "MenuScreen.h"
#include "../struct/Controls.h"

class CarrotQt5;

class VerticalItemListMenu : public MenuScreen {
public:
    VerticalItemListMenu(CarrotQt5* root);
    ~VerticalItemListMenu();
    void renderTick(bool topmost, bool topmostAfterPause) override;

protected:
    void processControlDownEvent(const ControlEvent& e) override;
};
