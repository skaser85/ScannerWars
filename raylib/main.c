#include <stdio.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define ASSETS_DIR "../assets"
#define QR_DIR ASSETS_DIR"/images/number-qrcodes"
#define DM_DIR ASSETS_DIR"/images/alphabet-data-matrix"

#define BOX_OUTLINE_THICCNESS 1
#define SPEED_MAX 5 
#define BARCODE_SIZE 75

typedef struct {
  Vector2 pos;
  Vector2 velo;
  Texture2D tex;
} Barcode;

typedef struct {
  Barcode *items;
  size_t capacity;
  size_t count;
} Barcodes;

typedef struct {
  char prefix;
  String_View *sv;
} Scan;

Rectangle get_barcode_rect(Barcode b) {
  return (Rectangle) { .x = b.pos.x, .y = b.pos.y, .width = b.tex.width, .height = b.tex.height };
}

void draw_barcode(Barcode b) {
  DrawTextureEx(b.tex, b.pos, 0, 1, WHITE);
  DrawRectangleLinesEx(get_barcode_rect(b), BOX_OUTLINE_THICCNESS, LIME);
}

void update_barcode(Barcode *me) {
  if (me->pos.x + me->velo.x < 0 || me->pos.x + me->tex.width + BOX_OUTLINE_THICCNESS + me->velo.x > GetScreenWidth()) {
    me->velo.x *= -1; 
  }


  if (me->pos.y + me->velo.y < 0 || me->pos.y + me->tex.height + BOX_OUTLINE_THICCNESS + me->velo.y> GetScreenHeight()) {
    me->velo.y *= -1; 
  }

  me->pos.x += me->velo.x;
  me->pos.y += me->velo.y;

}

int get_random_non_zero_int(int _min, int _max) {
  int v = GetRandomValue(_min, _max);
  if (v == 0) v = 1;
  return v;
}

void load_barcodes(const char* fp, Barcodes *bars) {
  FilePathList files = LoadDirectoryFiles(fp);
  for (unsigned int i = 0; i < files.count; ++i) {
    const char *f = files.paths[i];
    Image img = LoadImage(f);
    ImageResize(&img, BARCODE_SIZE, BARCODE_SIZE);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    Barcode b = (Barcode) { 
      (Vector2) { get_random_non_zero_int(100, GetScreenWidth() - 500), 
        get_random_non_zero_int(100, GetScreenHeight() - 500) },
        (Vector2) { get_random_non_zero_int(-SPEED_MAX, SPEED_MAX), 
          get_random_non_zero_int(-SPEED_MAX, SPEED_MAX) },
        tex 
    };
    da_append(bars, b);
  }
}

String_View *alloc_string_view() {
  String_View *sv = (String_View*)malloc(sizeof(String_View));
  memset(sv, 0, sizeof(*sv));

  return sv;
}

Scan *alloc_scan() {
  Scan *s = (Scan*)malloc(sizeof(Scan));
  memset(s, 0, sizeof(*s));

  s->sv = alloc_string_view();

  return s;
}

Scan *process_scan(String_Builder sb) {
  if (sb.count == 0) return NULL;
  if ((char)sb.items[0] != (char)KEY_LEFT_SHIFT) {
    nob_log(ERROR, "Invalid prefix. Expected %c. Got %c", KEY_LEFT_SHIFT, sb.items[0]);
    return NULL;
  }

  char prefix = '0';
  switch (sb.items[1]) {
    case 48: prefix = ')'; break;
    case 49: prefix = '!'; break;
    case 50: prefix = '@'; break;
    case 51: prefix = '#'; break;
    case 52: prefix = '$'; break;
    case 53: prefix = '%'; break;
    case 54: prefix = '^'; break;
    case 55: prefix = '&'; break;
    case 56: prefix = '*'; break;
    case 57: prefix = '('; break;
    default: {
      nob_log(ERROR, "Invalid prefix character. Expected one of 0-9. Got %c", sb.items[1]);
      return NULL;
    }
  }

  String_View *sv = alloc_string_view();
  sv->count = sb.count;
  sv->data = sb.items; 
  *sv = sv_chop_right(sv, sb.count-2);

  Scan *s = alloc_scan();
  s->prefix = prefix;
  s->sv = sv;
  return s;

}

int main() {

  InitWindow(1920, 1080, "Scanner Wars");
  SetTargetFPS(60);

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  SetRandomSeed(ts.tv_nsec);

  Barcodes bars = {0};
  load_barcodes(QR_DIR, &bars);
  load_barcodes(DM_DIR, &bars);

  String_Builder buff = {0};

  while (!WindowShouldClose()) {

    int k = GetKeyPressed();
    while (k > 0) {
      if (k == KEY_ENTER) {
        Scan *scan = process_scan(buff);
        if (scan) {
          nob_log(INFO, "\nPREFIX: %c\nSCAN: "SV_Fmt, scan->prefix, SV_Arg(*scan->sv));
        }
        buff.count = 0;
      } else {
        sb_append(&buff, (char)k);
      }
      k = GetKeyPressed();
    }

    da_foreach(Barcode, b, &bars) {
      update_barcode(b);
    }

    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));

    da_foreach(Barcode, b, &bars) {
      draw_barcode(*b);
    }

    EndDrawing();

  }

  da_foreach(Barcode, b, &bars) {
    UnloadTexture(b->tex);
  }

  CloseWindow();

  return 0;
}
