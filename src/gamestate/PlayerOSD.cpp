#include "PlayerOSD.h"
#include "../CarrotQt5.h"
#include "../actor/Player.h"

PlayerOSD::PlayerOSD(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> player, std::weak_ptr<sf::RenderWindow> canvas)
    : AnimationUser(root), owner(player), messageTimer(-1l), collectionMessageType(OSD_NONE), canvas(canvas), health(0), score(0),
    currentWeapon(WEAPON_BLASTER), messageOffsetAmount(0) {

    addTimer(7u, true, static_cast<TimerCallbackFunc>(&PlayerOSD::advanceCharIconFrame));

    heartTexture = sf::Texture();
    heartTexture.loadFromFile("Data/Assets/ui/heart.png");

    std::fill_n(weaponIconIdx, 9, -1);
    weaponIconIdx[0] = addAnimation(AnimState::UI_WEAPON_BLASTER, "pickup/fast_fire_jazz.png", 10, 1, 17, 22, 10, 5, 12);
    weaponIconIdx[1] = addAnimation(AnimState::UI_WEAPON_BOUNCER, "pickup/ammo_bouncer.png",   10, 1, 16, 13, 10, 5, 7);
    weaponIconIdx[2] = addAnimation(AnimState::UI_WEAPON_FREEZER, "pickup/ammo_freezer.png",   10, 1, 13, 15, 10, 4, 8);
    weaponIconIdx[3] = addAnimation(AnimState::UI_WEAPON_SEEKER,  "pickup/ammo_seeker.png",    10, 1, 19, 20, 10, 7, 10);
    weaponIconIdx[4] = addAnimation(AnimState::UI_WEAPON_RF,      "pickup/ammo_rf.png",        10, 1, 13, 20, 10, 4, 10);
    weaponIconIdx[5] = addAnimation(AnimState::UI_WEAPON_TOASTER, "pickup/ammo_toaster.png",   10, 1, 16, 14, 10, 5, 7);
    weaponIconIdx[6] = addAnimation(AnimState::UI_WEAPON_TNT,     "pickup/ammo_tnt.png",       10, 1, 20, 27, 10, 7, 13);
    weaponIconIdx[7] = addAnimation(AnimState::UI_WEAPON_PEPPER,  "pickup/ammo_pepper.png",     9, 1, 15, 13, 10, 5, 7);
    weaponIconIdx[8] = addAnimation(AnimState::UI_WEAPON_ELECTRO, "pickup/ammo_electro.png",   10, 1, 30, 21, 10, 14, 10);
    weaponIconSprite = std::make_unique<sf::Sprite>();
    weaponIconSprite->setTexture(*(animation_bank.at(weaponIconIdx[0])->animation_frames));
    weaponIconSprite->setTextureRect(sf::IntRect(0, 0, animation_bank.at(weaponIconIdx[0])->frame_width,
        animation_bank.at(weaponIconIdx[0])->frame_height));
    weaponIconSprite->setPosition(root->getViewWidth() - 85 - animation_bank.at(weaponIconIdx[0])->offset_x,
        root->getViewHeight() - 15 - animation_bank.at(weaponIconIdx[0])->offset_y);

    addAnimation(AnimState::UI_OSD_GEM_RED,   "pickup/gem.png", 8, 1, 25, 26, 10, 10, 13);
    addAnimation(AnimState::UI_OSD_GEM_GREEN, "pickup/gem.png", 8, 1, 25, 26, 10, 10, 13);
    addAnimation(AnimState::UI_OSD_GEM_BLUE,  "pickup/gem.png", 8, 1, 25, 26, 10, 10, 13);

    auto index = addAnimation(AnimState::UI_PLAYER_FACE, "ui/icon_jazz.png", 37, 1, 39, 39, 10, 0, 0);
    charIconSprite = std::make_unique<sf::Sprite>();
    charIconSprite->setTexture(*(animation_bank.at(index)->animation_frames));
    charIconSprite->setTextureRect(sf::IntRect(0, 0, animation_bank.at(index)->frame_width, 
        animation_bank.at(index)->frame_height));
    charIconSprite->setPosition(5, root->getViewHeight() - 40);

    collectionMessage = std::make_unique<BitmapString>(root->mainFont, "", FONT_ALIGN_CENTER);
    livesString       = std::make_unique<BitmapString>(root->mainFont, "x3", FONT_ALIGN_LEFT);
    scoreString       = std::make_unique<BitmapString>(root->mainFont, "00000000", FONT_ALIGN_LEFT);
    ammoString        = std::make_unique<BitmapString>(root->mainFont, "x^", FONT_ALIGN_LEFT);
}

