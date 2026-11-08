#include <cstdlib>
#include <getopt.h>
#include <omp.h>

#include "sandpile.h"

template <bool Odd, typename VerifyOutput>
int solve(int N, bool burning, bool doverify, bool dorender) {
  auto identity = [&] {
    if constexpr (VerifyOutput::value)
      if (burning)
        return Grid<Odd, VerifyOutput>::identity_burning(N);
    return Grid<Odd, VerifyOutput>::identity_difference(N);
  };
  auto grid = identity();

  if constexpr (VerifyOutput::value) {
    if (doverify) {
      bool ok = grid.verify_solution();
      printf("Verification of identity %s\n", ok ? "succeeded" : "failed");
      if (!ok)
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
  std::string algorithm = "burning";
  int N = -1;

  // reference for getopt
  // https://www.geeksforgeeks.org/c/getopt-function-in-c-to-parse-command-line-arguments/
  static const option longopts[] = {
      {"algorithm", required_argument, nullptr, 'a'}, {}};
  int opt;
  while ((opt = getopt_long(argc, argv, "rvn:", longopts, nullptr)) != -1) {
    switch (opt) {
    case 'a':
      algorithm = optarg;
      break;
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

  if (N < 0 || (algorithm != "burning" && algorithm != "difference")) {
    fprintf(stderr,
            "Usage: %s -n N [--algorithm burning|difference] [-r] [-v]\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  printf("running with %d threads\n", omp_get_max_threads());

  // burning needs the potential
  bool burning = algorithm == "burning";
  bool potential = burning || doverify;
  if (N & 1)
    return potential
               ? solve<true, std::true_type>(N, burning, doverify, dorender)
               : solve<true, std::false_type>(N, burning, doverify, dorender);
  return potential
             ? solve<false, std::true_type>(N, burning, doverify, dorender)
             : solve<false, std::false_type>(N, burning, doverify, dorender);
}
