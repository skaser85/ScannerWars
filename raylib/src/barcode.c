#include "utils.h"
#include "barcode.h"

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

Barcode *alloc_barcode() {
  Barcode *b = (Barcode*)malloc(sizeof(Barcode));
  memset(b, 0, sizeof(*b));

  return b;
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

Scans *alloc_scans() {
  Scans *s = (Scans*)malloc(sizeof(Scans));
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

Scans *collect_scans(String_Builder *sb) {
  Scans *scans = alloc_scans();
   
  int k = GetKeyPressed();
  while (k > 0) {
    if (k == KEY_ENTER) {
      //nob_log(INFO, "raw scan: \n"SV_Fmt, SV_Arg(sb_to_sv(buff)));
      Scan *scan = process_scan(*sb);
      sb->count = 0;
      da_append(scans, *scan);
    } else {
      sb_append(sb, (char)k);
    }
    k = GetKeyPressed();
  }

  return scans;
}


