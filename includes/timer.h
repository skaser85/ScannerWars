#ifndef TIMER_H
#define TIMER_H

#include "raylib.h"
#include "nob.h"

typedef enum {
  TT_NONE,
  TT_COUNT_DOWN,
  TT_COUNT_UP
} TimerType;

typedef struct {
  double start_time; // seconds
  double life_time; // seconds
  TimerType timer_type;
} Timer;

void timer_start(Timer *timer, double life_time);
bool timer_done(Timer timer);
double timer_get_elapsed(Timer timer);
Timer *alloc_timer();
const char *get_timer_text(Timer timer, size_t padding_amt);
Timer *create_timer(TimerType tt);
void handle_timer(Timer *timer);

#endif // TIMER_H
