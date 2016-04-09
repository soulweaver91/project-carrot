#include "TimerUser.h"


TimerUser::TimerUser() : nextTimer(0) {

}

TimerUser::~TimerUser() {

}

void TimerUser::advanceTimers() {
    for (int i = 0; i < timers.size(); ++i) {
        timers[i].second.framesLeft--;
        if (timers[i].second.framesLeft == 0) {
            invokeTimer(i);
            if (timers[i].second.recurring) {
                timers[i].second.framesRemainder += timers[i].second.initialFramesRemainder;
                while (timers[i].second.framesRemainder > 1) {
                    timers[i].second.framesRemainder -= 1.0;
                    timers[i].second.framesLeft++;
                }
                timers[i].second.framesLeft += timers[i].second.initialFramesLeft;
            } else {
                timers.removeAt(i);
                --i;
            }
        }
    }
}

void TimerUser::invokeTimer(int idx) {
    (this->*(timers[idx].second.func))();
}

unsigned long TimerUser::addTimer(unsigned frames, bool recurring, TimerCallbackFunc func) {
    ActorTimer t = { frames, 0.0, frames, 0.0, recurring, func };
    timers.append(qMakePair(nextTimer, t));
    return nextTimer++;
}
unsigned long TimerUser::addTimer(double frames, bool recurring, TimerCallbackFunc func) {
    unsigned long floored = qRound(floor(frames));

    ActorTimer t = { floored, frames - floored, floored, frames - floored, recurring, func };
    timers.append(qMakePair(nextTimer, t));
    return nextTimer++;
}

void TimerUser::cancelTimer(unsigned long idx) {
    for (int i = 0; i < timers.size(); ++i) {
        if (timers[i].first == idx) {
            timers.removeAt(i);
            return;
        }
    }
}