#include <cmath>
#include <omp.h>

#include "sandpile.h"

template <>
Grid<true>::Grid(int N) : N(N), R((N + 1) / 2), data((N + 2) * (R + 1)) {
  lo.reserve(R);
  hi.reserve(R);

  int N2 = N / 2;
  for (int i = 0; i < R; ++i) {
    int rad = static_cast<int>(std::sqrt(N2 * N2 - (i - N2) * (i - N2)));
    lo.push_back(N2 - rad);
    hi.push_back(std::min(std::max(i, N - 1 - i), N2 + rad) + 1);
  }
}

template <> Grid<false>::Grid(int N) : N(N), R(N), data((N + 2) * (R + 2)) {
  lo.reserve(R);
  hi.reserve(R);

  int N2 = N / 2;
  for (int i = 0; i < R; ++i) {
    int rad = static_cast<int>(std::sqrt(N2 * N2 - (i - N2) * (i - N2)));
    lo.push_back(N2 - rad);
    hi.push_back(std::min(std::max(i, N - 1 - i), N2 + rad) + 1);
  }
}

// if Odd, then Grid can store symmetric
template <bool Odd> Grid<Odd>::count &Grid<Odd>::operator[](int row, int col) {
  if (Odd && row >= R)
    row = N - 1 - row;
  return data[(row + 1) * (N + 2) + (col + 1)];
}

template <bool Odd>
const Grid<Odd>::count &Grid<Odd>::operator[](int row, int col) const {
  if (Odd && row >= R)
    row = N - 1 - row;
  return data[(row + 1) * (N + 2) + (col + 1)];
}

template <bool Odd>
int Grid<Odd>::gather(int N, int lo, int hi, const count *__restrict prev,
                      const count *__restrict cur, const count *__restrict next,
                      count *__restrict out) {
  bool changed = 0;
  for (int j = lo; j < hi; ++j) {
    out[j] = cur[j] - 4 * (cur[j] >= 4) + (prev[j] >= 4) + (cur[j - 1] >= 4) +
             (next[j] >= 4) + (cur[j + 1] >= 4);
    changed |= out[j] != cur[j];
  }
  return changed;
}

template <bool Odd> void Grid<Odd>::stabilize() {
  Grid<Odd> buf(N);
  bool c;
  do {
    c = false;
#pragma omp parallel for reduction(| : c) schedule(guided)
    for (int i = 0; i < R; ++i)
      c |= gather(N, lo[i], hi[i], &(*this)[i - 1], &(*this)[i],
                  &(*this)[i + 1], &buf[i]);
    std::swap(buf.data, this->data);
  } while (c);
}

template <bool Odd> Grid<Odd> Grid<Odd>::find_identity(int N) {
  Grid grid(N);

  // T(6)
  for (int i = 0; i < grid.R; ++i)
    for (int j = grid.lo[i]; j < grid.hi[i]; ++j)
      grid[i, j] = 6;
  grid.stabilize();

  // T(6 - T(6))
  for (int i = 0; i < grid.R; ++i)
    for (int j = grid.lo[i]; j < grid.hi[i]; ++j)
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
