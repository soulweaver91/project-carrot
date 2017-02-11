#include "DifficultySelectMenu.h"

#include "../CarrotQt5.h"

#include <QDir>
#include <QSettings>

DifficultySelectMenu::DifficultySelectMenu(CarrotQt5* mainClass, std::shared_ptr<NextLevelData> data) : VerticalItemListMenu(mainClass), nextData(data) {
    makeDifficultyOption(DIFFICULTY_EASY, "Easy");
    makeDifficultyOption(DIFFICULTY_NORMAL, "Normal");
    makeDifficultyOption(DIFFICULTY_HARD, "Hard");

    setMenuItemSelected(1);
    attractionText.setText("Select Difficulty");
}

DifficultySelectMenu::~DifficultySelectMenu() {

}

void DifficultySelectMenu::makeDifficultyOption(GameDifficulty difficulty, const QString& label) {
    menuOptions.append(buildMenuItem(
        [this, difficulty]() {
            nextData->difficulty = difficulty;
            root->startGame(*nextData);
        },
        label
    ));
}
