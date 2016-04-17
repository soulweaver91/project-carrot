#pragma once

#include <memory>
#include <QVariant>
#include <SFML/Graphics.hpp>
#include "../gamestate/AnimationUser.h"
#include "../graphics/BitmapFont.h"
#include "../struct/WeaponTypes.h"
#include "../struct/Resources.h"

class CarrotQt5;
class Player;

enum OSDMessageType {
    OSD_NONE,
    OSD_CUSTOM_TEXT,
    OSD_GEM_RED,
    OSD_GEM_GREEN,
    OSD_GEM_BLUE,
    OSD_GEM_PURPLE,
    OSD_COIN_GOLD,
    OSD_COIN_SILVER,
    OSD_BONUS_WARP_NOT_ENOUGH_COINS
};

class PlayerOSD : public AnimationUser {
public:
    PlayerOSD(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> player, std::weak_ptr<sf::RenderWindow> canvas);
    ~PlayerOSD();

    void drawOSD();
    void clearMessage();
    void setMessage(OSDMessageType type, QVariant param);
    void setWeaponType(WeaponType type, bool poweredUp);
    void setAmmo(unsigned ammo);
    void setHealth(unsigned newHealth);
    void setScore(unsigned long newScore);
    void setLives(unsigned lives);

private:
    std::weak_ptr<Player> owner;
    std::weak_ptr<sf::RenderWindow> canvas;
    std::unique_ptr<BitmapString> collectionMessage;
    OSDMessageType collectionMessageType;
    std::unique_ptr<BitmapString> livesString;
    std::unique_ptr<BitmapString> scoreString;
    std::unique_ptr<BitmapString> ammoString;

    unsigned long messageTimer;
    unsigned messageOffsetAmount;
    std::shared_ptr<GraphicResource> weaponIconIdx[9];
    std::shared_ptr<GraphicResource> collectibleGraphics;

    std::shared_ptr<AnimationInstance> charIcon;
    std::shared_ptr<AnimationInstance> weaponIcon;
    std::shared_ptr<AnimationInstance> collectibleIcon;

    sf::Texture heartTexture;

    size_t currentWeapon;
    unsigned long score;
    unsigned health;

    unsigned gemCounter;
    HSAMPLE gemSound;

};
