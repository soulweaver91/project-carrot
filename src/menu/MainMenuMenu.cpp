#include "MainMenuMenu.h"

#include "../CarrotQt5.h"
#include "EpisodeSelectMenu.h"

MainMenuMenu::MainMenuMenu(CarrotQt5* mainClass) : VerticalItemListMenu(mainClass) {
    menuOptions.append(buildMenuItem([this]() {
        root->pushState<EpisodeSelectMenu>(false);
    }, "New Game"));
    menuOptions.append(buildMenuItem(placeholderOption, "Load Game"));
    menuOptions.append(buildMenuItem(placeholderOption, "Settings"));
    menuOptions.append(buildMenuItem(placeholderOption, "High Scores"));
    menuOptions.append(buildMenuItem([this]() {
        root->quitFromMainMenu();
    }, "Quit Game"));
    setMenuItemSelected(0);
    cancelItem->callback = [this]() {
        setMenuItemSelected(-1, false);
    };

    attractionText.setText("Main Menu");
}

MainMenuMenu::~MainMenuMenu() {
}
