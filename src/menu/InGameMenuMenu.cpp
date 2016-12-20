#include "InGameMenuMenu.h"

#include "../CarrotQt5.h"
#include "EpisodeSelectMenu.h"

InGameMenuMenu::InGameMenuMenu(CarrotQt5* mainClass) : VerticalItemListMenu(mainClass) {
    menuOptions.append(buildMenuItem([this]() {
        root->startMainMenu();
        root->pushState<EpisodeSelectMenu>(false);
    }, "New Game"));
    menuOptions.append(buildMenuItem(placeholderOption, "Load Game"));
    menuOptions.append(buildMenuItem(placeholderOption, "Save Game"));
    menuOptions.append(buildMenuItem(placeholderOption, "Settings"));
    menuOptions.append(buildMenuItem([this]() {
        root->quitFromMainMenu();
    }, "Quit Game"));
    setMenuItemSelected(0);
    cancelItem->callback = [this]() {
        root->popState();
        root->popState();
    };

    attractionText.setText("");
}

InGameMenuMenu::~InGameMenuMenu() {
}
