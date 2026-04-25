#define NOB_IMPLEMENTATION
#include "nob.h"

#define RAYLIB_VERSION "5.5"

int main(int argc, char** argv) {
  
  GO_REBUILD_URSELF(argc, argv);

  Cmd cmd = {0};
  
  cmd_append(&cmd, "cc");
  cmd_append(&cmd, "-Wall");
  cmd_append(&cmd, "-Wextra");
  cmd_append(&cmd, "-ggdb");
  cmd_append(&cmd, "-I./raylib-"RAYLIB_VERSION"_linux_amd64/include/");
  cmd_append(&cmd, "-o", "main", "main.c");
  cmd_append(&cmd, "-L./raylib-"RAYLIB_VERSION"_linux_amd64/lib/");
  cmd_append(&cmd, "-l:libraylib.a");
  cmd_append(&cmd, "-lm");
  
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
