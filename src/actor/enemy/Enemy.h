#pragma once

#include <memory>
#include <QVector>
#include <QPair>

#include "../CommonActor.h"
#include "../../struct/PCEvent.h"

class CarrotQt5;

class Enemy : public CommonActor {
public:
    Enemy(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
    ~Enemy();
    virtual void tickEvent() override;
    virtual bool perish() override;
    bool hurtsPlayer();

protected:
    bool hurtPlayer;
    bool isAttacking;
    bool canMoveToPosition(double x, double y);
    void tryGenerateRandomDrop(const QVector<QPair<PCEvent, uint>>& dropTable = Enemy::defaultDropTable);

    const static QVector<QPair<PCEvent, uint>> defaultDropTable;
};
