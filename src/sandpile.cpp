#include <cmath>

#include "sandpile.h"

template <> Grid<true>::Grid(int N) : N(N), R((N + 1) / 2), data(N * R) {}
template <> Grid<false>::Grid(int N) : N(N), R(N), data(N * R) {}

// if Odd, then Grid can store symmetric
template <bool Odd> Grid<Odd>::count &Grid<Odd>::operator[](int row, int col) {
  if (row >= R)
    row = N - 1 - row;
  return data[row * N + col];
}

template <bool Odd>
const Grid<Odd>::count &Grid<Odd>::operator[](int row, int col) const {
  if (row >= R)
    row = N - 1 - row;
  return data[row * N + col];
}

int get_lo(int N, int row) {
  int N2 = N / 2;
  return N2 - (int)(std::sqrt(N2 * N2 - (row - N2) * (row - N2)));
}

int get_hi(int N, int row) {
  int N2 = N / 2;
  int thresh1 = std::max(row, N - 1 - row);
  int thresh2 = N2 + (int)(std::sqrt(N2 * N2 - (row - N2) * (row - N2)));
  return std::min(thresh1, thresh2) + 1;
}

template <bool Odd> void Grid<Odd>::stabilize() {
  int changed = 0;
  do {
    changed = 0;
    for (int i = 0; i < R; ++i) {
      int lo = get_lo(N, i);
      int hi = get_hi(N, i);
      for (int j = lo; j < hi; ++j) {
        if ((*this)[i, j] < 4)
          continue;
        changed = 1; // topple it
        (*this)[i, j] -= 4;
        if (i > 0)
          (*this)[i - 1, j]++;
        if (j > 0)
          (*this)[i, j - 1]++;
        // folded rows: row R-2 stands for two rows next to the middle row
        if (i < N - 1 && !(Odd && i == R - 1))
          (*this)[i + 1, j] += (Odd && i == R - 2) ? 2 : 1;
        if (j < N - 1)
          (*this)[i, j + 1]++;
      }
    }
  } while (changed);
}

template <bool Odd> Grid<Odd> Grid<Odd>::find_identity(int N) {
  Grid grid(N);

  // T(6)
  for (int i = 0; i < grid.R; ++i)
    for (int j = get_lo(N, i); j < get_hi(N, i); ++j)
      grid[i, j] = 6;
  grid.stabilize();

  // T(6 - T(6))
  for (int i = 0; i < grid.R; ++i)
    for (int j = get_lo(N, i); j < get_hi(N, i); ++j)
      grid[i, j] = 6 - grid[i, j];
  grid.stabilize();

  return grid;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

template <bool Odd> bool Grid<Odd>::render(const std::string &filename) const {
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

// explicit instantiation of Grid
template class Grid<true>;
template class Grid<false>;
