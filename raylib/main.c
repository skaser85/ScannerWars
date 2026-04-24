#include <stdio.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define SPEED_MAX 2 

typedef struct {
  Rectangle rect;
  Vector2 velo;
  const char* name; // temporary until we get images going
} Barcode;

typedef struct {
  Barcode *items;
  size_t capacity;
  size_t count;
} Barcodes;

void draw_barcode(Barcode b) {
  DrawRectangleLinesEx(b.rect, 1, LIME);
}

void update_barcode(Barcode *me) {
  if (me->rect.x + me->velo.x < 0 || me->rect.x + me->rect.width + me->velo.x > GetScreenWidth()) {
    me->velo.x *= -1; 
  }
 
  
  if (me->rect.y + me->velo.y < 0 || me->rect.y + me->rect.height + me->velo.y> GetScreenHeight()) {
    me->velo.y *= -1; 
  }

  me->rect.x += me->velo.x;
  me->rect.y += me->velo.y;
  
}

void check_barcode_collisions(Barcode *me, Barcodes *bars) {
  da_foreach(Barcode, b, bars) {
    if (me != b) {
      if (CheckCollisionRecs(me->rect, b->rect)) {
        Rectangle cRec = GetCollisionRec(me->rect, b->rect);
        //DrawRectangleRec(cRec, RED);
        if (cRec.width >= cRec.height) {
          me->velo.x *= -1;
          b->velo.x *= -1;
          if (me->velo.x < 0) {
            me->rect.x -= cRec.width;
          } else {
            me->rect.x += cRec.width;
          }
          if (b->velo.x < 0) {
            b->rect.x -= cRec.width;
          } else {
            b->rect.x += cRec.width;
          }
        } else {
          me->velo.y *= -1;
          b->velo.y *= -1;
          if (me->velo.y < 0) {
            me->rect.y -= cRec.height;
          } else {
            me->rect.y += cRec.height;
          }
          if (b->velo.y < 0) {
            b->rect.y -= cRec.height;
          } else {
            b->rect.y += cRec.height;
          }
        }
      }
    }
  }
}

int get_random_non_zero_int(int _min, int _max) {
  int v = GetRandomValue(_min, _max);
  if (v == 0) v = 1;
  return v;
}

int main() {
  
  InitWindow(800, 600, "Scanner Wars");
  SetTargetFPS(60);
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  SetRandomSeed(ts.tv_nsec);
  
  Barcodes bars = {0};
  size_t temp_bc_size = 50;
  for (size_t i = 0; i < 5; ++i) {
    Barcode b = (Barcode) { (Rectangle) { i*temp_bc_size+20*i, 100,
                                          temp_bc_size, temp_bc_size},
                            (Vector2) { get_random_non_zero_int(-SPEED_MAX, SPEED_MAX), 
                                        get_random_non_zero_int(-SPEED_MAX, SPEED_MAX) },
                            temp_sprintf("%ld", i)};
    da_append(&bars, b);
    nob_log(INFO, "Velo: %f, %f", b.velo.x, b.velo.y);
  }
  
  while (!WindowShouldClose()) {

    da_foreach(Barcode, b, &bars) {
      update_barcode(b);
    }

    da_foreach(Barcode, b, &bars) {
      check_barcode_collisions(b, &bars);
    }

    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));

    da_foreach(Barcode, b, &bars) {
      draw_barcode(*b);
    }
    
    EndDrawing();

  }

  CloseWindow();

  return 0;
}
