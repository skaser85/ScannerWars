#include <time.h>

#include "utils.h"
#include "ui.h"
#include "barcode.h"
#include "particles.h"
#include "timer.h"

#include "qrcode.h"

#include "raylib.h"
#include "raymath.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

#define ASSETS_DIR "./assets"
#define IMAGES_DIR ASSETS_DIR"/images"
#define FONTS_DIR ASSETS_DIR"/fonts"
#define SOUNDS_DIR ASSETS_DIR"/sounds"

#define QR_DIR IMAGES_DIR"/number-qrcodes"
#define DM_DIR IMAGTES_DIR"/alphabet-data-matrix"

#define DEFAULT_ROUND_TIME_SECS 10

typedef struct {
  char prefix;
  int score;
} Player;

typedef enum {
  GS_MENU,
  GS_SELECT_GM,
  GS_PLAYING,
  GS_GAME_OVER,
  GS_COUNT
} GameState;

typedef enum {
  GM_2P_VERSUS,
  GM_1P_SPEED_RUN,
  GM_1P_INFINITE,
  GM_COUNT
} GameMode;

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
  Timer *round_timer;
  String_Builder *scan_buffer;
  ButtonConfig default_button_config;
  GameMode game_mode;
  Colors colors;
  size_t round_time_secs;
} Game;

const char *get_game_mode_text(GameMode gm) {
  switch (gm) {
    case GM_2P_VERSUS: return "2P Versus";
    case GM_1P_SPEED_RUN: return "1P Speed Run";
    case GM_1P_INFINITE: return "1P Infinite";
    case GM_COUNT: return "you shouldn't be seeing this (GM_COUNT)";
    default: return temp_sprintf("idk, mang (GameMode: %d)", gm); 
  }
}

Player *alloc_player() {
  Player *p = (Player*)malloc(sizeof(Player));
  memset(p, 0, sizeof(*p));

  return p;
}

void init_game(Game *game) {
  game->barcodes = alloc_barcodes();
  game->particles = alloc_particles_collection();
  game->game_state = GS_MENU;
  game->p1 = alloc_player();
  game->p2 = alloc_player();

  game->p1->prefix = '#';
  game->p2->prefix = '@';

  if (game->sounds == NULL) {
    game->sounds = alloc_sounds();
    FilePathList soundFiles = LoadDirectoryFiles(SOUNDS_DIR);
    for (size_t i = 0; i < soundFiles.count; ++i){
      if (strcmp("boom.mp3", GetFileName(soundFiles.paths[i])) != 0) {
        da_append(game->sounds, LoadSound(soundFiles.paths[i]));
      }
    }
  }

  game->round_timer = NULL;

  game->scan_buffer = alloc_string_builder();

  da_append(&game->colors, RED);
  da_append(&game->colors, GREEN);
  da_append(&game->colors, BLUE);
  da_append(&game->colors, YELLOW);
  da_append(&game->colors, MAGENTA);
  da_append(&game->colors, LIME);
  da_append(&game->colors, PINK);
  da_append(&game->colors, PURPLE);
  da_append(&game->colors, GOLD);
  da_append(&game->colors, ORANGE);
  da_append(&game->colors, SKYBLUE);
  da_append(&game->colors, DARKBLUE);
  da_append(&game->colors, DARKGREEN);

}

void handle_gm_2p_versus(Scan *scan, Game *game) {
  handle_timer(game->round_timer, DEFAULT_ROUND_TIME_SECS);
  if (game->round_timer) {
    Barcode *b = kill_barcode(scan, game->barcodes);
    if (b) {
      da_append(game->particles, *generate_particles(b->pos, game->colors));
      if (game->brain_rot) {
        Sound s = get_random_sound(game->sounds);
        PlaySound(s);
      } else {
        PlaySound(game->boom);
      }
      if (scan->prefix == game->p1->prefix) {
        game->p1->score += b->value;
      } else if (scan->prefix == game->p2->prefix) {
        game->p2->score += b->value;
      }
      b = generate_qr_barcode(1);
      b->tint = WHITE;
      da_append(game->barcodes, *b);
      b = generate_qr_barcode(1);
      b->tint = WHITE;
      da_append(game->barcodes, *b);
    }
  }
}

