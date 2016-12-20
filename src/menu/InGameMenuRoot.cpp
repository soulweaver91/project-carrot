#include "InGameMenuRoot.h"

#include "../CarrotQt5.h"
#include "InGameMenuMenu.h"

InGameMenuRoot::InGameMenuRoot(CarrotQt5* root) : root(root) {
    overlay.setFillColor(sf::Color(0, 0, 0, 120));
}

InGameMenuRoot::~InGameMenuRoot() {
}

void InGameMenuRoot::logicTick(const ControlEventList& events) {
    root->pushState<InGameMenuMenu>(false);
}

void InGameMenuRoot::renderTick(bool topmost, bool) {
    auto canvas = root->getCanvas();
    sf::Vector2u viewSize = canvas->getSize();
    overlay.setSize(sf::Vector2f(viewSize));

    canvas->draw(overlay);
}

QString InGameMenuRoot::getType() {
    return "MENU_SCREEN";
}
