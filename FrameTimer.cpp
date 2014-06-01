#include "FrameTimer.h"

void TimerUser::animationTimerA() {
    // overridable
}

unsigned long TimerManager::addTimer(std::weak_ptr< TimerUser > callee, genericFunction function, unsigned long duration, bool recurring) {
    FrameTimer t = {duration, 0.0, duration, 0.0, recurring, callee, function};
    timers.append(qMakePair(next_index,t));
    return next_index++;
}
unsigned long TimerManager::addTimer(std::weak_ptr< TimerUser > callee, genericFunction function, double duration, bool recurring) {
    unsigned long floored = qRound(floor(duration));
    
    FrameTimer t = {floored, duration - floored, floored, duration - floored, recurring, callee, function};
    timers.append(qMakePair(next_index,t));
    return next_index++;
}
bool TimerManager::cancelTimer(unsigned long index) {
    return false;
}
void TimerManager::updateTimers() {
    for (int i = 0; i < timers.size(); ++i) {
        timers[i].second.frames_left--;
        if (timers[i].second.frames_left == 0) {
            invokeTimer(i);
            if (timers[i].second.recurring) {
                timers[i].second.frames_remainder += timers[i].second.frames_original_remainder;
                while (timers[i].second.frames_remainder > 1) {
                    timers[i].second.frames_remainder -= 1.0;
                    timers[i].second.frames_left++;
                }
                timers[i].second.frames_left += timers[i].second.frames_original;
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
