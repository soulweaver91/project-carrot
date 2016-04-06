#pragma once

#include <QVector>
#include <QPair>

class TimerUser;

typedef void(TimerUser::*TimerCallbackFunc)();

struct ActorTimer {
    // current state
    unsigned long frames_left;
    double frames_remainder; // increased by frames_original_remainder by every call if recurring
                             // initial state
    unsigned long frames_original;
    double frames_original_remainder;
    bool recurring;

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
    unsigned long next_timer;
    unsigned long animation_timer;
};
