#pragma once

#include <memory>
#include <QList>
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
        InvokableRootFunction remote_function;
        InvokableMenuFunction local_function;
    };
    bool is_local;
    std::unique_ptr<BitmapString> text;
    QVariant param;
};

class MenuScreen {
public:
    MenuScreen(std::shared_ptr<CarrotQt5> root, MenuEntryPoint entry = MENU_MAIN_MENU);
    ~MenuScreen();
    void tickEvent();
    void keyPressEvent(QKeyEvent* event);
private:
    void clearMenuList();
    void setMenuItemSelected(int idx = 0, bool relative = false);
    std::shared_ptr<MenuItem> buildMenuItem(InvokableMenuFunction local_func, QVariant param, const QString& label);
    std::shared_ptr<MenuItem> buildMenuItem(InvokableRootFunction remote_func, QVariant param, const QString& label);

    // valid pointers in menu options
    void loadLevelList(QVariant param);
    void loadEpisodeList(QVariant param);
    void loadMainMenu(QVariant param);
    void placeholderOption(QVariant param);

    std::shared_ptr<CarrotQt5> root;
    sf::Texture glow_a_tex;
    sf::Sprite glow_a;
    sf::Texture glow_b_tex;
    sf::Sprite glow_b[4];
    sf::Texture logo_tex;
    sf::Sprite logo;
    QList<std::shared_ptr<MenuItem>> menu_options;
    std::shared_ptr<MenuItem> cancel_item;
    int selected_item;
    BitmapString attraction_text;
    MenuLayout current_type;

};
