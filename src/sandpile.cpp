#include <cmath>
#include <limits>
#include <omp.h>
#include <stdexcept>
#include <utility>

#include "sandpile.h"

template <bool Odd, typename VerifyOutput>
Grid<Odd, VerifyOutput>::Grid(int N)
    : N(N), R(Odd ? (N + 1) / 2 : N), data(R, N), potential(R, N),
      topples(R, N) {
  if (Odd != (N % 2 == 1))
    throw std::invalid_argument("Grid: Odd must match the parity of N");

  lo.reserve(N);
  hi.reserve(N);

  int N2 = N / 2;
  for (int i = 0; i < N; ++i) {
    int rad = static_cast<int>(std::sqrt(N2 * N2 - (i - N2) * (i - N2)));
    lo.push_back(N2 - rad);
    hi.push_back(std::min(std::max(i, N - 1 - i), N2 + rad) + 1);
  }
}

template <bool Odd, typename VerifyOutput>
Grid<Odd, VerifyOutput>::Grid(int N, const Grid<true, std::true_type> &source)
  requires VerifyOutput::value
    : Grid(N) {
  interpolate(source);

  // set grid-values to be a stable state starting from interpolated potential
  // special-casing to ensure neer exceeds 8-bit number range
  for (bool neg : {false, true}) {
    bool changed;
    do {
      changed = false;
      // cells of one parity only neighbour cells of the other
      for (int p : {0, 1}) {
#pragma omp parallel for reduction(| : changed) schedule(guided)
        for (int i = 0; i < R; ++i) {
          for (int j = lo[i] + ((i + lo[i] + p) & 1); j < hi[i]; j += 2) {
            int32_t u = laplacian(i, j);
            if (neg ? u < 0 : u > std::numeric_limits<count>::max()) {
              potential[i, j] -= u >> 2;
              changed = true;
            }
          }
        }
      }
    } while (changed);
  }

  set_heights();
}

template <bool Odd, typename VerifyOutput>
bool Grid<Odd, VerifyOutput>::gather(int lo, int hi,
                                     const count *__restrict prev,
                                     const count *__restrict cur,
                                     const count *__restrict next,
                                     count *__restrict out,
                                     uint8_t *__restrict pot) {
  bool changed = 0;
  for (int j = lo; j < hi; ++j) {
    out[j] = cur[j] - 4 * (cur[j] >= 4) + (prev[j] >= 4) + (cur[j - 1] >= 4) +
             (next[j] >= 4) + (cur[j + 1] >= 4);
    if constexpr (VerifyOutput::value)
      pot[j] += cur[j] >= 4;
    changed |= out[j] != cur[j];
  }
  return changed;
}

template <bool Odd, typename VerifyOutput>
bool Grid<Odd, VerifyOutput>::gather(Matrix<count> &out, int row_lo, int row_hi,
                                     int col_lo, int col_hi) {
  bool changed = false;
  for (int i = row_lo; i < row_hi; ++i)
    changed |= gather(std::max(lo[i], col_lo), std::min(hi[i], col_hi),
                      &(*this)[i - 1], &(*this)[i], &(*this)[i + 1], &out[i, 0],
                      &topples[i, 0]);
  return changed;
}

template <bool Odd, typename VerifyOutput>
void Grid<Odd, VerifyOutput>::stabilize() {
  Matrix<count> buf(R, N);
  int sweeps = 0;
  bool c;
  do {
    c = false;
#pragma omp parallel for reduction(| : c) schedule(guided)
    for (int i = 0; i < R; ++i)
      c |= gather(buf, i, i + 1, 0, N);
    std::swap(buf, this->data);

    // a cell topples at most once per sweep, so flush before the counts wrap
    if constexpr (VerifyOutput::value) {
      if (++sweeps == std::numeric_limits<uint8_t>::max()) {
        flush();
        sweeps = 0;
      }
    }
  } while (c);
  if constexpr (VerifyOutput::value)
    flush();
}

