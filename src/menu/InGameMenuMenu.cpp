#include "InGameMenuMenu.h"

#include "../CarrotQt5.h"
#include "EpisodeSelectMenu.h"
#include "ConfirmationMenu.h"

InGameMenuMenu::InGameMenuMenu(CarrotQt5* mainClass) : VerticalItemListMenu(mainClass) {
    menuOptions.append(buildMenuItem([this]() {
        auto delayedRootPtr = root;
        root->pushState<ConfirmationMenu>(true, [delayedRootPtr](bool confirmed) {
            if (confirmed) {
                delayedRootPtr->startMainMenu();
                delayedRootPtr->pushState<EpisodeSelectMenu>(false);
            } else {
                delayedRootPtr->popState();
            }
        }, "Are you sure?");
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
