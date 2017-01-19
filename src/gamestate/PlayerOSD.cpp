#include "PlayerOSD.h"

#include <cmath>
#include "ActorAPI.h"
#include "GameView.h"
#include "../actor/Player.h"
#include "../graphics/ShaderSource.h"
#include "../graphics/BitmapString.h"

#define LEVEL_TEXT_STILL_FRAMES 350
#define LEVEL_TEXT_TRANSITION_FRAMES 100
#define LEVEL_TEXT_TOTAL_FRAMES LEVEL_TEXT_STILL_FRAMES + LEVEL_TEXT_TRANSITION_FRAMES * 2

PlayerOSD::PlayerOSD(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> player)
    : AnimationUser(api), owner(player), collectionMessageType(OSD_NONE), messageTimer(-1l), messageOffsetAmount(0),
    collectibleIconOffset(32.0f), currentWeapon(WEAPON_BLASTER), score(0), health(0),
    gemCounter(0), sugarRushLeft(0), levelTextFrame(LEVEL_TEXT_TOTAL_FRAMES), levelTextIdx(0) {

    heartTexture = sf::Texture();
    heartTexture.loadFromFile("Data/Assets/ui/heart.png");

    auto loadedResources = api->loadActorTypeResources("UI/PlayerOSD");
    if (loadedResources != nullptr) {
        loadAnimationSet(loadedResources->graphics);
        gemSound = loadedResources->sounds.value("PLAYER_PICKUP_GEM", SoundResource()).sound;
        notEnoughCoinsSound = loadedResources->sounds.value("PLAYER_BONUS_WARP_NOT_ENOUGH_COINS", SoundResource()).sound;
    }

    charIcon = std::make_shared<AnimationInstance>(this);
    weaponIcon = std::make_shared<AnimationInstance>(this);
    collectibleIcon = std::make_shared<AnimationInstance>(this);

    std::fill_n(weaponIconIdx, 18, nullptr);
    weaponIconIdx[0] = animationBank.value("UI_WEAPON_BLASTER_JAZZ");
    weaponIconIdx[1] = animationBank.value("UI_WEAPON_BOUNCER");
    weaponIconIdx[2] = animationBank.value("UI_WEAPON_FREEZER");
    weaponIconIdx[3] = animationBank.value("UI_WEAPON_SEEKER");
    weaponIconIdx[4] = animationBank.value("UI_WEAPON_RF");
    weaponIconIdx[5] = animationBank.value("UI_WEAPON_TOASTER");
    weaponIconIdx[6] = animationBank.value("UI_WEAPON_TNT");
    weaponIconIdx[7] = animationBank.value("UI_WEAPON_PEPPER");
    weaponIconIdx[8] = animationBank.value("UI_WEAPON_ELECTRO");
    weaponIconIdx[9] = animationBank.value("UI_WEAPON_POWERUP_BLASTER_JAZZ");
    weaponIconIdx[10] = animationBank.value("UI_WEAPON_POWERUP_BOUNCER");
    weaponIconIdx[11] = animationBank.value("UI_WEAPON_POWERUP_FREEZER");
    weaponIconIdx[12] = animationBank.value("UI_WEAPON_POWERUP_SEEKER");
    weaponIconIdx[13] = animationBank.value("UI_WEAPON_POWERUP_RF");
    weaponIconIdx[14] = animationBank.value("UI_WEAPON_POWERUP_TOASTER");
    weaponIconIdx[15] = animationBank.value("UI_WEAPON_POWERUP_TNT");
    weaponIconIdx[16] = animationBank.value("UI_WEAPON_POWERUP_PEPPER");
    weaponIconIdx[17] = animationBank.value("UI_WEAPON_POWERUP_ELECTRO");

    weaponIcon->setAnimation(weaponIconIdx[0]);

    auto charIconGfx = animationBank.value("UI_CHARACTER_ICON_JAZZ");
    charIcon->setAnimation(charIconGfx);

    collectionMessage = api->makeString("", NORMAL, FONT_ALIGN_CENTER);
    livesString       = api->makeString("x3", NORMAL, FONT_ALIGN_LEFT);
    scoreString       = api->makeString("00000000", NORMAL, FONT_ALIGN_LEFT);
    ammoString        = api->makeString("x^", NORMAL, FONT_ALIGN_LEFT);
    sugarRushText     = api->makeString("", NORMAL, FONT_ALIGN_CENTER);
    sugarRushText->setColoured(true);
    sugarRushText->setAnimation(true, -3.0, 3.0, 0.05, 0.95);

    levelTextString = api->makeString("", SMALL, FONT_ALIGN_CENTER);
    levelTextString->setAnimation(true, 0.8, 0.8, 0.03, 0.72);
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
    auto canvasPtr = canvas.get();

    unsigned vw = view->getViewWidth();
    unsigned vh = view->getViewHeight();
    charIcon->setSpritePosition({ 5.0f, (float)view->getViewHeight() });
    charIcon->drawCurrentFrame(*canvas);

    weaponIcon->setSpritePosition({
        view->getViewWidth() - 80.0f,
        view->getViewHeight() - 15.0f
    });
    weaponIcon->drawCurrentFrame(*canvas);

    livesString->drawString(canvasPtr, 40, vh - 25);

    sf::Sprite heartspr(heartTexture);
    for (unsigned i = 0; i < health; ++i) {
        heartspr.setPosition((vw - 100.0) + i * 18.0, 5.0);
        if (!((health == 1) && ((api->getFrame() % 6) > 2))) {
            canvas->draw(heartspr);
        }
    }

    ammoString->drawString(canvasPtr, vw - 70, vh - 25);

    // Draw the current score
    scoreString->drawString(canvasPtr, 6, 6);

    if (collectionMessageType != OSD_NONE) {
        messageOffsetAmount = std::min(60u, messageOffsetAmount + 1);
    } else {
        if (messageOffsetAmount > 0) {
            messageOffsetAmount--;
        }
    }
    if (messageOffsetAmount > 0) {
        collectionMessage->drawString(canvasPtr, vw / 2 + 30 - messageOffsetAmount / 2, vh - messageOffsetAmount / 2);
        if (collectibleGraphics != nullptr) {
            // TODO: Move the sprite from the player's position to the UI instead of from below with the count
            collectibleIcon->setSpritePosition({
                vw / 2.0f - collectionMessage->getWidth() / 2.0f + collectibleIconOffset - messageOffsetAmount / 2.0f,
                vh - messageOffsetAmount / 2.0f + 12.0f
            });

            collectibleIcon->drawCurrentFrame(*canvasPtr);
        }
    }

    if (levelTextFrame < LEVEL_TEXT_TOTAL_FRAMES) {
        int offset = 0;
        if (levelTextFrame < LEVEL_TEXT_TRANSITION_FRAMES) {
            offset = std::pow((LEVEL_TEXT_TRANSITION_FRAMES - levelTextFrame) / 12.0, 3);
        }
        if (levelTextFrame > LEVEL_TEXT_TRANSITION_FRAMES + LEVEL_TEXT_STILL_FRAMES) {
            offset = -std::pow((levelTextFrame - LEVEL_TEXT_TRANSITION_FRAMES - LEVEL_TEXT_STILL_FRAMES) / 12.0, 3);
        }
        levelTextString->drawString(canvasPtr, vw / 2 + offset, 10);

        levelTextFrame++;
    }

    if (sugarRushLeft > 0) {
        if (sugarRushLeft % 70 == 0) {
            sugarRushText->setText("Sugar  RUSH  " + QString::number(sugarRushLeft / 70 - 1));
        }
        if (sugarRushLeft > 140 || sugarRushLeft % 6 > 2) {
            sugarRushText->drawString(canvasPtr, vw / 2, 10);
        }
        sugarRushLeft--;
    }

    for (auto& text : specialOverlayTexts) {
        text->text->drawString(canvasPtr, vw * text->position.x, vh * text->position.y);
        text->updateFunc(*text);
        text->frame++;
    }

    for (auto& anim : specialOverlayGraphics) {
        anim->anim->setSpritePosition({ vw * anim->position.x, vh * anim->position.y });
        anim->anim->drawCurrentFrame(*canvasPtr);
        anim->anim->advanceTimers();
        anim->updateFunc(*anim);
        anim->frame++;
    }
}


