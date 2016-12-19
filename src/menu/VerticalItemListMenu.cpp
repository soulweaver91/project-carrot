#include "VerticalItemListMenu.h"

VerticalItemListMenu::VerticalItemListMenu(CarrotQt5* root) : MenuScreen(root) {

}

VerticalItemListMenu::~VerticalItemListMenu() {
}

void VerticalItemListMenu::renderTick(bool topmost, bool topmostAfterPause) {
    MenuScreen::renderTick(topmost, topmostAfterPause);

    if (!topmost && !topmostAfterPause) {
        return;
    }

    auto canvas = root->getCanvas();

    unsigned int viewWidth = canvas->getView().getSize().x;
    unsigned int viewHeight = canvas->getView().getSize().y;

    if (menuOptions.size() < 10) {
        for (int i = 0; i < menuOptions.size(); ++i) {
            menuOptions[i]->text->drawString(canvas, viewWidth / 2, 200 + ((viewHeight - 280) / menuOptions.size()) * i);
        }
    } else {
        int j = 0 - std::min(0, selectedItemIdx - 5);
        for (int i = std::max(0, selectedItemIdx - 5); i < std::min(menuOptions.size(), selectedItemIdx + 6); ++i, ++j) {
            menuOptions[i]->text->drawString(canvas, viewWidth / 2, 226 + 26 * j);
        }
        if (selectedItemIdx > 5) {
            BitmapString::drawString(canvas, root->getFont(), "-=...=-", viewWidth / 2 - 40, 200);
        }
        if ((menuOptions.size() - selectedItemIdx - 1) > 5) {
            BitmapString::drawString(canvas, root->getFont(), "-=...=-", viewWidth / 2 - 40, 512);
        }
    }
}

void VerticalItemListMenu::processControlDownEvent(const ControlEvent& e) {
    MenuScreen::processControlDownEvent(e);

    switch (e.first.keyboardKey) {
        case Qt::Key_Up:
            // Move selection up
            setMenuItemSelected(-1, true);
            break;
        case Qt::Key_Down:
            // Move selection down
            setMenuItemSelected(1, true);
            break;
    }
}
