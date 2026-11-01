#include <getopt.h>

#include "sandpile.h"

int main(int argc, char **argv) {
  bool dorender = false;
  int N = -1;

  // reference for getopt
  // https://www.geeksforgeeks.org/c/getopt-function-in-c-to-parse-command-line-arguments/
  int opt;
  while ((opt = getopt(argc, argv, "rn:")) != -1) {
    switch (opt) {
    case 'r':
      dorender = true;
      break;
    case 'n':
      N = std::stoi(optarg);
      break;
    }
  }

  if (N < 0) {
    fprintf(stderr, "Usage: %s -n N [-r]\n", argv[0]);
    return EXIT_FAILURE;
  }

  Grid grid = Grid::find_identity(N);

  if (dorender) {
    std::string filename = "identity.png";
    if (grid.render(filename)) {
      printf("Successfully saved sandpile identity to: %s\n", filename.c_str());
    } else {
      printf("Error: stb_image_write failed to save png file\n");
    }
  }

  return EXIT_SUCCESS;
}
