#include "timer.h"

void timer_start(Timer *timer, double life_time) {
  timer->start_time = GetTime();
  timer->life_time = life_time;
}

bool timer_done(Timer timer) {
  return GetTime() - timer.start_time >= timer.life_time;
}

double timer_get_elapsed(Timer timer) {
  return GetTime() - timer.start_time;
}

Timer *alloc_timer() {
  Timer *t = (Timer*)malloc(sizeof(Timer));
  memset(t, 0, sizeof(*t));

  return t;
}

const char *get_timer_text(Timer timer, size_t padding_amt) {
  const char *format = temp_sprintf("%%.%ldf", padding_amt);
  // "%.0f"
  if (timer.timer_type == TT_COUNT_DOWN)
    return temp_sprintf(format, timer.life_time - timer_get_elapsed(timer));
  return temp_sprintf(format, timer_get_elapsed(timer));
}

Timer *create_timer(TimerType tt, double lifetime) {
  Timer *timer = alloc_timer();
  timer->timer_type = tt;
  timer_start(timer, lifetime);
  return timer;
}

void handle_timer(Timer *timer) {
  if (timer_done(*timer)) {
    timer = NULL;
  }
}


