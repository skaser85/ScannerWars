#ifndef UTILS_H
#define UTILS_H

typedef struct {
  Sound *items;
  size_t capacity;
  size_t count;
} Sounds;

int get_random_non_zero_int(int _min, int _max);
Font init_font(const char* fp, int font_size, int *codepoints, int codepoint_count);
Sound get_random_sound(Sounds *sounds);
String_View *alloc_string_view(const char* text);
String_Builder *alloc_string_builder();

#endif // UTILS_H
