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
    if (timers[idx].second->useLambda) {
        timers[idx].second->lambda();
    } else {
        (this->*(timers[idx].second->func))();
    }
}

unsigned long TimerUser::addTimer(unsigned frames, bool recurring, TimerCallbackFunc func) {
    return addTimer(frames, 0.0, recurring, func);
}

unsigned long TimerUser::addTimer(double frames, bool recurring, TimerCallbackFunc func) {
    unsigned floored = qRound(floor(frames));
    return addTimer(floored, frames - floored, recurring, func);
}

unsigned long TimerUser::addTimer(unsigned frames, bool recurring, TimerLambdaCallbackFunc lambda) {
    return addTimer(frames, 0.0, recurring, lambda);
}

unsigned long TimerUser::addTimer(double frames, bool recurring, TimerLambdaCallbackFunc lambda) {
    unsigned floored = qRound(floor(frames));
    return addTimer(floored, frames - floored, recurring, lambda);
}

template<typename T>
unsigned long TimerUser::addTimer(unsigned frames, double remainder, bool recurring, T callback) {
    auto t = std::make_shared<ActorTimer>(frames, remainder, recurring, callback);
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