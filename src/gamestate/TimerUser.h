#pragma once

#include <QVector>
#include <QPair>

class TimerUser;

typedef void(TimerUser::*TimerCallbackFunc)();

struct ActorTimer {
    // current state
    unsigned long framesLeft;
    double framesRemainder; // increased by initialFramesRemainder by every call if recurring
                             // initial state
    unsigned long initialFramesLeft;
    double initialFramesRemainder;
    bool recurring;
    bool isNew;

    // callback
    TimerCallbackFunc func;
};

class TimerUser {
public:
    TimerUser();
    ~TimerUser();

    void advanceTimers();

protected:
    virtual unsigned long addTimer(double frames, bool recurring, TimerCallbackFunc func);
    virtual unsigned long addTimer(unsigned frames, bool recurring, TimerCallbackFunc func);
    virtual void invokeTimer(int idx);
    void cancelTimer(unsigned long idx);
    QVector<QPair<unsigned long, ActorTimer>> timers;
    unsigned long nextTimer;
};
