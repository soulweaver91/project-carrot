#pragma once

#include <memory>
#include <QVector>
#include <QVariant>
#include <QKeyEvent>
#include <QString>
#include <SFML/Graphics.hpp>

#include "../CarrotQt5.h"
#include "../graphics/BitmapFont.h"

class MenuScreen;
class CarrotQt5;

typedef void (MenuScreen::*InvokableMenuFunction)(QVariant);

enum MenuLayout {
    MENU_UNKNOWN,
    MENU_PLAIN_LIST,
    MENU_GRAPHIC_ON_LEFT_AND_LIST_ON_RIGHT,
    MENU_GRAPHIC_ON_RIGHT_AND_LIST_ON_LEFT,
    MENU_GRAPHIC_ONLY
};

enum MenuEntryPoint {
    MENU_MAIN_MENU,
    MENU_PAUSE_MENU
};

struct MenuItem {
    union {
        InvokableRootFunction remoteFunction;
        InvokableMenuFunction localFunction;
    };
    bool isLocal;
    std::unique_ptr<BitmapString> text;
    QVariant param;
};

class MenuScreen {
public:
    MenuScreen(std::shared_ptr<CarrotQt5> root, MenuEntryPoint entry = MENU_MAIN_MENU);
    ~MenuScreen();
    void tickEvent();
    void processControlDownEvent(const ControlEvent& e);
    void processControlHeldEvent(const ControlEvent& e);
    void processControlUpEvent(const ControlEvent& e);

private:
    void clearMenuList();
    void setMenuItemSelected(int idx = 0, bool relative = false);
    std::shared_ptr<MenuItem> buildMenuItem(InvokableMenuFunction localFunction, QVariant param, const QString& label);
    std::shared_ptr<MenuItem> buildMenuItem(InvokableRootFunction remoteFunction, QVariant param, const QString& label);

    // valid pointers in menu options
    void loadLevelList(QVariant param);
    void loadEpisodeList(QVariant param);
    void loadMainMenu(QVariant param);
    void placeholderOption(QVariant param);

    std::shared_ptr<CarrotQt5> root;
    sf::Texture mainMenuCircularGlowTexture;
    sf::Sprite mainMenuCircularGlowSprite;
    sf::Texture mainMenuConicGlowTexture;
    sf::Sprite mainMenuConicGlowSprite[4];
    sf::Texture projectCarrotLogoTexture;
    sf::Sprite projectCarrotLogoSprite;
    QVector<std::shared_ptr<MenuItem>> menuOptions;
    std::shared_ptr<MenuItem> cancelItem;
    int selectedItemIdx;
    BitmapString attractionText;
    MenuLayout currentMenuType;

};
