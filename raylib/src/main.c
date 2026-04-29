#include <stdio.h>
#include <time.h>

#include "qrcode.h"

#include "raylib.h"
#include "raymath.h"


#define NOB_IMPLEMENTATION
#include "nob.h"

#define ASSETS_DIR "../assets"
#define QR_DIR ASSETS_DIR"/images/number-qrcodes"
#define DM_DIR ASSETS_DIR"/images/alphabet-data-matrix"

#define BOX_OUTLINE_THICCNESS 1
#define SPEED_MAX 5 
#define BARCODE_SIZE 75

Color COLORS[10];

typedef struct {
  Vector2 pos;
  Vector2 velo;
  String_View *name;
  int value;
  Texture2D tex;
  bool living;
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

typedef struct {
  Vector2 pos;
  float radius;
  Color color;
  Vector2 velo;
  bool living;
} Particle;

typedef struct {
  Particle *items;
  size_t count;
  size_t capacity;
  bool living;
} Particles;

typedef struct {
  Particles *items;
  size_t count; 
  size_t capacity;
} ParticlesCollection;

typedef struct {
  size_t pad_x;
  size_t pad_y;
  Color color;
  Color hovered_color;
  size_t outline_thiccness;
  size_t font_size;
} ButtonConfig;

typedef struct {
  Vector2 pos;
  Vector2 dims;
  const char *text;
  ButtonConfig config;
} Button;

typedef enum {
  GS_MENU,
  GS_PLAYING,
  GS_COUNT
} GameState;

typedef struct {
  GameState game_state;
  Barcodes *barcodes;
  ParticlesCollection *particles;
} Game;

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

String_View *alloc_string_view(const char* text) {
  String_View *sv = (String_View*)malloc(sizeof(String_View));
  memset(sv, 0, sizeof(*sv));

  if (text) {
    sv->count = strlen(text);
    sv->data = text;
  }
  return sv;
}

Particle *alloc_particle() {
  Particle *p = (Particle*)malloc(sizeof(Particle));
  memset(p, 0, sizeof(*p));
  p->living = true;
  return p;
}

Particles *alloc_particles() {
  Particles *p = (Particles*)malloc(sizeof(Particles));
  memset(p, 0, sizeof(*p));
  p->living = true;
  return p;
}

ParticlesCollection *alloc_particles_collection() {
  ParticlesCollection *p = (ParticlesCollection*)malloc(sizeof(ParticlesCollection));
  memset(p, 0, sizeof(*p));
  return p;
}

Barcodes *alloc_barcodes() {
  Barcodes *b = (Barcodes*)malloc(sizeof(Barcodes));
  memset(b, 0, sizeof(*b));
  return b;
}

Scan *alloc_scan(const char* text) {
  Scan *s = (Scan*)malloc(sizeof(Scan));
  memset(s, 0, sizeof(*s));

  s->sv = alloc_string_view(text);

  return s;
}

Barcode *alloc_barcode() {
  Barcode *b = (Barcode*)malloc(sizeof(Barcode));
  memset(b, 0, sizeof(*b));

  return b;
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

  String_View *sv = alloc_string_view(temp_sv_to_cstr(sv_from_parts(sb.items, sb.count)));
  *sv = sv_chop_right(sv, sb.count-2);

  Scan *s = alloc_scan(temp_sv_to_cstr(*sv));
  s->prefix = prefix;
  s->sv = sv;
  return s;

}

Barcode *generate_qr_barcode(const char* text, int value) {

  const char* txt = (const char *)temp_sprintf("%s%d", text, value);

  uint8_t modules[qrcode_getBufferSize(3)];

  QRCode qrcode;
  qrcode_initText(&qrcode, modules, 3, ECC_MEDIUM, txt);

  int scale = 8;
  int border = 2;
  int side = qrcode.size + border*2;
  int imgW = side * scale;

  Image img = GenImageColor(imgW, imgW, WHITE);

  Color *pixels = (Color *)img.data;

  for (int y = 0; y < qrcode.size; y++) {
    for (int x = 0; x < qrcode.size; x++) {
      bool m = qrcode_getModule(&qrcode, x, y);
      Color c = m ? (Color){0,0,0,255} : (Color){255,255,255,255};
      int startY = (y + border) * scale;
      int startX = (x + border) * scale;
      for (int py = 0; py < scale; py++) {
        for (int px = 0; px < scale; px++) {
          int ix = startX + px;
          int iy = startY + py;
          pixels[iy * imgW + ix] = c;
        }
      }
    }
  }

  ImageResize(&img, BARCODE_SIZE, BARCODE_SIZE);

  Texture2D tex = LoadTextureFromImage(img);

  UnloadImage(img);

  Barcode *b = alloc_barcode(); 
  b->pos = (Vector2) { get_random_non_zero_int(BARCODE_SIZE, GetScreenWidth() - BARCODE_SIZE), 
    get_random_non_zero_int(BARCODE_SIZE, GetScreenHeight() - BARCODE_SIZE) };
  b->velo = (Vector2) { get_random_non_zero_int(-SPEED_MAX, SPEED_MAX), 
    get_random_non_zero_int(-SPEED_MAX, SPEED_MAX) };
  b->tex  = tex;
  b->name = alloc_string_view(txt);
  b->value = value;
  b->living = true;



  return b;
}

Barcode *kill_barcode(Scan *scan, Barcodes *bars) {
  da_foreach(Barcode, b, bars) {
    nob_log(INFO, "b->name: "SV_Fmt"scan->sv: "SV_Fmt, SV_Arg(*b->name), SV_Arg(*scan->sv));
    if (sv_eq(*b->name, *scan->sv)) {
      b->living = false;
      return b;
    }
  }
  return NULL;
}

Particles *generate_particles(Vector2 start_pos) {

  Particles *particles = alloc_particles();
  size_t particle_amount = (size_t)get_random_non_zero_int(10, 30); 

  for (size_t i = 0; i < particle_amount; ++i) {
    Particle *p = alloc_particle();
    p->pos = (Vector2) { .x = start_pos.x, .y = start_pos.y };
    p->radius = (float)get_random_non_zero_int(5, 15);
    p->color = COLORS[GetRandomValue(0, ARRAY_LEN(COLORS))];
    p->velo = (Vector2) { .x = get_random_non_zero_int(-20, 20),
      .y = get_random_non_zero_int(-20, 20) };
    p->living = true;
    da_append(particles, *p);
  }

  return particles;
}

void load_colors() {
  COLORS[0] = RED;
  COLORS[1] = BLUE;
  COLORS[2] = GREEN;
  COLORS[3] = PINK;
  COLORS[4] = LIME;
  COLORS[5] = YELLOW;
  COLORS[6] = VIOLET;
  COLORS[7] = GOLD;
  COLORS[8] = SKYBLUE;
  COLORS[9] = WHITE;
}

bool DrawButton(Button b) {
  int tw = MeasureText(b.text, b.config.font_size);
  int w = tw + b.config.pad_x*2;
  int h = b.config.font_size + b.config.pad_y*2;
  int tx = b.pos.x + b.config.pad_x;
  int ty = b.pos.y + b.config.pad_y;

  Rectangle bounds = (Rectangle) { .x = b.pos.x, .y = b.pos.y, .width = w, .height = h };
  bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
  Color color = hovered ? b.config.hovered_color : b.config.color;

  DrawText(b.text, tx, ty, b.config.font_size, color);
  if (b.config.outline_thiccness > 0) {
    DrawRectangleLinesEx(bounds, b.config.outline_thiccness, color);
  }

  return hovered;
}

size_t get_button_width(Button b) {
  int tw = MeasureText(b.text, b.config.font_size);
  int w = tw + b.config.pad_x*2;
  return (size_t)w;
}

size_t get_button_height(Button b) {
  return (size_t)b.config.font_size + b.config.pad_y*2;
}

void init_game(Game *game) {
  game->barcodes = alloc_barcodes();
  game->particles = alloc_particles_collection();
  game->game_state = GS_MENU;

  Barcode *b = generate_qr_barcode("idk", 1);
  da_append(game->barcodes, *b);
}

int main() {

  InitWindow(1920, 1080, "Scanner Wars");
  SetTargetFPS(60);

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  SetRandomSeed(ts.tv_nsec);
  SetExitKey(KEY_NULL);

  load_colors();

  int score = 0;

  String_Builder buff = {0};

  ButtonConfig default_button_config = {0};
  default_button_config.pad_x = 20;
  default_button_config.pad_y = 10;
  default_button_config.color = WHITE;
  default_button_config.hovered_color = SKYBLUE;
  default_button_config.outline_thiccness = 1;
  default_button_config.font_size = 28;

  Game game = {0};
  init_game(&game);

  while (!WindowShouldClose()) {
   
    if (game.game_state == GS_PLAYING) {
      if (IsKeyPressed(KEY_ESCAPE)) {
        game.game_state = GS_MENU;
      } else {

        int k = GetKeyPressed();
        while (k > 0) {
          if (k == KEY_ENTER) {
            //nob_log(INFO, "raw scan: "SV_Fmt, SV_Arg(sb_to_sv(buff)));
            Scan *scan = process_scan(buff);
            if (scan) {
              //nob_log(INFO, "\nPREFIX: %c\nSCAN: "SV_Fmt, scan->prefix, SV_Arg(*scan->sv));
              Barcode *b = kill_barcode(scan, game.barcodes);
              if (b) {
                score += b->value;
                da_append(game.particles, *generate_particles(b->pos));
              }
              b = generate_qr_barcode("ONE", 1);
              da_append(game.barcodes, *b);
              b = generate_qr_barcode("ONE", 1);
              da_append(game.barcodes, *b);
            }
            buff.count = 0;
          } else {
            sb_append(&buff, (char)k);
          }
          k = GetKeyPressed();
        }

        da_foreach(Barcode, b, game.barcodes) {
          if (b->living) update_barcode(b);
        }

        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        da_foreach(Barcode, b, game.barcodes) {
          if (b->living) draw_barcode(*b);
        }

        da_foreach(Particles, p, game.particles) {
          if (p->living) {
            size_t dead_count = 0;
            da_foreach(Particle, it, p) {
              if (it->living) {
                if (
                    (it->pos.x < 0 || it->pos.x > GetScreenWidth()) &&
                    (it->pos.y < 0 || it->pos.y > GetScreenHeight()) 
                   ) {
                  it->living = false;
                  dead_count++;
                } else {
                  it->pos = Vector2Add(it->pos, it->velo);
                  DrawCircleV(it->pos, it->radius, it->color);
                }
              }
            }
            if (dead_count >= p->count)
              p->living = false;
          }
        }

        DrawText(temp_sprintf("Score: %d", score), 50, 100, 28, RAYWHITE);
        
        EndDrawing();

      }
    } else if (game.game_state == GS_MENU) {
      
      BeginDrawing();
      ClearBackground(GetColor(0x360036FF));
    
      bool left_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
      
      Button start = {0};
      start.config = default_button_config;
      start.text = "Start New Game";
      start.config.outline_thiccness = 0;
      start.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(start)/2, .y = GetScreenHeight()/2-200 };
      if (DrawButton(start)) {
        if (left_clicked) {
          init_game(&game);
          game.game_state = GS_PLAYING;
        }
      }

      Button resume = {0};
      resume.config = default_button_config;
      resume.text = "Resume";
      resume.config.outline_thiccness = 0;
      resume.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(resume)/2, .y = start.pos.y + get_button_height(start) + resume.config.pad_y*4 };
      if (DrawButton(resume)) {
        if (left_clicked) {
          game.game_state = GS_PLAYING;
        }
      }
      
      Button exit_btn = {0};
      exit_btn.config = default_button_config;
      exit_btn.text = "Exit";
      exit_btn.config.outline_thiccness = 0;
      exit_btn.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(exit_btn)/2, .y = resume.pos.y + get_button_height(start) + exit_btn.config.pad_y*4 };
      if (DrawButton(exit_btn)) {
        if (left_clicked) {
          break;
        }
      }
      EndDrawing();
    }

  }

  da_foreach(Barcode, b, game.barcodes) {
    UnloadTexture(b->tex);
  }

  CloseWindow();

  return 0;
}