void handle_gm_1p_speed_run(Scan *scan, Game *game) {
  UNUSED(scan);
  UNUSED(game);  
}

void handle_gm_1p_infinite(Scan *scan, Game *game) {
  UNUSED(scan);
  UNUSED(game);
}

void init_gm_2p_versus(Game *game) {
  Barcode *b = generate_qr_barcode(1);
  b->tint = WHITE;
  da_append(game->barcodes, *b);

  b = generate_qr_barcode(1);
  b->tint = WHITE; 
  da_append(game->barcodes, *b);
}

void init_gm_1p_speed_run(Game *game) {
  Barcode *b = generate_qr_barcode(1);
  b->tint = WHITE;
  da_append(game->barcodes, *b);
}

void init_gm_1p_infinite(Game *game) {
  Barcode *b = generate_qr_barcode(1);
  b->tint = WHITE;
  da_append(game->barcodes, *b);
}

void update_playing(Game *game) {

  if (IsKeyPressed(KEY_ESCAPE)) {
    game->game_state = GS_MENU;
  } else {
    
    Scans *scans = collect_scans(game->scan_buffer); 
    if (scans->count > 0) {
      for (size_t i = 0; i < scans->count; ++i) {
        Scan *scan = &scans->items[i];
        switch (game->game_mode) {
          case GM_2P_VERSUS: handle_gm_2p_versus(scan, game); break;
          case GM_1P_SPEED_RUN: handle_gm_1p_speed_run(scan, game); break;
          case GM_1P_INFINITE: handle_gm_1p_infinite(scan, game); break;
          default: nob_log(ERROR, "Unhandled game mode (update_playing) %d %s", game->game_mode, get_game_mode_text(game->game_mode));
        } 
      }
    }

    da_foreach(Barcode, b, game->barcodes) {
      if (b->living) update_barcode(b);
    }

    update_particles(game->particles);
  }
}

void draw_gm_2p_versus(Game *game) {
  if (game->round_timer) {
    DrawText(get_timer_text(*game->round_timer, 0), GetScreenWidth()-200, 100, 64, WHITE);
  }
  DrawTextEx(game->player_font, temp_sprintf("Player 1 Score: %d", game->p1->score), (Vector2){.x=50,.y=100}, 28, 1, RAYWHITE);
  DrawTextEx(game->player_font, temp_sprintf("Player 2 Score: %d", game->p2->score), (Vector2){.x=50,.y=150}, 28, 1, RAYWHITE);
}

void draw_gm_1p_speed_run(Game *game) {
  UNUSED(game);
}

void draw_gm_1p_infinite(Game *game) {
  UNUSED(game);
}

void draw_playing(Game *game) {
  BeginDrawing();

  ClearBackground(GetColor(0x181818FF));

  switch (game->game_mode) {
    case GM_2P_VERSUS: draw_gm_2p_versus(game); break;
    case GM_1P_SPEED_RUN: draw_gm_1p_speed_run(game); break;
    case GM_1P_INFINITE: draw_gm_1p_infinite(game); break;
    default: nob_log(ERROR, "Unhandled game mode (draw_playing) %d %s", game->game_mode, get_game_mode_text(game->game_mode));
  } 
  
  da_foreach(Barcode, b, game->barcodes) {
    if (b->living) draw_barcode(*b);
  }

  da_foreach(Particles, p, game->particles) {
    if (p->living) {
      da_foreach(Particle, it, p) {
        if (it->living) {
          DrawCircleV(it->pos, it->radius, it->color);
        }
      }
    }
  }

  EndDrawing();
}

