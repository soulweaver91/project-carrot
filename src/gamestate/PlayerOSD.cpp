#include "PlayerOSD.h"
#include "../CarrotQt5.h"
#include "../actor/Player.h"
#include "../graphics/ShaderSource.h"

PlayerOSD::PlayerOSD(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> player, std::weak_ptr<sf::RenderWindow> canvas)
    : AnimationUser(root), owner(player), messageTimer(-1l), collectionMessageType(OSD_NONE), canvas(canvas), health(0), score(0),
    currentWeapon(WEAPON_BLASTER), messageOffsetAmount(0), gemCounter(0) {

    addTimer(7u, true, static_cast<TimerCallbackFunc>(&PlayerOSD::advanceCharIconFrame));

    heartTexture = sf::Texture();
    heartTexture.loadFromFile("Data/Assets/ui/heart.png");

    auto loadedResources = root->loadActorTypeResources("UI/PlayerOSD");
    if (loadedResources != nullptr) {
        loadAnimationSet(loadedResources->graphics);
        gemSound = loadedResources->sounds.value("PLAYER_PICKUP_GEM", SoundResource()).sound;
    }

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

    weaponIconSprite = std::make_unique<sf::Sprite>();
    weaponIconSprite->setTexture(*weaponIconIdx[0]->texture);
    weaponIconSprite->setTextureRect(sf::IntRect(0, 0, weaponIconIdx[0]->frameDimensions.x,
        weaponIconIdx[0]->frameDimensions.y));
    weaponIconSprite->setPosition(root->getViewWidth() - 85 - weaponIconIdx[0]->hotspot.x,
        root->getViewHeight() - 15 - weaponIconIdx[0]->hotspot.y);

    auto charIcon = animationBank.value("UI_CHARACTER_ICON_JAZZ");
    charIconSprite = std::make_unique<sf::Sprite>();
    charIconSprite->setTexture(*(charIcon->texture));
    charIconSprite->setTextureRect(sf::IntRect(0, 0, charIcon->frameDimensions.x,
        charIcon->frameDimensions.y));
    charIconSprite->setPosition(5, root->getViewHeight() - 40);

    collectionMessage = std::make_unique<BitmapString>(root->getFont(), "", FONT_ALIGN_CENTER);
    livesString       = std::make_unique<BitmapString>(root->getFont(), "x3", FONT_ALIGN_LEFT);
    scoreString       = std::make_unique<BitmapString>(root->getFont(), "00000000", FONT_ALIGN_LEFT);
    ammoString        = std::make_unique<BitmapString>(root->getFont(), "x^", FONT_ALIGN_LEFT);

    collectibleSprite = std::make_unique<sf::Sprite>();
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
        if (collectibleGraphics != nullptr) {
            sf::RenderStates state;
            if (currentAnimation.color != sf::Vector3i(0, 0, 0)) {
                auto shader = ShaderSource::getShader("ColorizeShader").get();
                if (shader != nullptr) {
                    shader->setParameter("color", 
                        currentAnimation.color.x / 255.0f, 
                        currentAnimation.color.y / 255.0f, 
                        currentAnimation.color.z / 255.0f);
                    state.shader = shader;
                }
            }
            // TODO: Move the sprite from the player's position to the UI instead of from below with the count
            collectibleSprite->setPosition(
                vw / 2 - collectionMessage->getWidth() / 2 + 20 - messageOffsetAmount / 2,
                vh - messageOffsetAmount / 2
            );

            canvasPtr->draw(*collectibleSprite, state);
        }

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
    gemCounter = 0;
}

void PlayerOSD::setMessage(OSDMessageType type, QVariant param) {
    cancelTimer(messageTimer);
    messageOffsetAmount = 0;
    messageTimer = addTimer(350u, false, static_cast<TimerCallbackFunc>(&PlayerOSD::clearMessage));
    collectionMessageType = type;
    currentAnimation.color = { 0, 0, 0 };
    collectibleFrame = 0;

    switch (type) {
        case OSD_GEM_RED:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            currentAnimation.color = { 511, 0, 0 };
            collectibleGraphics = animationBank.value("PICKUP_GEM", nullptr);
            break;
        case OSD_GEM_GREEN:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            currentAnimation.color = { 0, 511, 0 };
            collectibleGraphics = animationBank.value("PICKUP_GEM", nullptr);
            break;
        case OSD_GEM_BLUE:
            collectionMessage->setText("  x" + QString::number(param.toInt()));
            currentAnimation.color = { 0, 0, 511 };
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
        collectibleSprite->setTexture(*collectibleGraphics->texture);
        collectibleSprite->setTextureRect(sf::IntRect(
            0, 0,
            collectibleGraphics->frameDimensions.x,
            collectibleGraphics->frameDimensions.y));
    }

    if (type == OSD_GEM_BLUE || type == OSD_GEM_GREEN || type == OSD_GEM_RED || type == OSD_GEM_PURPLE) {
        root->getSoundSystem().lock()->playSFX(gemSound, 1.0, 4.25 - abs(gemCounter % 16 - 8.25));
        gemCounter++;
    }
}

void PlayerOSD::setWeaponType(WeaponType type, bool poweredUp) {
    currentWeapon = type;

    if (weaponIconIdx[type] != nullptr) {
        weaponIconSprite->setTexture(*(weaponIconIdx[type]->texture));
        weaponIconSprite->setTextureRect(sf::IntRect(0, 0, weaponIconIdx[type]->frameDimensions.x,
            weaponIconIdx[type]->frameDimensions.y));
        weaponIconSprite->setPosition(root->getViewWidth() - 85 - weaponIconIdx[type]->hotspot.x,
            root->getViewHeight() - 15 - weaponIconIdx[type]->hotspot.y);
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

    if (weaponIconIdx[currentWeapon] != nullptr) {
        weaponIconFrame = (weaponIconFrame + 1) % weaponIconIdx[currentWeapon]->frameCount;
    
        weaponIconSprite->setTextureRect(sf::IntRect(
            weaponIconFrame * weaponIconIdx[currentWeapon]->frameDimensions.x, 0,
            weaponIconIdx[currentWeapon]->frameDimensions.x,
            weaponIconIdx[currentWeapon]->frameDimensions.y));
    }

    if (collectibleGraphics != nullptr) {
        collectibleFrame = (collectibleFrame + 1) % collectibleGraphics->frameCount;

        collectibleSprite->setTextureRect(sf::IntRect(
            collectibleFrame * collectibleGraphics->frameDimensions.x, 0,
            collectibleGraphics->frameDimensions.x,
            collectibleGraphics->frameDimensions.y));
    }
}