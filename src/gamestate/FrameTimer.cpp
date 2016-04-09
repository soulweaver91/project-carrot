#include "FrameTimer.h"

void TimerUser::animationTimerA() {
    // overridable
}

unsigned long TimerManager::addTimer(std::weak_ptr<TimerUser> callee, genericFunction function, unsigned long duration, bool recurring) {
    FrameTimer t = {duration, 0.0, duration, 0.0, recurring, callee, function};
    timers.append(qMakePair(nextIndex, t));
    return nextIndex++;
}

unsigned long TimerManager::addTimer(std::weak_ptr<TimerUser> callee, genericFunction function, double duration, bool recurring) {
    unsigned long floored = qRound(floor(duration));
    
    FrameTimer t = {floored, duration - floored, floored, duration - floored, recurring, callee, function};
    timers.append(qMakePair(nextIndex, t));
    return nextIndex++;
}

bool TimerManager::cancelTimer(unsigned long index) {
    return false;
}

void TimerManager::updateTimers() {
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

void TimerManager::invokeTimer(int internal_index) {
    if (!timers[internal_index].second.callee.expired()) {
        TimerUser* a = (timers[internal_index].second.callee.lock().get());
        (a->*(timers[internal_index].second.func))();
    }
}
