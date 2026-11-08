#include <cstdint>
#include <string>
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

template <bool Odd> class Grid {
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

  // find the identity element of the NxN recurrent Abelian Sandpile Model
  static Grid find_identity(int N);

private:
  const int N;
  const int R; // half if it is odd

  // intervals of the cols that are non-empty
  std::vector<int> lo;
  std::vector<int> hi;

  // stabilize, sweeping every row
  void stabilize();

  // stabilize, early exit on TileW x TileH
  template <int TileW = 64, int TileH = 4> void stabilize_tiled();

  // topple [row_lo, row_hi) x [col_lo, col_hi) into out
  bool gather(Grid &out, int row_lo, int row_hi, int col_lo, int col_hi) const;

  // topple cells [lo, hi) into out
  static bool gather(int lo, int hi, const count *__restrict prev,
                     const count *__restrict cur, const count *__restrict next,
                     count *__restrict out);

  PaddedMatrix<count> data;
};
