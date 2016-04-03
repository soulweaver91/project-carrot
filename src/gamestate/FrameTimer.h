#pragma once

#include <memory>
#include <QList>
#include <QPair>

class TimerManager;

class TimerUser {
    friend TimerManager;
    protected:
        virtual void animationTimerA();
};
typedef void (TimerUser::*genericFunction)(void);

struct FrameTimer {
    // current state
    unsigned long frames_left;
    double frames_remainder; // increased by frames_original_remainder by every call if recurring
    // initial state
    unsigned long frames_original;
    double frames_original_remainder;
    bool recurring;

    // call details
    std::weak_ptr< TimerUser > callee;
    genericFunction func;
};

class TimerManager {
    public:
        unsigned long addTimer(std::weak_ptr< TimerUser > callee, genericFunction function, unsigned long duration, bool recurring);
        unsigned long addTimer(std::weak_ptr< TimerUser > callee, genericFunction function, double duration, bool recurring);
        bool cancelTimer(unsigned long index);
        void updateTimers();
    private:
        // calls timers.value(index).func on timers.value(index).callee
        void invokeTimer(int internal_index);
        QList< QPair<unsigned long, FrameTimer> > timers;

        // manage indices manually to not reuse or move existing ones
        unsigned long next_index;
};
