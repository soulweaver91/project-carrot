#pragma once

#include <memory>
#include <QVector>
#include <QPair>

#include "../CommonActor.h"
#include "../../struct/PCEvent.h"

class ActorAPI;

class Enemy : public CommonActor {
public:
    Enemy(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
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
};
