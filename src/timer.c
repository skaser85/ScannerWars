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

void handle_timer(Timer *timer, double default_timer_lifetime) {
  if (timer == NULL) {
    timer = alloc_timer();
    timer->timer_type = TT_COUNT_DOWN;
    timer_start(timer, default_timer_lifetime);
  }
  if (timer_done(*timer)) {
    timer = NULL;
  }
}


