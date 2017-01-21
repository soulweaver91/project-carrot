#pragma once

#include <memory>
#include <QVariant>
#include <SFML/Graphics.hpp>
#include "../gamestate/AnimationUser.h"
#include "../struct/WeaponTypes.h"
#include "../struct/Resources.h"
#include "../struct/SequentialIDList.h"
#include "../graphics/BitmapString.h"

class CarrotQt5;
class Player;

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

struct SpecialOSDText {
    // as percentages
    sf::Vector2f position;
    std::shared_ptr<BitmapString> text;
    std::function<void(SpecialOSDText&)> updateFunc;
    uint frame;

    SpecialOSDText(sf::Vector2f p, std::shared_ptr<BitmapString> t, std::function<void(SpecialOSDText&)> cb) : frame(0) {
        position = p;
        text = t;
        updateFunc = cb;
    }
};

struct SpecialOSDGraphics {
    // as percentages
    sf::Vector2f position;
    std::shared_ptr<AnimationInstance> anim;
    std::function<void(SpecialOSDGraphics&)> updateFunc;
    uint frame;

    SpecialOSDGraphics(sf::Vector2f p, std::shared_ptr<AnimationInstance> a, std::function<void(SpecialOSDGraphics&)> cb) : frame(0) {
        position = p;
        anim = a;
        updateFunc = cb;
    }
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
    void initLevelStartOverlay();
    void initLevelCompletedOverlay(uint redGems, uint greenGems, uint blueGems, uint purpleGems);

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

    int createOverlayText(uint lifetime, sf::Vector2f position, const QString& text, std::function<void(SpecialOSDText&)> updateFunc,
                          FontAlign align = FONT_ALIGN_CENTER, double vx = 0.0, double vy = 0.0, double s = 0.0, double a = 0.0);
    int createOverlayGraphics(uint lifetime, sf::Vector2f position, std::shared_ptr<GraphicResource> anim, std::function<void(SpecialOSDGraphics&)> updateFunc);

    SequentialIDList<SpecialOSDText> specialOverlayTexts;
    SequentialIDList<SpecialOSDGraphics> specialOverlayGraphics;

    template<typename T>
    std::function<void(T&)> getSlideInUpdateFunc(uint startFrame, uint endFrame, double finalX);

    static const QStringList levelStartStrings;
};
