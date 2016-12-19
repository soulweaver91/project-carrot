#include "MenuScreen.h"
#include <memory>
#include <cmath>
#include <QDir>
#include <QSettings>

MenuScreen::MenuScreen(CarrotQt5* root, MenuEntryPoint entry) : root(root), selectedItemIdx(0),
currentMenuType(MENU_UNKNOWN), attractionText(root->getFont(), "", FONT_ALIGN_RIGHT) {
    cancelItem = buildMenuItem(placeholderOption, "");
    switch (entry) {
        case MENU_MAIN_MENU:
            loadMainMenu();
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

std::shared_ptr<MenuItem> MenuScreen::buildMenuItem(MenuFunctionCallback callback, const QString& label) {
    auto m = std::make_shared<MenuItem>();
    m->callback = callback;
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
            cancelItem->callback();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // Select the currently highlighted option and run its designated function
            menuOptions[selectedItemIdx]->callback();
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

void MenuScreen::processControlUpEvent(const ControlEvent&) {
    // No use at the moment, but defined for the sake of consistency.
}

void MenuScreen::logicTick(const ControlEventList& events) {
    processControlEvents(events);
}

void MenuScreen::renderTick(bool) {
    auto canvas = root->getCanvas();

    unsigned int viewWidth = canvas->getView().getSize().x;
    unsigned int viewHeight = canvas->getView().getSize().y;

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

    attractionText.drawString(canvas, viewWidth - 10, viewHeight - 30);
}

QString MenuScreen::getType() {
    return "MENU_SCREEN";
}

void MenuScreen::processControlEvents(const ControlEventList& events) {
    for (const auto& pair : events.controlDownEvents) {
        processControlDownEvent(pair);
    }

    for (const auto& key : events.controlHeldEvents.keys()) {
        processControlHeldEvent(qMakePair(key, events.controlHeldEvents.value(key)));
    }

    for (const auto& pair : events.controlUpEvents) {
        processControlUpEvent(pair);
    }
}

void MenuScreen::loadLevelList() {
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
                QString levelName = levels.at(i);
                menuOptions.append(buildMenuItem(
                    [this, levelName]() {
                        root->startGame(levelName);
                    },
                    levelData.value("Level/FormalName").toString() + " ~ " + levelName)
                );
            }
        }
    } 
    menuOptions.append(buildMenuItem([this]() {
        loadMainMenu();
    }, "Cancel"));
    setMenuItemSelected(0);
    cancelItem->callback = [this]() {
        loadEpisodeList();
    };
    currentMenuType = MENU_PLAIN_LIST;
    attractionText.setText("Select Level");
}

void MenuScreen::loadEpisodeList() {
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
                QString epName = eps.at(i);
                QString levelName = level_data.value("Episode/FirstLevel").toString();
                menuOptions.append(buildMenuItem(
                    [this, levelName, epName]() {
                        root->startGame(levelName, epName);
                    },
                    level_data.value("Episode/FormalName").toString())
                );
            }
        }
    } 
    menuOptions.append(buildMenuItem([this]() {
        loadLevelList();
    }, "Home Cooked Levels"));
    setMenuItemSelected(0);
    cancelItem->callback = [this]() {
        loadMainMenu();
    };
    currentMenuType = MENU_PLAIN_LIST;
    attractionText.setText("Select Episode");
}

void MenuScreen::loadMainMenu() {
    clearMenuList();
    
    menuOptions.append(buildMenuItem([this]() {
        loadEpisodeList();
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
    currentMenuType = MENU_PLAIN_LIST;
    attractionText.setText("Main Menu");
}

const MenuFunctionCallback MenuScreen::placeholderOption = []() {};
