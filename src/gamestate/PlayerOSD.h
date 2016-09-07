#pragma once

#include <memory>
#include <QVariant>
#include <SFML/Graphics.hpp>
#include "../gamestate/AnimationUser.h"
#include "../struct/WeaponTypes.h"
#include "../struct/Resources.h"

class CarrotQt5;
class Player;
class BitmapString;

enum OSDMessageType {
    OSD_NONE,
    OSD_CUSTOM_TEXT,
    OSD_GEM_RED,
    OSD_GEM_GREEN,
    OSD_GEM_BLUE,
    OSD_GEM_PURPLE,
    OSD_COIN_SILVER,
    OSD_COIN_GOLD,
    OSD_BONUS_WARP_NOT_ENOUGH_COINS
};

class PlayerOSD : public AnimationUser {
public:
    PlayerOSD(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> player);
    ~PlayerOSD();

    void drawOSD(std::shared_ptr<GameView>& view);
    void setMessage(OSDMessageType type, QVariant param);
    void setWeaponType(WeaponType type, bool poweredUp);
    void setAmmo(unsigned ammo);
    void setHealth(unsigned newHealth);
    void setScore(unsigned long newScore);
    void setLives(unsigned lives);
    void setSugarRushActive();
    void setLevelText(int idx);

private:
    std::weak_ptr<Player> owner;
    std::shared_ptr<BitmapString> collectionMessage;
    OSDMessageType collectionMessageType;
    std::shared_ptr<BitmapString> livesString;
    std::shared_ptr<BitmapString> scoreString;
    std::shared_ptr<BitmapString> ammoString;

    unsigned long messageTimer;
    unsigned messageOffsetAmount;
    std::shared_ptr<GraphicResource> weaponIconIdx[18];
    std::shared_ptr<GraphicResource> collectibleGraphics;

    std::shared_ptr<AnimationInstance> charIcon;
    std::shared_ptr<AnimationInstance> weaponIcon;
    std::shared_ptr<AnimationInstance> collectibleIcon;
    float collectibleIconOffset;

    sf::Texture heartTexture;

    size_t currentWeapon;
    unsigned long score;
    unsigned health;

    unsigned gemCounter;
    HSAMPLE gemSound;
    HSAMPLE notEnoughCoinsSound;

    uint sugarRushLeft;
    std::shared_ptr<BitmapString> sugarRushText;

    std::shared_ptr<BitmapString> levelTextString;
    uint levelTextFrame;
    int levelTextIdx;
};
