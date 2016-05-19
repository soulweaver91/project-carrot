#pragma once

#include <memory>
#include <QVector>
#include <QPair>

class TimerManager;
class TimerUser;
typedef void (TimerUser::*genericFunction)(void);

struct FrameTimer {
    // current state
    unsigned long framesLeft;
    double framesRemainder; // increased by initialFramesRemainder by every call if recurring
    // initial state
    unsigned long initialFramesLeft;
    double initialFramesRemainder;
    bool recurring;

    // call details
    std::weak_ptr<TimerUser> callee;
    genericFunction func;
};

class TimerManager {
public:
    unsigned long addTimer(std::weak_ptr<TimerUser> callee, genericFunction function, unsigned long duration, bool recurring);
    unsigned long addTimer(std::weak_ptr<TimerUser> callee, genericFunction function, double duration, bool recurring);
    bool cancelTimer(unsigned long index);
    void updateTimers();
private:
    // calls timers.value(index).func on timers.value(index).callee
    void invokeTimer(int internalIndex);
    QVector<QPair<unsigned long, FrameTimer>> timers;

    // manage indices manually to not reuse or move existing ones
    unsigned long nextIndex;
};
