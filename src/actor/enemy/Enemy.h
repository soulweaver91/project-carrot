#pragma once

#include <memory>
#include <QVector>
#include <QPair>

#include "../CommonActor.h"
#include "../../struct/PCEvent.h"

class ActorAPI;

enum LastHitDirection {
    NONE,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Enemy : public CommonActor {
public:
    Enemy(const ActorInstantiationDetails& initData);
    ~Enemy();
    virtual void tickEvent() override;
    virtual bool perish() override;
    bool hurtsPlayer();
    virtual void handleCollision(std::shared_ptr<CommonActor> other) override;

protected:
    bool hurtPlayer;
    bool isAttacking;
    bool canMoveToPosition(double x, double y);
    void tryGenerateRandomDrop(const QVector<QPair<PCEvent, uint>>& dropTable = Enemy::defaultDropTable);

    const static QVector<QPair<PCEvent, uint>> defaultDropTable;
    LastHitDirection lastHitDir;
};
