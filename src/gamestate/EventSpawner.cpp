#include "EventSpawner.h"

#include "../gamestate/LevelManager.h"
#include "../actor/collectible/Collectible.h"
#include "../actor/collectible/AmmoCollectible.h"
#include "../actor/collectible/CoinCollectible.h"
#include "../actor/collectible/FastFireCollectible.h"
#include "../actor/collectible/FoodCollectible.h"
#include "../actor/collectible/CarrotCollectible.h"
#include "../actor/enemy/Enemy.h"
#include "../actor/enemy/NormalTurtle.h"
#include "../actor/enemy/LabRat.h"
#include "../actor/enemy/Lizard.h"
#include "../actor/enemy/TurtleShell.h"
#include "../actor/enemy/Sucker.h"
#include "../actor/enemy/SuckerFloat.h"
#include "../actor/lighting/StaticLight.h"
#include "../actor/lighting/PulsatingLight.h"
#include "../actor/solidobj/SolidObject.h"
#include "../actor/solidobj/PushBox.h"
#include "../actor/solidobj/TriggerCrate.h"
#include "../actor/solidobj/Bridge.h"
#include "../actor/solidobj/MovingPlatform.h"
#include "../actor/solidobj/PowerUpMonitor.h"
#include "../actor/solidobj/CrateContainer.h"
#include "../actor/solidobj/BarrelContainer.h"
#include "../actor/solidobj/AmmoBarrel.h"
#include "../actor/solidobj/AmmoCrate.h"
#include "../actor/solidobj/GemBarrel.h"
#include "../actor/solidobj/GemCrate.h"
#include "../actor/Player.h"
#include "../actor/SavePoint.h"
#include "../actor/Spring.h"
#include "../actor/BonusWarp.h"
#include "../struct/WeaponTypes.h"

EventSpawner::EventSpawner() {

}

EventSpawner::~EventSpawner() {

}

