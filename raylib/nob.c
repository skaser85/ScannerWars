#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char** argv) {
  
  GO_REBUILD_URSELF(argc, argv);

  Cmd cmd = {0};

  cmd_append(&cmd, "cc");
  cmd_append(&cmd, "-Wall");
  cmd_append(&cmd, "-Wextra");
  cmd_append(&cmd, "-o");
  cmd_append(&cmd, "main");
  cmd_append(&cmd, "main.c");
  
  //cmd_append(&cmd, "-lraylib", "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");

  cmd_append(&cmd, "-I./raylib-6.0_linux_amd64/include/");
  cmd_append(&cmd, "-L./raylib-6.0_linux_amd64/lib/");
  cmd_append(&cmd, "-l:libraylib.a");
  cmd_append(&cmd, "-lpthread");
  cmd_append(&cmd, "-ldl");
  cmd_append(&cmd, "-lrt");
  cmd_append(&cmd, "-lGL");
  cmd_append(&cmd, "-lX11");
  cmd_append(&cmd, "-lm");

  if (!cmd_run(&cmd)) return 1;

  return 0;
}
