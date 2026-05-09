#include "ui.h"

size_t get_button_width(Button b, Font font) {
  Vector2 td = MeasureTextEx(font, b.text, (float)font.baseSize, 1);
  int w = td.x + b.config.pad_x*2;
  return (size_t)w;
}

size_t get_button_height(Button b, Font font) {
  Vector2 td = MeasureTextEx(font, b.text, (float)font.baseSize, 1);
  return (size_t)td.y + b.config.pad_y*2;
}

bool draw_button(Button b, Font font) {
  Vector2 td = MeasureTextEx(font, b.text, font.baseSize, 1);
  int tw = td.x;
  int w = tw + b.config.pad_x*2;
  int h = font.baseSize + b.config.pad_y*2;
  int tx = b.pos.x + b.config.pad_x;
  int ty = b.pos.y + b.config.pad_y + font.baseSize/6;

  Rectangle bounds = (Rectangle) { .x = b.pos.x, .y = b.pos.y, .width = w, .height = h };
  bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

  Color color = b.config.color;
  if (b.state == BTN_DISABLED) {
    color = b.config.disabled_color;
    hovered = false;
  } else {
    if (hovered) {
      color = b.config.hovered_color;
    } else {
      if (b.state == BTN_SELECTED) {
        color = b.config.selected_color;
      }
    }
  }

  DrawTextEx(font, b.text, (Vector2){.x=tx,.y=ty}, (float)font.baseSize, 1, color);
  //DrawRectangleLinesEx(bounds, 1, color);
  if (b.config.outline_thiccness > 0) {
    DrawRectangleLinesEx(bounds, b.config.outline_thiccness, color);
  }

  return hovered;
}

bool draw_stepper(Stepper *stepper, Font font) {
  Vector2 td = MeasureTextEx(font, stepper->effective_text, (float)font.baseSize, 1); 

  Button incr = {0};
  incr.config = stepper->button_config;
  incr.text = "+";
  
  Button decr = {0};
  decr.config = stepper->button_config;
  decr.text = "-";

  size_t w = get_button_width(incr, font)*2+decr.config.pad_x*4+td.x;

  decr.pos = (Vector2) { .x = stepper->pos.x-w/2, .y = stepper->pos.y};
  incr.pos = (Vector2) { 
                          .x = decr.pos.x+
                               get_button_width(incr, font)+
                               decr.config.pad_x, 
                          .y = stepper->pos.y
                        };
 
  stepper->decr = draw_button(decr, font);
  stepper->incr = draw_button(incr, font);

  DrawTextEx(font, stepper->text, (Vector2){.x=incr.pos.x+get_button_width(incr, font)+incr.config.pad_x*2,.y=stepper->pos.y+font.baseSize/2.5}, font.baseSize, 1, WHITE);
  
  return stepper->decr || stepper->incr;
}
