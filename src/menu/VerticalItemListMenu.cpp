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

    uint viewWidth = canvas->getView().getSize().x;
    uint viewHeight = canvas->getView().getSize().y;
    int maxItemCount = (std::max(300u, viewHeight) - 300) / 26;
    if (maxItemCount % 2 == 0) {
        maxItemCount++;
    }
    int maxItemCountHalf = (maxItemCount + 1) / 2 - 1;

    if (menuOptions.size() < maxItemCount) {
        for (int i = 0; i < menuOptions.size(); ++i) {
            menuOptions[i]->text->drawString(canvas, viewWidth / 2, 200 + ((viewHeight - 280) / menuOptions.size()) * i);
        }
    } else {
        int j = 0 - std::min(0, selectedItemIdx - maxItemCountHalf);
        for (int i = std::max(0, selectedItemIdx - maxItemCountHalf); i < std::min(menuOptions.size(), selectedItemIdx + (maxItemCountHalf + 1)); ++i, ++j) {
            menuOptions[i]->text->drawString(canvas, viewWidth / 2, 226 + 26 * j);
        }
        if (selectedItemIdx > maxItemCountHalf) {
            BitmapString::drawString(canvas, root->getFont(), "-=...=-", viewWidth / 2 - 40, 200);
        }
        if ((menuOptions.size() - selectedItemIdx - 1) > maxItemCountHalf) {
            BitmapString::drawString(canvas, root->getFont(), "-=...=-", viewWidth / 2 - 40, 200 + (maxItemCount + 1) * 26);
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
