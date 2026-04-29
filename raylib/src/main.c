#include <stdio.h>
#include <time.h>

// qrcode needs to go before raylib otherwise weird errors happen
#include "qrcode.h"

#include "raylib.h"
#include "raymath.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

#define ASSETS_DIR "../assets"
#define IMAGES_DIR ASSETS_DIR"/images"
#define FONTS_DIR ASSETS_DIR"/fonts"
#define SOUNDS_DIR ASSETS_DIR"/sounds"

#define QR_DIR IMAGES_DIR"/number-qrcodes"
#define DM_DIR IMAGTES_DIR"/alphabet-data-matrix"

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
  Color tint;
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

typedef struct {
  char prefix;
  int score;
} Player;

typedef struct {
  Sound *items;
  size_t capacity;
  size_t count;
} Sounds;

typedef enum {
  GS_MENU,
  GS_PLAYING,
  GS_COUNT
} GameState;

typedef struct {
  GameState game_state;
  Barcodes *barcodes;
  ParticlesCollection *particles;
  Player *p1;
  Player *p2;
  Font title_font;
  Font item_font;
  Font player_font;
  const char* title;
  Sound boom;
  Sounds *sounds;
  bool brain_rot;
} Game;

Rectangle get_barcode_rect(Barcode b) {
  return (Rectangle) { .x = b.pos.x, .y = b.pos.y, .width = b.tex.width, .height = b.tex.height };
}