template <bool Odd, typename VerifyOutput>
void Grid<Odd, VerifyOutput>::flush() {
#pragma omp parallel for schedule(guided)
  for (int i = 0; i < R; ++i) {
    for (int j = lo[i]; j < hi[i]; ++j) {
      potential[i, j] -= topples[i, j];
      topples[i, j] = 0;
    }
  }
}

template <bool Odd, typename VerifyOutput>
template <int TileW, int TileH>
void Grid<Odd, VerifyOutput>::stabilize_tiled() {
  const int rows = (R + TileH - 1) / TileH;
  const int cols = (N + TileW - 1) / TileW;

  // whether each tile changed in the previous / current sweep
  // aligned to remove false sharing
  struct alignas(64) Flag {
    uint8_t flag;
  };
  PaddedMatrix<Flag> prev(rows, cols, Flag{1}), cur(rows, cols);

  Matrix<count> buf(R, N);
  int sweeps = 0;
  bool c;
  do {
    c = false;
#pragma omp parallel for reduction(| : c) schedule(guided)
    for (int ti = 0; ti < rows; ++ti) {
      auto active = [&](int tj) {
        return prev[ti, tj].flag || prev[ti - 1, tj].flag ||
               prev[ti + 1, tj].flag || prev[ti, tj - 1].flag ||
               prev[ti, tj + 1].flag;
      };

      // gather each run of consecutive active tiles in a single pass
      int tj = 0;
      while (tj < cols) {
        if (!active(tj)) {
          cur[ti, tj].flag = 0;
          ++tj;
          continue;
        }

        int end = tj + 1;
        while (end < cols && active(end))
          ++end;

        bool changed = gather(buf, ti * TileH, std::min((ti + 1) * TileH, R),
                              tj * TileW, std::min(end * TileW, N));
        for (; tj < end; ++tj)
          cur[ti, tj].flag = changed;
        c |= changed;
      }
    }
    std::swap(buf, this->data);
    std::swap(prev, cur);

    // a cell topples at most once per sweep, so flush before the counts wrap
    if constexpr (VerifyOutput::value) {
      if (++sweeps == std::numeric_limits<uint8_t>::max()) {
        flush();
        sweeps = 0;
      }
    }
  } while (c);
  if constexpr (VerifyOutput::value)
    flush();
}

template <bool Odd, typename VerifyOutput>
void Grid<Odd, VerifyOutput>::interpolate(
    const Grid<true, std::true_type> &source) {
  const double ratio = static_cast<double>(source.N + 1) / (N + 1);
  const double scale = 1 / (ratio * ratio);
  const auto &g = source.potential;

#pragma omp parallel for schedule(guided)
  for (int i = 0; i < R; ++i) {
    double x = (i + 1) * ratio - 1;
    int x0 = std::floor(x);
    double dx = x - x0;
    for (int j = lo[i]; j < hi[i]; ++j) {
      double y = (j + 1) * ratio - 1;
      int y0 = std::floor(y);
      double dy = y - y0;
      double v = (1 - dx) * ((1 - dy) * g[x0, y0] + dy * g[x0, y0 + 1]) +
                 dx * ((1 - dy) * g[x0 + 1, y0] + dy * g[x0 + 1, y0 + 1]);
      // raising by N adds N L 1, i.e. N rounds of burning in one stabilize;
      // this exceeds how far the guess is below the identity (about N / 2)
      potential[i, j] = std::lround(scale * v) + N;
    }
  }
}

template <bool Odd, typename VerifyOutput>
void Grid<Odd, VerifyOutput>::set_heights() {
#pragma omp parallel for schedule(guided)
  for (int i = 0; i < R; ++i)
    for (int j = lo[i]; j < hi[i]; ++j)
      (*this)[i, j] = laplacian(i, j);
}