bool draw_menu(Game *game) {
  BeginDrawing();

  ClearBackground(GetColor(0x360036FF));

  Vector2 td = MeasureTextEx(game->title_font, game->title, game->title_font.baseSize, 1);
  Color color = (int)GetTime() % 2 == 0 ? RED : WHITE;
  DrawTextEx(game->title_font, game->title, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=100}, 200, 1, color); 

  bool left_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  size_t y = 400;

  Button start = {0};
  start.config = game->default_button_config;
  start.text = "Start New Game";
  start.config.outline_thiccness = 0;
  start.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(start, game->item_font)/2, .y = y };
  if (draw_button(start, game->item_font)) {
    if (left_clicked) {
      init_game(game);
      game->game_state = GS_PLAYING;
    }
  }


  y = start.pos.y + get_button_height(start, game->item_font) + game->default_button_config.pad_y*4;
  Stepper round_time_stepper = {0};
  round_time_stepper.text = temp_sprintf("Round Time (sec): %03ld", game->round_time_secs);
  round_time_stepper.effective_text = "Round Time (sec): ";
  round_time_stepper.pos = (Vector2) { .x = GetScreenWidth()/2, .y = y };
  round_time_stepper.button_config = game->default_button_config;
  round_time_stepper.button_config.outline_thiccness = 2;
  if (draw_stepper(&round_time_stepper, game->item_font)) {
    if (left_clicked) {
      if (round_time_stepper.incr) game->round_time_secs += 1;
      if (round_time_stepper.decr) game->round_time_secs -= 1;
    }
  }
  
  y = round_time_stepper.pos.y + game->item_font.baseSize + game->default_button_config.pad_y*4;
  Button resume = {0};
  resume.config = game->default_button_config;
  resume.text = "Resume";
  resume.config.outline_thiccness = 0;
  resume.pos = (Vector2) { 
      .x = GetScreenWidth()/2-get_button_width(resume, game->item_font)/2, 
      .y = y
  };
  if (draw_button(resume, game->item_font)) {
    if (left_clicked) {
      game->game_state = GS_PLAYING;
    }
  }

  Button game_mode_btn = {0};
  game_mode_btn.config = game->default_button_config;
  game_mode_btn.text = "Select Game Mode"; 
  game_mode_btn.config.outline_thiccness = 0;
  game_mode_btn.pos = (Vector2) { 
      .x = GetScreenWidth()/2-get_button_width(game_mode_btn, game->item_font)/2, 
      .y = resume.pos.y + get_button_height(start, game->item_font) + game_mode_btn.config.pad_y*4 };
  if (draw_button(game_mode_btn, game->item_font)) {
    if (left_clicked) {
      game->game_state = GS_SELECT_GM;
    }
  }

  Button exit_btn = {0};
  exit_btn.config = game->default_button_config;
  exit_btn.text = "Exit";
  exit_btn.config.outline_thiccness = 0;
  exit_btn.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(exit_btn, game->item_font)/2, .y = game_mode_btn.pos.y + get_button_height(game_mode_btn, game->item_font) + exit_btn.config.pad_y*4 };
  if (draw_button(exit_btn, game->item_font)) {
    if (left_clicked) {
      EndDrawing();
      return true; 
    }
  }

  EndDrawing();
  return false; 
}

void draw_select_game_mode(Game *game) {
  BeginDrawing();

  ClearBackground(GetColor(0x360036FF));

  Vector2 td = MeasureTextEx(game->title_font, "Select Game Mode", game->title_font.baseSize, 1);
  Color color = (int)GetTime() % 2 == 0 ? SKYBLUE : WHITE;

  size_t y = 100;
  DrawTextEx(game->title_font, "Select Game Mode", (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=100},200,1,color);

  bool left_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

  y += 300;
  for (size_t i = 0; i < GM_COUNT; ++i) {
    Button opt = {0};
    opt.config = game->default_button_config;
    opt.text = get_game_mode_text(i);
    opt.config.outline_thiccness = 0;
    opt.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(opt, game->item_font)/2, .y = y};
    if (game->game_mode == i) opt.state = BTN_SELECTED;
    if (draw_button(opt, game->item_font)) {
      if (left_clicked) {
        game->game_mode = i;
        game->barcodes->count = 0;
        game->particles->count = 0;
        switch (game->game_mode) {
          case GM_2P_VERSUS: init_gm_2p_versus(game); break;
          case GM_1P_SPEED_RUN: init_gm_1p_speed_run(game); break;
          case GM_1P_INFINITE: init_gm_1p_infinite(game); break;
          default: nob_log(ERROR, "Unhandled game mode (draw_select_game_mode) %d %s", game->game_mode, get_game_mode_text(game->game_mode));
        }
      }
    }
    y = opt.pos.y + get_button_height(opt, game->item_font) + opt.config.pad_y*4;
  }

  Button return_to_menu_btn = {0};
  return_to_menu_btn.config = game->default_button_config;
  return_to_menu_btn.text = "Return to Main Menu";
  return_to_menu_btn.config.outline_thiccness = 5;
  return_to_menu_btn.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(return_to_menu_btn, game->item_font)/2, .y = y};
  if (draw_button(return_to_menu_btn, game->item_font)) {
    if (left_clicked) {
      game->game_state = GS_MENU;
    }
  }

  EndDrawing();
}

