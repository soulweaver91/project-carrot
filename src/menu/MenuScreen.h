#pragma once

#include <memory>
#include <functional>
#include <QVector>
#include <QKeyEvent>
#include <QString>
#include <SFML/Graphics.hpp>

#include "../ModeManager.h"
#include "../CarrotQt5.h"
#include "../graphics/BitmapFont.h"

class MenuScreen;
class CarrotQt5;

typedef std::function<void()> MenuFunctionCallback;

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
    MenuFunctionCallback callback;
    std::unique_ptr<BitmapString> text;
};

class MenuScreen : public ModeManager {
public:
    MenuScreen(CarrotQt5* root, MenuEntryPoint entry = MENU_MAIN_MENU);
    ~MenuScreen();
    void tick(const ControlEventList& events) override;
    void processControlEvents(const ControlEventList& events);
    void processControlDownEvent(const ControlEvent& e);
    void processControlHeldEvent(const ControlEvent& e);
    void processControlUpEvent(const ControlEvent& e);

private:
    void clearMenuList();
    void setMenuItemSelected(int idx = 0, bool relative = false);
    std::shared_ptr<MenuItem> buildMenuItem(MenuFunctionCallback callback, const QString& label);

    void loadLevelList();
    void loadEpisodeList();
    void loadMainMenu();

    CarrotQt5* root;
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

    static const MenuFunctionCallback placeholderOption;
};
