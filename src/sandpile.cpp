#include "sandpile.h"
#include <Halide.h>
#include <cstdint>
#include <unistd.h>

Grid::Grid(int N) : N(N), data(N * N) {}

Grid::count &Grid::operator[](int row, int col) { return data[row * N + col]; }
const Grid::count &Grid::operator[](int row, int col) const {
  return data[row * N + col];
}

void Grid::stabilize() {
  Halide::Var x("x"), y("y");
  Halide::Func unstable("unstable"), pull("pull"), histogram("histogram");

  Halide::Buffer<count> grid(data.data(), N, N);
  Halide::Func clamped = Halide::BoundaryConditions::constant_exterior(grid, 0);

  // pacman shape consists of two parts:
  //     circle at (N/2, N/2) with radius N/2
  //     everything besides a triangle, j <= max(i, N - 1 - i)
  Halide::RDom r(0, N, 0, N);
  const int N2 = N / 2;
  r.where((r.x - N2) * (r.x - N2) + (r.y - N2) * (r.y - N2) <= N2 * N2);
  r.where(r.x <= Halide::max(r.y, N - 1 - r.y));

  // check if a given cell is stable or not
  unstable(x, y) = clamped(x, y) >= 4;

  // count the numbe of stable cells (pos 0) or unstable (pos 1)
  histogram(unstable(r.x, r.y)) += 1;

  // incr the value of the cell by the number of unstable nbrs
  pull(x, y) = count(0); // undef<count> more expensive, as copy needs to check
  pull(r.x, r.y) = clamped(r.x, r.y) -
                   4 * Halide::cast<count>(unstable(r.x, r.y)) +
                   Halide::cast<count>(unstable(r.x - 1, r.y)) +
                   Halide::cast<count>(unstable(r.x + 1, r.y)) +
                   Halide::cast<count>(unstable(r.x, r.y - 1)) +
                   Halide::cast<count>(unstable(r.x, r.y + 1));

  // tiling didn't do anything
  //    Halide::RVar x_outer, x_inner, y_outer, y_inner;
  //    pull.update(0).tile(r.x, r.y, x_outer, y_outer, x_inner, y_inner, 64,
  //    64);

  // vectorization didn't seem to work
  //    pull.update(0).vectorize(r.x, 4);

  // parallel doesn't help as it increases the cost per round
  // and there are many short rounds...
  //    pull.update(0).parallel(r.x);

  Halide::Buffer<int> counts;
  do {
    Halide::Buffer<count> buf = pull.realize({N, N});
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        (*this)[i, j] = buf(j, i);

    counts = histogram.realize({2});
  } while (counts(1) > 0);
}

Grid Grid::find_identity(int N) {
  std::set_terminate(Halide::Internal::unhandled_exception_handler);

  Grid grid(N);

  // T(6)
  for (int i = 0; i < N; ++i)
    for (int j = 0; j <= std::max(i, N - 1 - i); ++j)
      if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
          (N / 2) * (N / 2))
        grid[i, j] = 6;

  grid.stabilize();

  // T(6 - T(6))
  for (int i = 0; i < N; ++i)
    for (int j = 0; j <= std::max(i, N - 1 - i); ++j)
      if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
          (N / 2) * (N / 2))
        grid[i, j] = 6 - grid[i, j];
  grid.stabilize();

  return grid;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool Grid::render(const std::string &filename) const {
  int chan = 3, stride = N * chan;
  std::vector<char> pixels(N * N * chan);

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch ((*this)[i, j]) {
      case 0: // obsidian
        r = 15;
        g = 15;
        b = 25;
        break;
      case 1: // electric cyan
        r = 0;
        g = 210;
        b = 255;
        break;
      case 2: // vibrant magenta
        r = 230;
        g = 30;
        b = 130;
        break;
      case 3: // bright gold
        r = 255;
        g = 200;
        b = 0;
        break;
      default: // white
        r = 0;
        g = 0;
        b = 0;
      }
      int pid = (i * N + j) * chan;
      pixels[pid] = r;
      pixels[pid + 1] = g;
      pixels[pid + 2] = b;
    }
  }

  return stbi_write_png(filename.data(), N, N, chan, pixels.data(), stride);
}
