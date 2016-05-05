#include "PlayerOSD.h"
#include "../CarrotQt5.h"
#include "../actor/Player.h"
#include "../graphics/ShaderSource.h"

PlayerOSD::PlayerOSD(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> player)
    : AnimationUser(root), owner(player), messageTimer(-1l), collectionMessageType(OSD_NONE), health(0), score(0),
    currentWeapon(WEAPON_BLASTER), messageOffsetAmount(0), gemCounter(0), sugarRushLeft(0) {

    heartTexture = sf::Texture();
    heartTexture.loadFromFile("Data/Assets/ui/heart.png");

    auto loadedResources = root->loadActorTypeResources("UI/PlayerOSD");
    if (loadedResources != nullptr) {
        loadAnimationSet(loadedResources->graphics);
        gemSound = loadedResources->sounds.value("PLAYER_PICKUP_GEM", SoundResource()).sound;
    }

    charIcon = std::make_shared<AnimationInstance>(this);
    weaponIcon = std::make_shared<AnimationInstance>(this);
    collectibleIcon = std::make_shared<AnimationInstance>(this);

    std::fill_n(weaponIconIdx, 9, nullptr);
    weaponIconIdx[0] = animationBank.value("UI_WEAPON_BLASTER_JAZZ");
    weaponIconIdx[1] = animationBank.value("UI_WEAPON_BOUNCER");
    weaponIconIdx[2] = animationBank.value("UI_WEAPON_FREEZER");
    weaponIconIdx[3] = animationBank.value("UI_WEAPON_SEEKER");
    weaponIconIdx[4] = animationBank.value("UI_WEAPON_RF");
    weaponIconIdx[5] = animationBank.value("UI_WEAPON_TOASTER");
    weaponIconIdx[6] = animationBank.value("UI_WEAPON_TNT");
    weaponIconIdx[7] = animationBank.value("UI_WEAPON_PEPPER");
    weaponIconIdx[8] = animationBank.value("UI_WEAPON_ELECTRO");

    weaponIcon->setAnimation(weaponIconIdx[0]);

    auto charIconGfx = animationBank.value("UI_CHARACTER_ICON_JAZZ");
    charIcon->setAnimation(charIconGfx);

    collectionMessage = std::make_unique<BitmapString>(root->getFont(), "", FONT_ALIGN_CENTER);
    livesString       = std::make_unique<BitmapString>(root->getFont(), "x3", FONT_ALIGN_LEFT);
    scoreString       = std::make_unique<BitmapString>(root->getFont(), "00000000", FONT_ALIGN_LEFT);
    ammoString        = std::make_unique<BitmapString>(root->getFont(), "x^", FONT_ALIGN_LEFT);
    sugarRushText     = std::make_unique<BitmapString>(root->getFont(), "", FONT_ALIGN_CENTER);
    sugarRushText->setColoured(true);
    sugarRushText->setAnimation(true, -3.0, 3.0, 0.05, 0.95);
}

PlayerOSD::~PlayerOSD() {

}

void PlayerOSD::drawOSD(std::shared_ptr<GameView>& view) {
    advanceTimers();
    charIcon->advanceTimers();
    weaponIcon->advanceTimers();

    if (collectibleGraphics != nullptr) {
        collectibleIcon->advanceTimers();
    }

    auto canvas = view->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    unsigned vw = view->getViewWidth();
    unsigned vh = view->getViewHeight();
    charIcon->setSpritePosition({ 5.0f, (float)view->getViewHeight() });
    charIcon->drawCurrentFrame(*canvas);

    weaponIcon->setSpritePosition({
        view->getViewWidth() - 80.0f,
        view->getViewHeight() - 15.0f
    });
    weaponIcon->drawCurrentFrame(*canvas);

    livesString->drawString(canvas, 40, vh - 25);

    sf::Sprite heartspr(heartTexture);
    for (unsigned i = 0; i < health; ++i) {
        heartspr.setPosition((vw - 100.0) + i * 18.0, 5.0);
        if (!((health == 1) && ((root->getFrame() % 6) > 2))) {
            canvas->draw(heartspr);
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
        if (collectibleGraphics != nullptr) {
            // TODO: Move the sprite from the player's position to the UI instead of from below with the count
            collectibleIcon->setSpritePosition({
                vw / 2.0f - collectionMessage->getWidth() / 2.0f + 32.0f - messageOffsetAmount / 2.0f,
                vh - messageOffsetAmount / 2.0f + 12.0f
            });

            collectibleIcon->drawCurrentFrame(*canvas);
        }
    }

    if (sugarRushLeft > 0) {
        if (sugarRushLeft % 70 == 0) {
            sugarRushText->setText("Sugar  RUSH  " + QString::number(sugarRushLeft / 70 - 1));
        }
        if (sugarRushLeft > 140 || sugarRushLeft % 6 > 2) {
            sugarRushText->drawString(canvas, vw / 2, 10);
        }
        sugarRushLeft--;
    }
}


void PlayerOSD::setMessage(OSDMessageType type, QVariant param) {
    cancelTimer(messageTimer);
    messageOffsetAmount = 0;
    messageTimer = addTimer(350u, false, [this]() {
        collectionMessageType = OSD_NONE;
        gemCounter = 0;
    });

    collectionMessageType = type;
    collectibleIcon->setColor({ 0, 0, 0 });

    switch (type) {
        case OSD_GEM_RED:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            collectibleIcon->setColor({ 512, 0, 0 });
            collectibleGraphics = animationBank.value("PICKUP_GEM", nullptr);
            break;
        case OSD_GEM_GREEN:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            collectibleIcon->setColor({ 0, 512, 0 });
            collectibleGraphics = animationBank.value("PICKUP_GEM", nullptr);
            break;
        case OSD_GEM_BLUE:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            collectibleIcon->setColor({ 0, 0, 512 });
            collectibleGraphics = animationBank.value("PICKUP_GEM", nullptr);
            break;
        case OSD_GEM_PURPLE:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            collectibleIcon->setColor({ 256, 0, 512 });
            collectibleGraphics = animationBank.value("PICKUP_GEM", nullptr);
            break;
        case OSD_COIN_SILVER:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            collectibleGraphics = animationBank.value("PICKUP_COIN_SILVER", nullptr);
            break;
        case OSD_COIN_GOLD:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            collectibleGraphics = animationBank.value("PICKUP_COIN_GOLD", nullptr);
            break;
        case OSD_BONUS_WARP_NOT_ENOUGH_COINS:
            collectionMessage->setText("need   x" + QString::number(param.toInt()) + " more");
            break;
        case OSD_CUSTOM_TEXT:
            collectionMessage->setText(param.toString());
            break;
    }

    if (collectibleGraphics != nullptr) {
        collectibleIcon->setAnimation(collectibleGraphics);
    }

    if (type == OSD_GEM_BLUE || type == OSD_GEM_GREEN || type == OSD_GEM_RED || type == OSD_GEM_PURPLE) {
        root->getSoundSystem().lock()->playSFX(gemSound, false, 1.0, 4.25 - abs(gemCounter % 16 - 8.25));
        gemCounter++;
    }
}

void PlayerOSD::setWeaponType(WeaponType type, bool poweredUp) {
    currentWeapon = type;

    if (weaponIconIdx[type] != nullptr) {
        weaponIcon->setAnimation(weaponIconIdx[type]);
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

void PlayerOSD::setSugarRushActive() {
    sugarRushLeft = 21 * 70;
}

