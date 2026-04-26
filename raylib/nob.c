#define NOB_IMPLEMENTATION
#include "./thirdparty/nob/nob.h"

#define FLAG_IMPLEMENTATION
#include "./thirdparty/nob/flag.h"

#define SRC_DIR "./src"
#define BUILD_DIR "./build"

#define RAYLIB_VERSION "5.5"

static void usage(void)
{
    fprintf(stderr, "Usage: %s [<FLAGS>] [--] [<program args>]\n", flag_program_name());
    fprintf(stderr, "FLAGS:\n");
    flag_print_options(stderr);
}

int main(int argc, char** argv) {
  
  GO_REBUILD_URSELF(argc, argv);

  bool run = false;
  bool help = false;
  flag_bool_var(&run, "run", false, "Run the program after compilation");
  flag_bool_var(&help, "help", false, "Print this help message");

  if (!flag_parse(argc, argv)) {
    usage();
    flag_print_error(stderr);
    return 1;
  }

  if (help) {
    usage();
    return 0;
  }
  
  Cmd cmd = {0};
  
  cmd_append(&cmd, "cc");
  cmd_append(&cmd, "-Wall");
  cmd_append(&cmd, "-Wextra");
  cmd_append(&cmd, "-ggdb");

  // include qrcode header file
  cmd_append(&cmd, "-I./thirdparty/qrcode/");

  // include raylib header files 
  cmd_append(&cmd, "-I./thirdparty/raylib-"RAYLIB_VERSION"_linux_amd64/include/");

  // include nob header file
  cmd_append(&cmd, "-I./thirdparty/nob/");

  // define what to create and from what source code
  cmd_append(&cmd, "-o", BUILD_DIR"/main", SRC_DIR"/main.c");
  
  // link qrcode lib 
  cmd_append(&cmd, "-L./thirdparty/qrcode/");
  cmd_append(&cmd, "-lqrcode");

  // link raylib libs
  cmd_append(&cmd, "-L./thirdparty/raylib-"RAYLIB_VERSION"_linux_amd64/lib/");
  cmd_append(&cmd, "-l:libraylib.a");
  cmd_append(&cmd, "-lm");
 
  if (!cmd_run(&cmd)) return 1;
  
  if (run) {
    cmd_append(&cmd, BUILD_DIR"/main");
    if (!cmd_run(&cmd)) return 1;
  }

  return 0;
}
