#include "EpisodeSelectMenu.h"

#include "LevelSelectMenu.h"
#include "../CarrotQt5.h"

#include <QDir>
#include <QSettings>

EpisodeSelectMenu::EpisodeSelectMenu(CarrotQt5* mainClass) : VerticalItemListMenu(mainClass) {
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
                    root->startGame(NextLevelData(levelName, epName));
                },
                    level_data.value("Episode/FormalName").toString())
                );
            }
        }
    }
    menuOptions.append(buildMenuItem([this]() {
        root->pushState<LevelSelectMenu>(false);
    }, "Home Cooked Levels"));
    setMenuItemSelected(0);
    attractionText.setText("Select Episode");
}

EpisodeSelectMenu::~EpisodeSelectMenu() {
}
