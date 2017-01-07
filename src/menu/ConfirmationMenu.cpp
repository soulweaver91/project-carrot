#include "ConfirmationMenu.h"

ConfirmationMenu::ConfirmationMenu(CarrotQt5* mainClass, std::function<void(bool)> callback, const QString& text, const QString& yesLabel, const QString& noLabel) 
    : MenuScreen(mainClass), text(text) {
    menuOptions.append(buildMenuItem([this, callback]() {
        root->popState();
        callback(true);
    }, yesLabel));
    menuOptions.append(buildMenuItem([this, callback]() {
        root->popState();
        callback(false);
    }, noLabel));
    cancelItem = buildMenuItem([this, callback]() {
        root->popState();
        callback(false);
    }, noLabel);
    setMenuItemSelected(0);
}

ConfirmationMenu::~ConfirmationMenu() {
}

void ConfirmationMenu::renderTick(bool, bool) {
    auto canvas = root->getCanvas();
    uint viewWidth = canvas->getView().getSize().x;
    uint viewHeight = canvas->getView().getSize().y;

    BitmapString::drawString(canvas, root->getFont(), text, viewWidth / 2, viewHeight / 2 - 50, FONT_ALIGN_CENTER);
    menuOptions[0]->text->drawString(canvas, viewWidth / 2 - 100, viewHeight / 2 + 50);
    menuOptions[1]->text->drawString(canvas, viewWidth / 2 + 100, viewHeight / 2 + 50);
}

void ConfirmationMenu::processControlDownEvent(const ControlEvent& e) {
    MenuScreen::processControlDownEvent(e);

    switch (e.first.keyboardKey) {
        case Qt::Key_Left:
            setMenuItemSelected(-1, true);
            break;
        case Qt::Key_Right:
            setMenuItemSelected(1, true);
            break;
    }
}
