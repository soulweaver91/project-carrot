#ifndef H_CP_ENEMY
#define H_CP_ENEMY

#include "CommonActor.h"
#include "CarrotQt5.h"
#include "EventMap.h"
#include <SFML/Graphics.hpp>

class Enemy : public CommonActor {
    public:
        Enemy(CarrotQt5* root, double x = 0.0, double y = 0.0);
        ~Enemy();
        virtual void tickEvent() override;
        bool hurtsPlayer();

    protected:
        bool hurtPlayer;
        bool canMoveToPosition(double x, double y);
};

#endif
