#include "CharacterSelectMenu.h"

#include "DifficultySelectMenu.h"

#include "../CarrotQt5.h"

#include <QDir>
#include <QSettings>

CharacterSelectMenu::CharacterSelectMenu(CarrotQt5* mainClass, std::shared_ptr<NextLevelData> data) : VerticalItemListMenu(mainClass), nextData(data) {
    makeCharacterOption(CHAR_JAZZ, "Jazz");
    makeCharacterOption(CHAR_SPAZ, "Spaz");
    makeCharacterOption(CHAR_LORI, "Lori");

    setMenuItemSelected(0);
    attractionText.setText("Select Character");
}

CharacterSelectMenu::~CharacterSelectMenu() {

}

void CharacterSelectMenu::makeCharacterOption(PlayerCharacter character, const QString& label) {
    menuOptions.append(buildMenuItem(
        [this, character]() {
            nextData->characters[0] = character;
            root->pushState<DifficultySelectMenu>(false, nextData);
        },
        label
    ));
}
