#include "MenuScreen.h"
#include <memory>
#include <cmath>

MenuScreen::MenuScreen(CarrotQt5* mainClass) : root(mainClass), selectedItemIdx(0), attractionText(root->getFont(), "", FONT_ALIGN_RIGHT) {
    cancelItem = buildMenuItem([this]() {
        root->popState();
    }, "");
}

MenuScreen::~MenuScreen() {

}

void MenuScreen::clearMenuList() {
    menuOptions.clear();
    selectedItemIdx = 0;
}

std::shared_ptr<MenuItem> MenuScreen::buildMenuItem(MenuFunctionCallback callback, const QString& label) {
    auto m = std::make_shared<MenuItem>();
    m->callback = callback;
    m->text = std::make_unique<BitmapString>(root->getFont(), label, FONT_ALIGN_CENTER);
    return m;
}

void MenuScreen::setMenuItemSelected(int idx, bool relative) {
    if (menuOptions.size() == 0) {
        // this should not happen
        return;
    }
    
    menuOptions[selectedItemIdx]->text->setAnimation(false);
    menuOptions[selectedItemIdx]->text->setColoured(false);

    int new_idx = (relative ? selectedItemIdx : 0) + idx;
    while (new_idx < 0) {
        new_idx += menuOptions.size();
    }
    while (new_idx >= menuOptions.size()) {
        new_idx -= menuOptions.size();
    }
    selectedItemIdx = new_idx;
    menuOptions[selectedItemIdx]->text->setAnimation(true, 4, 4, 0.05, 0.3);
    menuOptions[selectedItemIdx]->text->setColoured(true);
}

void MenuScreen::processControlDownEvent(const ControlEvent& e) {
    std::shared_ptr<MenuItem> it;
    switch (e.first.keyboardKey) {
        case Qt::Key_Escape:
            cancelItem->callback();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // Select the currently highlighted option and run its designated function
            menuOptions[selectedItemIdx]->callback();
            break;
    }
}

void MenuScreen::processControlHeldEvent(const ControlEvent& e) {
    if (e.second.heldDuration > 20 && e.second.heldDuration % 5 == 0) {
        processControlDownEvent(e);
    }
}

void MenuScreen::processControlUpEvent(const ControlEvent&) {
    // No use at the moment, but defined for the sake of consistency.
}

void MenuScreen::logicTick(const ControlEventList& events) {
    processControlEvents(events);
}

void MenuScreen::renderTick(bool topmost, bool topmostAfterPause) {
    if (!topmost && !topmostAfterPause) {
        return;
    }

    auto canvas = root->getCanvas();

    unsigned int viewWidth = canvas->getView().getSize().x;
    unsigned int viewHeight = canvas->getView().getSize().y;

    attractionText.drawString(canvas, viewWidth - 10, viewHeight - 30);
}

QString MenuScreen::getType() {
    return "MENU_SCREEN";
}

void MenuScreen::processControlEvents(const ControlEventList& events) {
    for (const auto& pair : events.controlDownEvents) {
        processControlDownEvent(pair);
    }

    for (const auto& key : events.controlHeldEvents.keys()) {
        processControlHeldEvent(qMakePair(key, events.controlHeldEvents.value(key)));
    }

    for (const auto& pair : events.controlUpEvents) {
        processControlUpEvent(pair);
    }
}

const MenuFunctionCallback MenuScreen::placeholderOption = []() {};