PlayerOSD::~PlayerOSD() {

}

void PlayerOSD::drawOSD() {
    advanceTimers();

    auto canvasPtr = canvas.lock();
    if (canvasPtr == nullptr) {
        return;
    }

    unsigned vw = root->getViewWidth();
    unsigned vh = root->getViewHeight();
    canvasPtr->draw(*charIconSprite);
    canvasPtr->draw(*weaponIconSprite);

    livesString->drawString(root->getCanvas(), 40, vh - 25);

    sf::Sprite heartspr(heartTexture);
    for (unsigned i = 0; i < health; ++i) {
        heartspr.setPosition((vw - 100.0) + i * 18.0, 5.0);
        if (!((health == 1) && ((root->getFrame() % 6) > 2))) {
            canvasPtr->draw(heartspr);
        }
    }

    ammoString->drawString(canvas, vw - 70, vh - 25);

    // Draw the current score
    scoreString->drawString(canvas, 6, 6);

    if (collectionMessageType != OSD_NONE) {
        messageOffsetAmount = std::min(60u, messageOffsetAmount + 1);
    } else {
        if (messageOffsetAmount > 0) {
            messageOffsetAmount--;
        }
    }
    if (messageOffsetAmount > 0) {
        collectionMessage->drawString(canvas, vw / 2 + 30 - messageOffsetAmount / 2, vh - messageOffsetAmount / 2);
        switch (collectionMessageType) {
            case OSD_GEM_RED:

            case OSD_NONE:
            default:
                break;
        }
    }
}

void PlayerOSD::clearMessage() {
    collectionMessageType = OSD_NONE;
}

void PlayerOSD::setMessage(OSDMessageType type, QVariant param) {
    cancelTimer(messageTimer);
    messageOffsetAmount = 0;
    messageTimer = addTimer(350u, false, static_cast<TimerCallbackFunc>(&PlayerOSD::clearMessage));
    collectionMessageType = type;

    switch (type) {
        case OSD_GEM_RED:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            break;
        case OSD_GEM_GREEN:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            break;
        case OSD_GEM_BLUE:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            break;
        case OSD_COIN_SILVER:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            break;
        case OSD_COIN_GOLD:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            break;
        case OSD_BONUS_WARP_NOT_ENOUGH_COINS:
            collectionMessage->setText("need   x" + QString::number(param.toInt()) + " more");
            break;
        case OSD_CUSTOM_TEXT:
            collectionMessage->setText(param.toString());
            break;
    }
}

void PlayerOSD::setWeaponType(WeaponType type, bool poweredUp) {
    currentWeapon = type;

    if (weaponIconIdx[type] != -1) {
        auto wp = animation_bank.at(weaponIconIdx[type]);
        weaponIconSprite->setTexture(*(wp->animation_frames));
        weaponIconSprite->setTextureRect(sf::IntRect(0, 0, wp->frame_width, wp->frame_height));
        weaponIconSprite->setPosition(root->getViewWidth() - 85 - wp->offset_x, root->getViewHeight() - 15 - wp->offset_y);
        weaponIconFrame = 0;
    }
}

void PlayerOSD::setAmmo(unsigned ammo) {
    if (currentWeapon == WEAPON_BLASTER) {
        ammoString->setText("x^");
    } else {
        ammoString->setText(QString("x") + QString::number(ammo));
    }
}

void PlayerOSD::setHealth(unsigned newHealth) {
    health = std::min(newHealth, 5U);
}

void PlayerOSD::setScore(unsigned long newScore) {
    score = std::min(newScore, 99999999UL);
    scoreString->setText(QString::number(score).rightJustified(8, '0', false));
}

void PlayerOSD::setLives(unsigned lives) {
    livesString->setText(QString("x") + QString::number(lives));
}

// TODO: Get rid of this, this is a ridiculous hack.
// Possibly break AnimationUser even further in the future to allow more than one animation at once.
void PlayerOSD::advanceCharIconFrame() {
    charIconFrame = (charIconFrame + 1) % 37;
    charIconSprite->setTextureRect(sf::IntRect(charIconFrame * 39, 0, 39, 39));

    if (weaponIconIdx[currentWeapon] != -1) {
        weaponIconFrame = (weaponIconFrame + 1) % animation_bank.at(weaponIconIdx[currentWeapon])->frame_cols;

        weaponIconSprite->setTextureRect(sf::IntRect(
            weaponIconFrame*animation_bank.at(weaponIconIdx[currentWeapon])->frame_width, 0,
            animation_bank.at(weaponIconIdx[currentWeapon])->frame_width,
            animation_bank.at(weaponIconIdx[currentWeapon])->frame_height));
    }
}