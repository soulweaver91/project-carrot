#pragma once

#include <memory>

#include "VerticalItemListMenu.h"
#include "../struct/NextLevelData.h"
#include "../struct/PlayerCharacter.h"

class CarrotQt5;

class CharacterSelectMenu : public VerticalItemListMenu {
public:
    CharacterSelectMenu(CarrotQt5* root, std::shared_ptr<NextLevelData> data);
    ~CharacterSelectMenu();

private:
    std::shared_ptr<NextLevelData> nextData;
    void makeCharacterOption(PlayerCharacter character, const QString& label);
};