void PlayerOSD::setMessage(OSDMessageType type, QVariant param) {
    if (type == OSD_BONUS_WARP_NOT_ENOUGH_COINS && type == collectionMessageType) {
        return;
    }

    cancelTimer(messageTimer);
    messageOffsetAmount = 0;
    messageTimer = addTimer(350u, false, [this]() {
        collectionMessageType = OSD_NONE;
        gemCounter = 0;
    });

    collectionMessageType = type;
    collectibleIcon->setColor({ 0, 0, 0 });
    collectibleIconOffset = 32.0f;
    collectibleGraphics = nullptr;

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
            collectionMessage->setText("need    x" + QString::number(param.toInt()) + " more");
            collectibleGraphics = animationBank.value("PICKUP_COIN_SILVER", nullptr);
            collectibleIconOffset = 32.0f + api->getStringWidth("need  ");
            api->playSound(notEnoughCoinsSound, false);
            break;
        case OSD_CUSTOM_TEXT:
            collectionMessage->setText(param.toString());
            break;
        default:
            break;
    }

    if (collectibleGraphics != nullptr) {
        collectibleIcon->setAnimation(collectibleGraphics);
    }

    if (type == OSD_GEM_BLUE || type == OSD_GEM_GREEN || type == OSD_GEM_RED || type == OSD_GEM_PURPLE) {
        api->playSound(gemSound, false, 1.0f, static_cast<float>(4.25 - std::abs(gemCounter % 16 - 8.25)));
        gemCounter++;
    }
}

