#pragma once

#include <QVector>
#include <QPair>
#include <functional>
#include <memory>

class TimerUser;

typedef void(TimerUser::*TimerCallbackFunc)();
typedef std::function<void()> TimerLambdaCallbackFunc;

struct ActorTimer {
    // current state
    unsigned framesLeft;
    double framesRemainder; // increased by initialFramesRemainder by every call if recurring
    // initial state
    unsigned initialFramesLeft;
    double initialFramesRemainder;
    bool recurring;
    bool isNew;

    // callback
    bool useLambda;
    TimerCallbackFunc func;
    TimerLambdaCallbackFunc lambda;

    ActorTimer(unsigned framesLeft, double framesRemainder, bool recurring, TimerCallbackFunc func)
    : framesLeft(framesLeft), initialFramesLeft(framesLeft), framesRemainder(framesRemainder),
    initialFramesRemainder(framesRemainder), recurring(recurring), isNew(true), func(func), useLambda(false) {

    }

    ActorTimer(unsigned framesLeft, double framesRemainder, bool recurring, TimerLambdaCallbackFunc lambda)
    : framesLeft(framesLeft), initialFramesLeft(framesLeft), framesRemainder(framesRemainder),
    initialFramesRemainder(framesRemainder), recurring(recurring), isNew(true), lambda(lambda), useLambda(true) {

    }
};

class TimerUser {
public:
    TimerUser();
    ~TimerUser();

    void advanceTimers();

protected:
    virtual unsigned long addTimer(double frames, bool recurring, TimerCallbackFunc func);
    virtual unsigned long addTimer(unsigned frames, bool recurring, TimerCallbackFunc func);
    virtual unsigned long addTimer(double frames, bool recurring, TimerLambdaCallbackFunc func);
    virtual unsigned long addTimer(unsigned frames, bool recurring, TimerLambdaCallbackFunc func);
    virtual void invokeTimer(int idx);
    void cancelTimer(unsigned long idx);
    QVector<QPair<unsigned long, std::shared_ptr<ActorTimer>>> timers;
    unsigned long nextTimer;

private:
    template<typename T>
    unsigned long addTimer(unsigned frames, double remainder, bool recurring, T callback);
};
