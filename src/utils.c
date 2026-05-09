#include "utils.h"

int get_random_non_zero_int(int _min, int _max) {
  int v = GetRandomValue(_min, _max);
  if (v == 0) v = 1;
  return v;
}

Font init_font(const char* fp, int font_size, int *codepoints, int codepoint_count) {
  if (FileExists(fp)) 
    return LoadFontEx(fp, font_size, codepoints, codepoint_count);
  return GetFontDefault();
}

Sound get_random_sound(Sounds *sounds) {
  return sounds->items[GetRandomValue(0, sounds->count-1)];
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

String_Builder *alloc_string_builder() {
  String_Builder *sb = (String_Builder*)malloc(sizeof(String_Builder));
  memset(sb, 0, sizeof(*sb));

  return sb; 
}

Sounds *alloc_sounds() {
  Sounds *s = (Sounds*)malloc(sizeof(Sounds));
  memset(s, 0, sizeof(*s));

  return s;
}


