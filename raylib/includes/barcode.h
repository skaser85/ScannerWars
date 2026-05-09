#ifndef BARCODE_H
#define BARCODE_H

#define BOX_OUTLINE_THICCNESS 1
#define SPEED_MAX 5 
#define BARCODE_SIZE 75

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
  Scan *items;
  size_t capacity;
  size_t count;
} Scans;

Rectangle get_barcode_rect(Barcode b);
void draw_barcode(Barcode b);
void update_barcode(Barcode *me);

Barcode *alloc_barcode();
Barcodes *alloc_barcodes();
Scan *alloc_scan(const char* text);
Scans *alloc_scans();

Scan *process_scan(String_Builder sb);
size_t get_id();
Barcode *generate_qr_barcode(int value);
Barcode *kill_barcode(Scan *scan, Barcodes *bars);
Scans *collect_scans(String_Builder *sb);

#endif // BARCODE_H
