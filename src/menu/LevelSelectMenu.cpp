#include "LevelSelectMenu.h"

#include "../CarrotQt5.h"

#include <QDir>
#include <QSettings>

LevelSelectMenu::LevelSelectMenu(CarrotQt5* mainClass) : VerticalItemListMenu(mainClass) {
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
        root->popState();
    }, "Cancel"));
    setMenuItemSelected(0);
    attractionText.setText("Select Level");
}
