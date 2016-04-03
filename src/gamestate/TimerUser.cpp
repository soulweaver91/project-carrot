#include "TimerUser.h"


TimerUser::TimerUser() : animation_timer(-1l), next_timer(0) {

}

TimerUser::~TimerUser() {

}

void TimerUser::advanceTimers() {
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

void TimerUser::invokeTimer(int idx) {
    (this->*(timers[idx].second.func))();
}

unsigned long TimerUser::addTimer(unsigned frames, bool recurring, TimerCallbackFunc func) {
    ActorTimer t = { frames, 0.0, frames, 0.0, recurring, func };
    timers.append(qMakePair(next_timer, t));
    return next_timer++;
}
unsigned long TimerUser::addTimer(double frames, bool recurring, TimerCallbackFunc func) {
    unsigned long floored = qRound(floor(frames));

    ActorTimer t = { floored, frames - floored, floored, frames - floored, recurring, func };
    timers.append(qMakePair(next_timer, t));
    return next_timer++;
}

void TimerUser::cancelTimer(unsigned long idx) {
    for (int i = 0; i < timers.size(); ++i) {
        if (timers[i].first == idx) {
            timers.removeAt(i);
            return;
        }
    }
}