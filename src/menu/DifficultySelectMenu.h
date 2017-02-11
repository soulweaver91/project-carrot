#pragma once

#include "VerticalItemListMenu.h"
#include "../struct/NextLevelData.h"
#include <memory>

class CarrotQt5;

class DifficultySelectMenu : public VerticalItemListMenu {
public:
    DifficultySelectMenu(CarrotQt5* root, std::shared_ptr<NextLevelData> data);
    ~DifficultySelectMenu();

private:
    std::shared_ptr<NextLevelData> nextData;
    void makeDifficultyOption(GameDifficulty difficulty, const QString& label);
};
