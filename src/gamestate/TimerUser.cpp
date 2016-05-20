#include "TimerUser.h"


TimerUser::TimerUser() : nextTimer(0) {

}

TimerUser::~TimerUser() {

}

void TimerUser::advanceTimers() {
    for (auto& timer : timers) {
        timer.second->isNew = false;
    }

    for (int i = 0; i < timers.size(); ++i) {
        if (timers[i].second->isNew) {
            continue;
        }

        timers[i].second->framesLeft--;
        if (timers[i].second->framesLeft == 0) {
            invokeTimer(i);
            if (timers[i].second->recurring) {
                timers[i].second->framesRemainder += timers[i].second->initialFramesRemainder;
                while (timers[i].second->framesRemainder > 1) {
                    timers[i].second->framesRemainder -= 1.0;
                    timers[i].second->framesLeft++;
                }
                timers[i].second->framesLeft += timers[i].second->initialFramesLeft;
            } else {
                timers.removeAt(i);
                --i;
            }
        }
    }
}

void TimerUser::invokeTimer(int idx) {
    timers[idx].second->cb();
}

unsigned long TimerUser::addTimer(unsigned frames, bool recurring, TimerLambdaCallbackFunc cb) {
    return addTimer(frames, 0.0, recurring, cb);
}

unsigned long TimerUser::addTimer(double frames, bool recurring, TimerLambdaCallbackFunc cb) {
    unsigned floored = qRound(floor(frames));
    return addTimer(floored, frames - floored, recurring, cb);
}

unsigned long TimerUser::addTimer(unsigned frames, double remainder, bool recurring, TimerLambdaCallbackFunc cb) {
    auto t = std::make_shared<TimerInstance>(frames, remainder, recurring, cb);
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