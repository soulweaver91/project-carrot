#pragma once

#include <QVector>
#include <QPair>
#include <functional>
#include <memory>

class TimerUser;

typedef std::function<void()> TimerLambdaCallbackFunc;

struct TimerInstance {
    // current state
    unsigned framesLeft;
    double framesRemainder; // increased by initialFramesRemainder by every call if recurring
    // initial state
    unsigned initialFramesLeft;
    double initialFramesRemainder;
    bool recurring;
    bool isNew;
    TimerLambdaCallbackFunc cb;

    TimerInstance(unsigned framesLeft, double framesRemainder, bool recurring, TimerLambdaCallbackFunc cb)
    : framesLeft(framesLeft), framesRemainder(framesRemainder), initialFramesLeft(framesLeft),
    initialFramesRemainder(framesRemainder), recurring(recurring), isNew(true), cb(cb) {

    }
};

class TimerUser {
public:
    TimerUser();
    ~TimerUser();

    void advanceTimers();

protected:
    virtual unsigned long addTimer(double frames, bool recurring, TimerLambdaCallbackFunc cb);
    virtual unsigned long addTimer(unsigned frames, bool recurring, TimerLambdaCallbackFunc cb);
    virtual void invokeTimer(int idx);
    void cancelTimer(unsigned long idx);

    QVector<QPair<unsigned long, std::shared_ptr<TimerInstance>>> timers;
    unsigned long nextTimer;

private:
    unsigned long addTimer(unsigned frames, double remainder, bool recurring, TimerLambdaCallbackFunc cb);
};
