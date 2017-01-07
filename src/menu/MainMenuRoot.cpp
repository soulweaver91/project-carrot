#include "MainMenuRoot.h"

#include "../CarrotQt5.h"
#include "MainMenuMenu.h"

MainMenuRoot::MainMenuRoot(CarrotQt5* root) : root(root) {
    mainMenuCircularGlowTexture.loadFromFile("Data/Textures/radialglow.png");
    mainMenuCircularGlowSprite.setTexture(mainMenuCircularGlowTexture);
    mainMenuCircularGlowSprite.setPosition(400, 100);
    mainMenuCircularGlowSprite.setOrigin(313, 313);

    mainMenuConicGlowTexture.loadFromFile("Data/Textures/coneglow.png");
    for (int i = 0; i < 4; ++i) {
        mainMenuConicGlowSprite[i].setTexture(mainMenuConicGlowTexture);
        mainMenuConicGlowSprite[i].setPosition(400, 100);
        mainMenuConicGlowSprite[i].setOrigin(121, 78);
        mainMenuConicGlowSprite[i].setRotation(i * 90);
    }

    projectCarrotLogoTexture.loadFromFile("Data/PCLogo-300px.png");
    projectCarrotLogoSprite.setTexture(projectCarrotLogoTexture);
    projectCarrotLogoSprite.setPosition(400, 10);
    projectCarrotLogoSprite.setOrigin(150, 0);

    root->getSoundSystem()->clearSounds();
    root->getSoundSystem()->unregisterAllSoundListeners();
    root->getSoundSystem()->setMusic("Music/Menu.it");
}

MainMenuRoot::~MainMenuRoot() {
}

void MainMenuRoot::logicTick(const ControlEventList&) {
    root->pushState<MainMenuMenu>(false);
}

void MainMenuRoot::renderTick(bool, bool) {
    auto canvas = root->getCanvas();

    unsigned int viewWidth = canvas->getView().getSize().x;
    unsigned int viewHeight = canvas->getView().getSize().y;

    for (int i = 0; i < 4; ++i) {
        mainMenuConicGlowSprite[i].rotate(0.4);
        mainMenuConicGlowSprite[i].setPosition(sf::Vector2f(viewWidth / 2, 100));
        canvas->draw(mainMenuConicGlowSprite[i]);
    }
    mainMenuCircularGlowSprite.setPosition(sf::Vector2f(viewWidth / 2, 100));
    projectCarrotLogoSprite.setPosition(sf::Vector2f(viewWidth / 2, 10));
    canvas->draw(mainMenuCircularGlowSprite);
    canvas->draw(projectCarrotLogoSprite);


    BitmapString::drawString(canvas, root->getFont(SMALL), CP_VERSION + " v" + CP_VERSION_NUM +
                             " built on " + QString(__DATE__) + " " + QString(__TIME__), 10, viewHeight - 34);
    BitmapString::drawString(canvas, root->getFont(SMALL), "(c) 2013-2017 Project Carrot Team", 10, viewHeight - 22);

}

QString MainMenuRoot::getType() {
    return "MENU_SCREEN";
}
