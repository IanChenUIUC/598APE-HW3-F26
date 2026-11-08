#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// 2D array with {0}-padding of width 1
template <typename T> class PaddedMatrix {
public:
  PaddedMatrix(int rows, int cols, T init = T{})
      : rows(rows), cols(cols), data((rows + 2) * (cols + 2)) {
    for (int i = 0; i < rows; ++i)
      for (int j = 0; j < cols; ++j)
        (*this)[i, j] = init;
  }

  const T &operator[](int row, int col) const {
    return data[(row + 1) * (cols + 2) + (col + 1)];
  }
  T &operator[](int row, int col) {
    return data[(row + 1) * (cols + 2) + (col + 1)];
  }

private:
  int rows;
  int cols;

  // row-major storage of data
  std::vector<T> data;
};

template <bool Odd, typename VerifyOutput = std::true_type> class Grid {
public:
  using count = int8_t;
  Grid(int N);

  // 2-D array indexing
  const count &operator[](int row, int col) const;
  count &operator[](int row, int col);

  // 1-D array indexing
  const count &operator[](int row) const { return (*this)[row, 0]; }
  count &operator[](int row) { return (*this)[row, 0]; }

  // render to an image; return whether writing succeeded
  bool render(const std::string &filename) const;

  // find the identity element of the Pacman recurrent Abelian Sandpile Model;
  // if VerifyOutput, also keep its potential as a certificate
  static Grid find_identity(int N);

  // whether the grid is the identity: recurrent, and zero by its potential
  bool verify_solution() const
    requires VerifyOutput::value;

private:
  const int N;
  const int R; // half if it is odd

  // intervals of the cols that are non-empty
  std::vector<int> lo;
  std::vector<int> hi;

  // stored (row, col) of (row, col) in the full NxN grid
  std::pair<int, int> index(int row, int col) const;

  // whether the configuration is recurrent, by Dhar's burning test
  bool recurrent() const;

  // whether the configuration is zero, i.e. data = L potential
  bool is_zero() const;

  // stabilize, sweeping every row
  void stabilize();

  // stabilize, early exit on TileW x TileH
  template <int TileW = 64, int TileH = 4> void stabilize_tiled();

  // topple [row_lo, row_hi) x [col_lo, col_hi) into out;
  // accumulate updates into potential (for use as a certificate)
  bool gather(PaddedMatrix<count> &out, int row_lo, int row_hi, int col_lo,
              int col_hi);

  // topple cells [lo, hi) into out, counting the topples in cnt
  static bool gather(int lo, int hi, const count *__restrict prev,
                     const count *__restrict cur, const count *__restrict next,
                     count *__restrict out, uint8_t *__restrict cnt);

  PaddedMatrix<count> data;

  // potential g with data = L g, where L is the Laplacian
  // lazily updated from potential - topples using flush
  PaddedMatrix<int32_t> potential;
  PaddedMatrix<uint8_t> topples;
  void flush();
};