void PlayerOSD::setWeaponType(WeaponType type, bool poweredUp) {
    currentWeapon = type;
    uint iconIdx = (uint)type + (poweredUp ? 9 : 0);

    if (weaponIconIdx[type] != nullptr) {
        weaponIcon->setAnimation(weaponIconIdx[iconIdx]);
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
    initLevelStartOverlay();
}

void PlayerOSD::setSugarRushActive() {
    sugarRushLeft = 21 * 70;
}

void PlayerOSD::setLevelText(int idx) {
    if (idx == levelTextIdx && levelTextFrame < LEVEL_TEXT_TOTAL_FRAMES) {
        return;
    }

    levelTextIdx = idx;
    levelTextFrame = 0;
    levelTextString->setText(api->getLevelText(idx));
}

void PlayerOSD::initLevelStartOverlay() {
    auto id = createOverlayText(350u, sf::Vector2f(0.5, 0.48), levelStartStrings.at(qrand() % levelStartStrings.length()), [](SpecialOSDText& text) {
        if (text.frame >= 250u) {
            text.position.x = 0.5 - std::pow((text.frame - 250u) / 100.0, 2.0) * 0.6;
        }
    }, FONT_ALIGN_CENTER, 0, 6, 0.05, 0.5);
}

void PlayerOSD::initLevelCompletedOverlay(uint redGems, uint greenGems, uint blueGems, uint) {
    createOverlayText(440u, sf::Vector2f(3, 0.1), "LEVEL COMPLETE", getSlideInUpdateFunc<SpecialOSDText>(0, 35, 0.5), FONT_ALIGN_CENTER, 0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.4), QString::number(redGems),   getSlideInUpdateFunc<SpecialOSDText>(50,  85, 0.6), FONT_ALIGN_RIGHT, 0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.5), QString::number(greenGems), getSlideInUpdateFunc<SpecialOSDText>(60,  95, 0.6), FONT_ALIGN_RIGHT, 0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.6), QString::number(blueGems),  getSlideInUpdateFunc<SpecialOSDText>(70, 105, 0.6), FONT_ALIGN_RIGHT, 0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.4), "x 1 ",                     getSlideInUpdateFunc<SpecialOSDText>(50,  85, 0.7), FONT_ALIGN_LEFT,  0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.5), "x 5 ",                     getSlideInUpdateFunc<SpecialOSDText>(60,  95, 0.7), FONT_ALIGN_LEFT,  0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.6), "x 10",                     getSlideInUpdateFunc<SpecialOSDText>(70, 105, 0.7), FONT_ALIGN_LEFT,  0, 1, 0.05, 0.5);
    createOverlayText(440u, sf::Vector2f(3, 0.75), "TOTAL      " + QString::number(redGems + greenGems * 5 + blueGems * 10),
                      getSlideInUpdateFunc<SpecialOSDText>(80, 115, 0.7), FONT_ALIGN_RIGHT, 0, 1, 0.05, 0.5);

    int id;
    id = createOverlayGraphics(440u, sf::Vector2f(3, 0.42), animationBank.value("PICKUP_GEM", nullptr), getSlideInUpdateFunc<SpecialOSDGraphics>(50,  85, 0.65));
    specialOverlayGraphics.at(id)->anim->setColor({ 512, 0, 0 });

    id = createOverlayGraphics(440u, sf::Vector2f(3, 0.52), animationBank.value("PICKUP_GEM", nullptr), getSlideInUpdateFunc<SpecialOSDGraphics>(60,  95, 0.65));
    specialOverlayGraphics.at(id)->anim->setColor({ 0, 512, 0 });

    id = createOverlayGraphics(440u, sf::Vector2f(3, 0.62), animationBank.value("PICKUP_GEM", nullptr), getSlideInUpdateFunc<SpecialOSDGraphics>(70, 105, 0.65));
    specialOverlayGraphics.at(id)->anim->setColor({ 0, 0, 512 });

}