// Dhar's burning test for recurrence
template <bool Odd, typename VerifyOutput>
bool Grid<Odd, VerifyOutput>::recurrent() const {
  for (int i = 0; i < N; ++i)
    for (int j = lo[i]; j < hi[i]; ++j)
      if ((*this)[i, j] < 0 || (*this)[i, j] > 3)
        return false;

  PaddedMatrix<uint8_t> v(N, N);
  std::vector<std::pair<int, int>> stack;
  for (int i = 0; i < N; ++i) {
    for (int j = lo[i]; j < hi[i]; ++j) {
      v[i, j] = 1;
      stack.emplace_back(i, j);
    }
  }

  int cells = stack.size();
  while (!stack.empty()) {
    auto [i, j] = stack.back();
    stack.pop_back();
    if (!v[i, j])
      continue;
    int nbrs = v[i - 1, j] + v[i + 1, j] + v[i, j - 1] + v[i, j + 1];
    if ((*this)[i, j] < nbrs)
      continue;

    v[i, j] = 0;
    --cells;
    stack.insert(stack.end(), {{i - 1, j}, {i + 1, j}, {i, j - 1}, {i, j + 1}});
  }

  return cells == 0;
}

template <bool Odd, typename VerifyOutput>
int32_t Grid<Odd, VerifyOutput>::laplacian(int row, int col) const {
  const auto &g = potential;
  return 4 * g[row, col] - g[row - 1, col] - g[row + 1, col] - g[row, col - 1] -
         g[row, col + 1];
}

template <bool Odd, typename VerifyOutput>
bool Grid<Odd, VerifyOutput>::is_zero() const {
  for (int i = 0; i < N; ++i)
    for (int j = lo[i]; j < hi[i]; ++j)
      if ((*this)[i, j] != laplacian(i, j))
        return false;
  return true;
}

template <bool Odd, typename VerifyOutput>
bool Grid<Odd, VerifyOutput>::verify_solution() const
  requires VerifyOutput::value
{
  return recurrent() && is_zero();
}

template <bool Odd, typename VerifyOutput>
Grid<Odd, VerifyOutput> Grid<Odd, VerifyOutput>::identity_difference(int N) {
  Grid grid(N);

  // T(6)
  for (int i = 0; i < grid.R; ++i)
    for (int j = grid.lo[i]; j < grid.hi[i]; ++j)
      grid[i, j] = 6;
  grid.stabilize_tiled();

  // T(6 - T(6)), where 6 - T(6) = L x1 has potential x1
  for (int i = 0; i < grid.R; ++i) {
    for (int j = grid.lo[i]; j < grid.hi[i]; ++j) {
      grid[i, j] = 6 - grid[i, j];
      if constexpr (VerifyOutput::value)
        grid.potential[i, j] = -grid.potential[i, j];
    }
  }
  grid.stabilize_tiled();

  return grid;
}

template <bool Odd, typename VerifyOutput>
Grid<Odd, VerifyOutput> Grid<Odd, VerifyOutput>::identity_burning(int N)
  requires VerifyOutput::value
{
  constexpr int base = 61;
  if (N <= base)
    return identity_difference(N);

  Grid grid(N, Grid<true, std::true_type>::identity_burning(((N - 1) / 2) | 1));
  grid.stabilize_tiled();

  // recurrent <-> adding L 1 leaves all grid cells unchanged
  Matrix<count> before(grid.R, grid.N);
  do {
    before = grid.data;
#pragma omp parallel for schedule(guided)
    for (int i = 0; i < grid.R; ++i)
      for (int j = grid.lo[i]; j < grid.hi[i]; ++j)
        ++grid.potential[i, j];
    grid.set_heights();
    grid.stabilize_tiled();
  } while (grid.data != before);

  return grid;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

template <bool Odd, typename VerifyOutput>
bool Grid<Odd, VerifyOutput>::render(const std::string &filename) const {
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
template class Grid<true, std::true_type>;
template class Grid<true, std::false_type>;
template class Grid<false, std::true_type>;
template class Grid<false, std::false_type>;
