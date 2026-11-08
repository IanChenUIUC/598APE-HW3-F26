#include <cstdlib>
#include <getopt.h>
#include <omp.h>

#include "sandpile.h"

template <bool Odd, typename VerifyOutput> int solve(int N, bool dorender) {
  auto grid = Grid<Odd, VerifyOutput>::identity_difference(N);
  if constexpr (VerifyOutput::value) {
    if (grid.verify_solution()) {
      printf("Verification of identity succeeded\n");
    } else {
      printf("Verification of identity failed\n");
      return EXIT_FAILURE;
    }
  }
  if (dorender) {
    std::string filename = "identity.png";
    if (grid.render(filename)) {
      printf("Successfully saved sandpile identity to: %s\n", filename.c_str());
    } else {
      printf("Error: stb_image_write failed to save png file\n");
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  bool dorender = false;
  bool doverify = false;
  int N = -1;

  // reference for getopt
  // https://www.geeksforgeeks.org/c/getopt-function-in-c-to-parse-command-line-arguments/
  int opt;
  while ((opt = getopt(argc, argv, "rvn:")) != -1) {
    switch (opt) {
    case 'r':
      dorender = true;
      break;
    case 'v':
      doverify = true;
      break;
    case 'n':
      N = std::stoi(optarg);
      break;
    }
  }

  if (N < 0) {
    fprintf(stderr, "Usage: %s -n N [-r] [-v]\n", argv[0]);
    return EXIT_FAILURE;
  }

  printf("running with %d threads\n", omp_get_max_threads());

  if (N & 1)
    return doverify ? solve<true, std::true_type>(N, dorender)
                    : solve<true, std::false_type>(N, dorender);
  return doverify ? solve<false, std::true_type>(N, dorender)
                  : solve<false, std::false_type>(N, dorender);
}
