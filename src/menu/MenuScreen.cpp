#include <memory>
#include "MenuScreen.h"
#include <QDir>
#include <QSettings>

MenuScreen::MenuScreen(std::shared_ptr<CarrotQt5> root, MenuEntryPoint entry) : root(root), selectedItemIdx(0),
    currentMenuType(MENU_PLAIN_LIST), attractionText(root->getFont(), "", FONT_ALIGN_RIGHT) {
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

    cancelItem = buildMenuItem(&MenuScreen::placeholderOption, QVariant(""), "");
    switch (entry) {
        case MENU_MAIN_MENU:
            loadMainMenu(QVariant(""));
            break;
        case MENU_PAUSE_MENU:
            break;
    }

}

MenuScreen::~MenuScreen() {

}

void MenuScreen::clearMenuList() {
    menuOptions.clear();
    selectedItemIdx = 0;
}

std::shared_ptr<MenuItem> MenuScreen::buildMenuItem(InvokableMenuFunction localFunction, QVariant param, const QString& label) {
    auto m = std::make_shared<MenuItem>();
    m->isLocal = true;
    m->localFunction = localFunction;
    m->param = param;
    m->text = std::make_unique<BitmapString>(root->getFont(), label, FONT_ALIGN_CENTER);
    return m;
}

std::shared_ptr<MenuItem> MenuScreen::buildMenuItem(InvokableRootFunction remoteFunction, QVariant param, const QString& label) {
    auto m = std::make_shared<MenuItem>();
    m->isLocal = false;
    m->remoteFunction = remoteFunction;
    m->param = param;
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
            it = cancelItem;
            if (it->isLocal) {
                (this->*(it->localFunction))(it->param);
            } else {
                root->invokeFunction(it->remoteFunction, it->param);
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // Select the currently highlighted option and run its designated function
            it = menuOptions[selectedItemIdx];
            if (it->isLocal) {
                (this->*(it->localFunction))(it->param);
            } else {
                root->invokeFunction(it->remoteFunction, it->param);
            }
            break;
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

void MenuScreen::processControlHeldEvent(const ControlEvent& e) {
    if (e.second.heldDuration > 20 && e.second.heldDuration % 5 == 0) {
        processControlDownEvent(e);
    }
}

void MenuScreen::processControlUpEvent(const ControlEvent& e) {
    // No use at the moment, but defined for the sake of consistency.
}

void MenuScreen::tickEvent() {
    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

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
    

    BitmapString::drawString(canvas, root->getFont(), CP_VERSION + " v" + CP_VERSION_NUM, 10, viewHeight - 30);
    attractionText.drawString(canvas, viewWidth - 10, viewHeight - 30);

    switch (currentMenuType) {
        case MENU_PLAIN_LIST:
            if (menuOptions.size() < 10) {
                for (int i = 0; i < menuOptions.size(); ++i) {
                    menuOptions[i]->text->drawString(canvas, viewWidth / 2,200 + ((viewHeight - 280) / menuOptions.size()) * i);
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
            break;
        default:
            // ?
            break;
    }
}

void MenuScreen::loadLevelList(QVariant param) {
    clearMenuList();
    QDir levelDir("Levels");
    if (levelDir.exists()) {
        QStringList levels = levelDir.entryList();
        for (int i = 0; i < levels.size(); ++i) {
            if (levels.at(i) == "." || levels.at(i) == "..") {
                continue;
            }
            if (QDir(levelDir.absoluteFilePath(levels.at(i))).exists()) {
                QSettings levelData(levelDir.absoluteFilePath(levels.at(i) + "/config.ini"), QSettings::Format::IniFormat);
                menuOptions.append(buildMenuItem(
                    &CarrotQt5::startGame,
                    QVariant(levelDir.absoluteFilePath(levels.at(i))),
                    levelData.value("Level/FormalName").toString() + " ~ " + levels.at(i))
                );
            }
        }
    } 
    menuOptions.append(buildMenuItem(&MenuScreen::loadMainMenu, QVariant(""), "Cancel"));
    setMenuItemSelected(0);
    cancelItem->localFunction = &MenuScreen::loadMainMenu;
    currentMenuType = MENU_PLAIN_LIST;
    attractionText.setText("Select Level");
}

void MenuScreen::loadEpisodeList(QVariant param) {
    clearMenuList();
    QDir episodeDir("Episodes");
    if (episodeDir.exists()) {
        QStringList eps = episodeDir.entryList();
        for (int i = 0; i < eps.size(); ++i) {
            if (eps.at(i) == "." || eps.at(i) == "..") {
                continue;
            }
            if (QDir(episodeDir.absoluteFilePath(eps.at(i))).exists()) {
                QSettings level_data(episodeDir.absoluteFilePath(eps.at(i) + "/config.ini"), QSettings::Format::IniFormat);
                menuOptions.append(buildMenuItem(
                    &CarrotQt5::startGame,
                    QVariant(episodeDir.absoluteFilePath(eps.at(i)) + "/" + level_data.value("Episode/FirstLevel").toString()),
                    level_data.value("Episode/FormalName").toString())
                );
            }
        }
    } 
    menuOptions.append(buildMenuItem(&MenuScreen::loadLevelList, QVariant(""), "Home Cooked Levels"));
    setMenuItemSelected(0);
    cancelItem->localFunction = &MenuScreen::loadMainMenu;
    currentMenuType = MENU_PLAIN_LIST;
    attractionText.setText("Select Episode");
}

void MenuScreen::loadMainMenu(QVariant param) {
    clearMenuList();
    
    menuOptions.append(buildMenuItem(&MenuScreen::loadEpisodeList, QVariant(""), "New Game"));
    menuOptions.append(buildMenuItem(&MenuScreen::placeholderOption, QVariant(""), "Load Game"));
    menuOptions.append(buildMenuItem(&MenuScreen::placeholderOption, QVariant(""), "Settings"));
    menuOptions.append(buildMenuItem(&MenuScreen::placeholderOption, QVariant(""), "High Scores"));
    menuOptions.append(buildMenuItem(&CarrotQt5::quitFromMainMenu, QVariant(""), "Quit Game"));
    setMenuItemSelected(0);
    cancelItem->localFunction = &MenuScreen::placeholderOption;
    currentMenuType = MENU_PLAIN_LIST;
    attractionText.setText("Main Menu");
}

void MenuScreen::placeholderOption(QVariant param) {
    // do nothing
}