bool EventSpawner::initializeSpawnableList() {
    for (uint eventNo = static_cast<uint>(PC_AMMO_BOUNCER); eventNo <= static_cast<uint>(PC_AMMO_ELECTRO); ++eventNo) {
        registerTrivialSpawnable<AmmoCollectible>(static_cast<PCEvent>(eventNo), "Object/Collectible",
            static_cast<WeaponType>(eventNo - static_cast<uint>((PC_AMMO_BOUNCER)) + 1));
    }

    for (uint eventNo = static_cast<uint>(PC_GEM_RED); eventNo <= static_cast<uint>(PC_GEM_PURPLE); ++eventNo) {
        registerTrivialSpawnable<GemCollectible>(static_cast<PCEvent>(eventNo), "Object/Collectible",
            static_cast<GemType>((eventNo - static_cast<uint>(PC_GEM_RED))));
    }

    registerTrivialSpawnable<CoinCollectible>(PC_COIN_SILVER, "Object/Collectible", COIN_SILVER);
    registerTrivialSpawnable<CoinCollectible>(PC_COIN_GOLD, "Object/Collectible", COIN_GOLD);
    registerTrivialSpawnable<FastFireCollectible>(PC_FAST_FIRE, "Object/Collectible");
    registerTrivialSpawnable<EnemyNormalTurtle>(PC_ENEMY_TURTLE_NORMAL, "Enemy/Turtle");
    registerTrivialSpawnable<EnemyLizard>(PC_ENEMY_LIZARD, "Enemy/Lizard");
    registerTrivialSpawnable<EnemySuckerFloat>(PC_ENEMY_SUCKER_FLOAT, "Enemy/SuckerFloat");
    registerTrivialSpawnable<EnemySucker>(PC_ENEMY_SUCKER, "Enemy/Sucker");
    registerTrivialSpawnable<EnemyLabRat>(PC_ENEMY_LAB_RAT, "Enemy/LabRat");
    registerTrivialSpawnable<SavePoint>(PC_SAVE_POINT, "Object/SavePoint");

    registerSpawnable(PC_PUSHABLE_ROCK, "Object/PushBox", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<PushBox>(fromEventMap, x, y, params[0]);
    });

    registerSpawnable(PC_TRIGGER_CRATE, "Object/TriggerCrate", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<TriggerCrate>(fromEventMap, x, y, params[0]);
    });

    registerSpawnable(PC_BRIDGE, "Object/BridgePiece", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<DynamicBridge>(fromEventMap, x, y, params[0], static_cast<DynamicBridgeType>(params[1]), params[2]);
    });

    for (uint eventNo = static_cast<uint>(PC_SPRING_RED); eventNo <= static_cast<uint>(PC_SPRING_BLUE); ++eventNo) {
        registerSpawnable(static_cast<PCEvent>(eventNo), "Object/Spring", 
            [this, eventNo](bool fromEventMap, int x, int y, const quint16 params[]) {
            return createCommonActorEvent<Spring>(fromEventMap, x, y, static_cast<SpringType>(1 + (eventNo - PC_SPRING_RED)),
                static_cast<unsigned char>(params[0]));
        });
    }

    registerSpawnable(PC_MOVING_PLATFORM, "Object/MovingPlatform", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<MovingPlatform>(fromEventMap, x, y,
            static_cast<PlatformType>(params[0]), params[3], static_cast<qint16>(params[2]), params[1], params[4] != 0);
    });

    for (uint eventNo = static_cast<uint>(PC_FOOD_APPLE); eventNo <= static_cast<uint>(PC_FOOD_CHEESE); ++eventNo) {
        registerTrivialSpawnable<FoodCollectible>(static_cast<PCEvent>(eventNo), "Object/Collectible", static_cast<PCEvent>(eventNo));
    }

    registerTrivialSpawnable<TurtleShell>(PC_TURTLE_SHELL, "Enemy/TurtleShell", 0.0, 0.0);
    registerTrivialSpawnable<CarrotCollectible>(PC_CARROT, "Object/Collectible", false);
    registerTrivialSpawnable<CarrotCollectible>(PC_CARROT_FULL, "Object/Collectible", true);
    registerSpawnable(PC_WARP_COIN_BONUS, "Object/BonusWarp", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<BonusWarp>(fromEventMap, x, y, params);
    });

    for (uint eventNo = static_cast<uint>(PC_POWERUP_BLASTER); eventNo <= static_cast<uint>(PC_POWERUP_ELECTRO); ++eventNo) {
        registerTrivialSpawnable<PowerUpMonitor>(static_cast<PCEvent>(eventNo), "Object/PowerUpMonitor",
            static_cast<WeaponType>((eventNo - static_cast<uint>(PC_POWERUP_BLASTER))));
    }

    registerSpawnable(PC_LIGHT_STEADY, "Object/NoResources", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<StaticLight>(fromEventMap, x, y, params[0]);
    });

    registerSpawnable(PC_LIGHT_PULSE, "Object/NoResources", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<PulsatingLight>(fromEventMap, x, y, params[0], params[1], params[2]);
    });

    registerSpawnable(PC_CRATE_AMMO, "Object/CrateContainer", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<AmmoCrate>(fromEventMap, x, y, params[0] < WEAPONCOUNT ? (WeaponType)params[0] : (WeaponType)0);
    });

    registerSpawnable(PC_BARREL_AMMO, "Object/BarrelContainer", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<AmmoBarrel>(fromEventMap, x, y, params[0] < WEAPONCOUNT ? (WeaponType)params[0] : (WeaponType)0);
    });

    registerSpawnable(PC_CRATE_GENERAL, "Object/CrateContainer", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<CrateContainer>(fromEventMap, x, y, (PCEvent)params[0], params[1]);
    });

    registerSpawnable(PC_BARREL_GENERAL, "Object/BarrelContainer", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<BarrelContainer>(fromEventMap, x, y, (PCEvent)params[0], params[1]);
    });

    registerSpawnable(PC_CRATE_GEM, "Object/CrateContainer", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<GemCrate>(fromEventMap, x, y, params[0], params[1], params[2], params[3]);
    });

    registerSpawnable(PC_BARREL_GEM, "Object/BarrelContainer", [this](bool fromEventMap, int x, int y, const quint16 params[]) {
        return createCommonActorEvent<GemBarrel>(fromEventMap, x, y, params[0], params[1], params[2], params[3]);
    });

    return true;
}

void EventSpawner::setApi(apiHandle newApi) {
    api = newApi;
}

void EventSpawner::registerSpawnable(PCEvent type, const QString& resourceName, SpawnerFunction spawner) {
    spawnableEvents.insert(type, { type, resourceName, spawner });
}

std::shared_ptr<CommonActor> EventSpawner::spawnEvent(bool fromEventMap, PCEvent type, int x, int y, const quint16 params[8]) const {
    if (spawnableEvents.contains(type)) {
        auto apiPtr = api.lock();
        if (apiPtr != nullptr) {
            return spawnableEvents.value(type).spawner(fromEventMap, x, y, params);
        }
    }

    return nullptr;
}

QString EventSpawner::getEventResourceName(PCEvent type) const {
    if (spawnableEvents.contains(type)) {
        return spawnableEvents.value(type).identifier;
    }

    return "Object/NoResources";
}

template<typename T, typename... P>
void EventSpawner::registerTrivialSpawnable(PCEvent type, const QString& resourceName, P... params) {
    registerSpawnable(type, resourceName, [this, params...](bool fromEventMap, int x, int y, const quint16[]) {
        return createCommonActorEvent<T, P...>(fromEventMap, x, y, *&params...);
    });
}

template<typename T, typename... P>
std::shared_ptr<CommonActor> EventSpawner::createCommonActorEvent(bool fromEventMap, int x, int y, P... params) {
    auto apiPtr = api.lock();
    if (apiPtr != nullptr) {
        return std::make_shared<T>(ActorInstantiationDetails(
            apiPtr,
            { 32.0 * x + 16.0, 32.0 * y + 16.0 },
            fromEventMap
        ), *&params...);
    }

    return nullptr;
}