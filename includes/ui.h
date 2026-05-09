#ifndef UI_H
#define UI_H

#include <stddef.h>

#include "raylib.h"
#include "raymath.h"

typedef enum {
  BTN_NORMAL,
  BTN_DISABLED,
  BTN_SELECTED,
  BTN_COUNT
} ButtonState;

typedef struct {
  size_t pad_x;
  size_t pad_y;
  Color color;
  Color hovered_color;
  Color disabled_color;
  Color selected_color;
  size_t outline_thiccness;
  size_t font_size;
} ButtonConfig;

typedef struct {
  Vector2 pos;
  Vector2 dims;
  const char *text;
  ButtonConfig config;
  ButtonState state;
} Button;

typedef struct {
  Vector2 pos;
  const char *text;
  const char *effective_text;
  ButtonConfig button_config;
  bool incr;
  bool decr;
} Stepper;

size_t get_button_width(Button b, Font font);
size_t get_button_height(Button b, Font font);
bool draw_button(Button b, Font font);
bool draw_stepper(Stepper *stepper, Font font);

#endif // UI_H