int PlayerOSD::createOverlayText(uint lifetime, sf::Vector2f position, const QString& text, std::function<void(SpecialOSDText&)> updateFunc,
                                 FontAlign align, double vx, double vy, double s, double a) {
    int id = specialOverlayTexts.insertAndGetID(position, api->makeString(text, NORMAL, align), updateFunc);

    addTimer(lifetime, false, [this, id]() {
        specialOverlayTexts.remove(id);
    });

    specialOverlayTexts.at(id)->text->setAnimation(true, vx, vy, s, a);

    return id;
}

int PlayerOSD::createOverlayGraphics(uint lifetime, sf::Vector2f position, std::shared_ptr<GraphicResource> anim, std::function<void(SpecialOSDGraphics&)> updateFunc) {
    int id = specialOverlayGraphics.insertAndGetID(position, nullptr, updateFunc);

    addTimer(lifetime, false, [this, id]() {
        specialOverlayGraphics.remove(id);
    });

    specialOverlayGraphics.at(id)->anim = std::make_shared<AnimationInstance>(this);
    specialOverlayGraphics.at(id)->anim->setAnimation(anim);

    return id;
}

template<typename T>
std::function<void(T&)> PlayerOSD::getSlideInUpdateFunc(int startFrame, int endFrame, double finalX) {
    return [startFrame, endFrame, finalX](auto& item) {
        if (item.frame > startFrame && item.frame <= endFrame) {
            item.position.x = finalX + std::pow((endFrame - item.frame) / 35.0, 2.0) * 0.6;
        }
    };
}


const QStringList PlayerOSD::levelStartStrings = {
    "LETS ROCK",
    "GET READY",
    "KICK SHELL",
    "GET MOVING",
    "SHAKE YOUR TAIL",
    "BUST A MOVE"
};
