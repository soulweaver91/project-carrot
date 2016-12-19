#include "PauseScreen.h"

#include "../CarrotQt5.h"

PauseScreen::PauseScreen(CarrotQt5* root, bool useText) : root(root), useText(useText) {
    // Define the pause text and add vertical bounce animation to it
    pausedText = std::make_unique<BitmapString>(root->getFont(NORMAL), "Pause", FONT_ALIGN_CENTER);
    pausedText->setAnimation(true, 0.0, 6.0, 0.015, 1.25);

    overlay.setFillColor(sf::Color(0, 0, 0, 120));
}

PauseScreen::~PauseScreen() {
}

void PauseScreen::logicTick(const ControlEventList&) {

}

void PauseScreen::renderTick(bool) {
    auto canvas = root->getCanvas();

    // Set up a partially translucent black overlay
    sf::Vector2u viewSize = canvas->getSize();
    overlay.setSize(sf::Vector2f(viewSize));

    canvas->draw(overlay);
    if (useText) {
        pausedText->drawString(canvas, viewSize.x / 2, viewSize.y / 2 - 20);
    }
}

QString PauseScreen::getType() {
    return "PAUSE_SCREEN";
}
