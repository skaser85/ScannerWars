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

bool DrawButton(Button b, Font font) {
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