void draw_barcode(Barcode b) {
  DrawTextureEx(b.tex, b.pos, 0, 1, b.tint);
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

Player *alloc_player() {
  Player *p = (Player*)malloc(sizeof(Player));
  memset(p, 0, sizeof(*p));

  return p;
}

Sounds *alloc_sounds() {
  Sounds *s = (Sounds*)malloc(sizeof(Sounds));
  memset(s, 0, sizeof(*s));

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

  String_View *sv = alloc_string_view(temp_sv_to_cstr(sv_from_parts(sb.items, sb.count)));
  *sv = sv_chop_right(sv, sb.count-2);

  Scan *s = alloc_scan(temp_sv_to_cstr(*sv));
  s->prefix = prefix;
  s->sv = sv;
  return s;

}

size_t get_id() {
  static size_t id = 0;
  return ++id;
}

Barcode *generate_qr_barcode(int value) {

  const char* txt = (const char *)temp_sprintf("%ld%d", get_id(), value);

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
    nob_log(INFO, "\nb->name: "SV_Fmt"\nscan->sv: "SV_Fmt, SV_Arg(*b->name), SV_Arg(*scan->sv));
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

bool DrawButton(Button b, Font font) {
  Vector2 td = MeasureTextEx(font, b.text, font.baseSize, 1);
  int tw = td.x;
  int w = tw + b.config.pad_x*2;
  int h = font.baseSize + b.config.pad_y*2;
  int tx = b.pos.x + b.config.pad_x;
  int ty = b.pos.y + b.config.pad_y + font.baseSize/6;

  Rectangle bounds = (Rectangle) { .x = b.pos.x, .y = b.pos.y, .width = w, .height = h };
  bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
  Color color = hovered ? b.config.hovered_color : b.config.color;

  DrawTextEx(font, b.text, (Vector2){.x=tx,.y=ty}, (float)font.baseSize, 1, color);
  //DrawRectangleLinesEx(bounds, 1, color);
  if (b.config.outline_thiccness > 0) {
    DrawRectangleLinesEx(bounds, b.config.outline_thiccness, color);
  }

  return hovered;
}

size_t get_button_width(Button b, Font font) {
  Vector2 td = MeasureTextEx(font, b.text, (float)font.baseSize, 1);
  int w = td.x + b.config.pad_x*2;
  return (size_t)w;
}

size_t get_button_height(Button b, Font font) {
  Vector2 td = MeasureTextEx(font, b.text, (float)font.baseSize, 1);
  return (size_t)td.y + b.config.pad_y*2;
}

void init_game(Game *game) {
  game->barcodes = alloc_barcodes();
  game->particles = alloc_particles_collection();
  game->game_state = GS_MENU;
  game->p1 = alloc_player();
  game->p2 = alloc_player();

  game->p1->prefix = '#';
  game->p2->prefix = '@';

  Barcode *b = generate_qr_barcode(1);
  b->tint = WHITE;
  da_append(game->barcodes, *b);
  
  b = generate_qr_barcode(1);
  b->tint = PINK; 
  da_append(game->barcodes, *b);

  game->sounds = alloc_sounds();
  FilePathList soundFiles = LoadDirectoryFiles(SOUNDS_DIR);
  for (size_t i = 0; i < soundFiles.count; ++i){
    if (strcmp("boom.mp3", GetFileName(soundFiles.paths[i])) != 0) {
      da_append(game->sounds, LoadSound(soundFiles.paths[i]));
    }
  }
}

Font init_font(const char* fp, int font_size, int *codepoints, int codepoint_count) {
  if (FileExists(fp)) 
    return LoadFontEx(fp, font_size, codepoints, codepoint_count);
  return GetFontDefault();
}

Sound get_random_sound(Sounds *sounds) {
  return sounds->items[GetRandomValue(0, sounds->count-1)];
}

int main() {

  InitWindow(1920, 1080, "Scanner Wars");
  SetTargetFPS(60);
  InitAudioDevice();

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  SetRandomSeed(ts.tv_nsec);
  SetExitKey(KEY_NULL);

  load_colors();

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
  game.title_font = init_font(FONTS_DIR"/hellbone/Hellbone-Demo.otf", 200, NULL, 0);
  game.player_font = init_font(FONTS_DIR"/asteroid_blaster/Asteroid Blaster.ttf", 200, NULL, 0);
  game.item_font = init_font(FONTS_DIR"/asteroid_blaster/Asteroid Blaster.ttf", 64, NULL, 0);
  game.title = "SCANNER WARS";
  game.boom = LoadSound(SOUNDS_DIR"/boom.mp3");
  game.brain_rot = false;

  while (!WindowShouldClose()) {
   
    if (game.game_state == GS_PLAYING) {
      if (IsKeyPressed(KEY_ESCAPE)) {
        game.game_state = GS_MENU;
      } else {

        int k = GetKeyPressed();
        while (k > 0) {
          if (k == KEY_ENTER) {
            //nob_log(INFO, "raw scan: \n"SV_Fmt, SV_Arg(sb_to_sv(buff)));
            Scan *scan = process_scan(buff);
            if (scan) {
              //nob_log(INFO, "\nPREFIX: %c\nSCAN: "SV_Fmt, scan->prefix, SV_Arg(*scan->sv));
              Barcode *b = kill_barcode(scan, game.barcodes);
              if (b) {
                //nob_log(INFO, "x: %f, y: %f", b->pos.x, b->pos.y);
                da_append(game.particles, *generate_particles(b->pos));
                if (game.brain_rot) {
                  Sound s = get_random_sound(game.sounds);
                  PlaySound(s);
                } else {
                  PlaySound(game.boom);
                }
                if (scan->prefix == game.p1->prefix) {
                  game.p1->score += b->value;
                  b = generate_qr_barcode(1);
                  b->tint = WHITE;
                  da_append(game.barcodes, *b);
                  b = generate_qr_barcode(1);
                  b->tint = WHITE;
                  da_append(game.barcodes, *b);
                } else if (scan->prefix == game.p2->prefix) {
                  game.p2->score += b->value;
                  b = generate_qr_barcode(1);
                  b->tint = PINK;
                  da_append(game.barcodes, *b);
                  b = generate_qr_barcode(1);
                  b->tint = PINK;
                  da_append(game.barcodes, *b);
                }
              }
              
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

        DrawTextEx(game.player_font, temp_sprintf("Player 1 Score: %d", game.p1->score), (Vector2){.x=50,.y=100}, 28, 1, RAYWHITE);
        DrawTextEx(game.player_font, temp_sprintf("Player 2 Score: %d", game.p2->score), (Vector2){.x=50,.y=150}, 28, 1, RAYWHITE);
        
        EndDrawing();

      }
    } else if (game.game_state == GS_MENU) {
      
      BeginDrawing();
      ClearBackground(GetColor(0x360036FF));
      
      Vector2 td = MeasureTextEx(game.title_font, game.title, game.title_font.baseSize, 1);
      Color color = (int)GetTime() % 2 == 0 ? RED : WHITE;
      DrawTextEx(game.title_font, game.title, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=100}, 200, 1, color); 

      bool left_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
      
      Button start = {0};
      start.config = default_button_config;
      start.text = "Start New Game";
      start.config.outline_thiccness = 0;
      start.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(start, game.item_font)/2, .y = 400};
      if (DrawButton(start, game.item_font)) {
        if (left_clicked) {
          init_game(&game);
          game.game_state = GS_PLAYING;
        }
      }

      Button resume = {0};
      resume.config = default_button_config;
      resume.text = "Resume";
      resume.config.outline_thiccness = 0;
      resume.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(resume, game.item_font)/2, .y = start.pos.y + get_button_height(start, game.item_font) + resume.config.pad_y*4 };
      if (DrawButton(resume, game.item_font)) {
        if (left_clicked) {
          game.game_state = GS_PLAYING;
        }
      }
      
      Button brain_rot_btn = {0};
      brain_rot_btn.config = default_button_config;
      if (game.brain_rot) {
        brain_rot_btn.text = "Deactivate Brain Rot";
      } else {
        brain_rot_btn.text = "Activate Brain Rot";
      }
      brain_rot_btn.config.outline_thiccness = 0;
      brain_rot_btn.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(brain_rot_btn, game.item_font)/2, .y = resume.pos.y + get_button_height(start, game.item_font) + brain_rot_btn.config.pad_y*4 };
      if (DrawButton(brain_rot_btn, game.item_font)) {
        if (left_clicked) {
          game.brain_rot = !game.brain_rot;
        }
      }

      Button exit_btn = {0};
      exit_btn.config = default_button_config;
      exit_btn.text = "Exit";
      exit_btn.config.outline_thiccness = 0;
      exit_btn.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(exit_btn, game.item_font)/2, .y = brain_rot_btn.pos.y + get_button_height(brain_rot_btn, game.item_font) + exit_btn.config.pad_y*4 };
      if (DrawButton(exit_btn, game.item_font)) {
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

  UnloadFont(game.title_font);
  UnloadFont(game.item_font);
  UnloadFont(game.player_font);

  UnloadSound(game.boom);

  CloseWindow();

  return 0;
}