void draw_game_over(Game *game) {
  BeginDrawing();

  ClearBackground(GetColor(0x003636FF));

  const char *p1_score = temp_sprintf("Player 1 Final Score: %d", game->p1->score);
  Vector2 td = MeasureTextEx(game->item_font, p1_score, (float)game->item_font.baseSize, 1); 
  DrawTextEx(game->item_font, p1_score, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=200}, game->item_font.baseSize, 1, WHITE);

  const char *p2_score = temp_sprintf("Player 2 Final Score: %d", game->p2->score);
  td = MeasureTextEx(game->item_font, p2_score, (float)game->item_font.baseSize, 1); 
  DrawTextEx(game->item_font, p2_score, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=200+game->item_font.baseSize*1.5}, game->item_font.baseSize, 1, WHITE);

  if (game->p1->score == game->p2->score) {
    const char *msg = "YOU TIED!!!!!111!!!!1!!!!!!!";
    td = MeasureTextEx(game->item_font, msg, (float)game->item_font.baseSize, 1);
    DrawTextEx(game->item_font, msg, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=200+game->item_font.baseSize*4.5}, game->item_font.baseSize, 1, MAGENTA);
  } else if (game->p1->score > game->p2->score) {
    const char *msg = "PLAYER 1 WINS!!!!!!!";
    td = MeasureTextEx(game->item_font, msg, (float)game->item_font.baseSize, 1);
    DrawTextEx(game->item_font, msg, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=200+game->item_font.baseSize*4.5}, game->item_font.baseSize, 1, MAGENTA);
  } else {
    const char *msg = "PLAYER 2 WINS!!!!!!!";
    td = MeasureTextEx(game->item_font, msg, (float)game->item_font.baseSize, 1);
    DrawTextEx(game->item_font, msg, (Vector2){.x=GetScreenWidth()/2-td.x/2,.y=200+game->item_font.baseSize*4.5}, game->item_font.baseSize, 1, MAGENTA);
  }

  bool left_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

  Button return_to_menu_btn = {0};
  return_to_menu_btn.config = game->default_button_config;
  return_to_menu_btn.text = "Return to Main Menu";
  return_to_menu_btn.config.outline_thiccness = 5;
  return_to_menu_btn.pos = (Vector2) { .x = GetScreenWidth()/2-get_button_width(return_to_menu_btn, game->item_font)/2, .y = game->item_font.baseSize*12 + return_to_menu_btn.config.pad_y*4 };
  if (draw_button(return_to_menu_btn, game->item_font)) {
    if (left_clicked) {
      init_game(game);
      game->game_state = GS_MENU;
    }
  }

  EndDrawing();
}

int main() {

  InitWindow(1920, 1080, "Scanner Wars");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);
  InitAudioDevice();

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  SetRandomSeed(ts.tv_nsec);

  ButtonConfig default_button_config = {0};
  default_button_config.pad_x = 20;
  default_button_config.pad_y = 10;
  default_button_config.color = WHITE;
  default_button_config.hovered_color = SKYBLUE;
  default_button_config.disabled_color = GRAY;
  default_button_config.selected_color = PINK;
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
  game.default_button_config = default_button_config;
  game.game_mode = GM_2P_VERSUS;
  game.round_time_secs = 60;

  while (!WindowShouldClose()) {
    if (game.game_state == GS_PLAYING) {
      update_playing(&game);
      draw_playing(&game); 
    } else if (game.game_state == GS_MENU) {
      if (draw_menu(&game)) break; 
    } else if (game.game_state == GS_GAME_OVER) {
      draw_game_over(&game); 
    } else if (game.game_state == GS_SELECT_GM) {
      draw_select_game_mode(&game);
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
