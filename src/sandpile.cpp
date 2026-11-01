#include "sandpile.h"

Grid::Grid(int N) : N(N), data(N * N) {}

Grid::count &Grid::operator[](int row, int col) { return data[row * N + col]; }
const Grid::count &Grid::operator[](int row, int col) const {
  return data[row * N + col];
}

void Grid::stabilize() {
  int changed = 0;
  do {
    changed = 0;
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j <= std::max(i, N - 1 - i); ++j) {
        if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
            (N / 2) * (N / 2)) {
          if ((*this)[i, j] < 4)
            continue;
          changed = 1; // topple it
          (*this)[i, j] -= 4;
          if (i > 0)
            (*this)[i - 1, j]++;
          if (j > 0)
            (*this)[i, j - 1]++;
          if (i < N - 1)
            (*this)[i + 1, j]++;
          if (j < N - 1)
            (*this)[i, j + 1]++;
        }
      }
    }
  } while (changed);
}

Grid Grid::find_identity(int N) {
  Grid grid(N);

  // T(6)
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      grid[i, j] = 6;
  grid.stabilize();

  // T(6 - T(6))
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
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
