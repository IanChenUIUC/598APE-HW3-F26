#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

// 2D array with {0}-padding of width 1;
// if Mirror, rows past the last row are reflected about it
template <typename T, bool Mirror = false> class PaddedMatrix {
public:
  PaddedMatrix(int rows, int cols, T init = T{})
      : rows(rows), cols(cols), data((rows + 2) * (cols + 2)) {
    for (int i = 0; i < rows; ++i)
      for (int j = 0; j < cols; ++j)
        (*this)[i, j] = init;
  }

  const T &operator[](int row, int col) const { return data[offset(row, col)]; }
  T &operator[](int row, int col) { return data[offset(row, col)]; }

  bool operator==(const PaddedMatrix &) const = default;

private:
  int rows;
  int cols;

  // row-major storage of data
  std::vector<T> data;

  int offset(int row, int col) const {
    if constexpr (Mirror)
      row = row < rows ? row : 2 * (rows - 1) - row;
    return (row + 1) * (cols + 2) + (col + 1);
  }
};

template <bool Odd, typename VerifyOutput = std::true_type> class Grid {
public:
  using count = int8_t;
  Grid(int N);

  // 2-D array indexing
  const count &operator[](int row, int col) const { return data[row, col]; }
  count &operator[](int row, int col) { return data[row, col]; }

  // 1-D array indexing
  const count &operator[](int row) const { return (*this)[row, 0]; }
  count &operator[](int row) { return (*this)[row, 0]; }

  // render to an image; return whether writing succeeded
  bool render(const std::string &filename) const;

  // find the identity element of the Pacman recurrent Abelian Sandpile Model;
  // if VerifyOutput, also keep its potential as a certificate
  static Grid identity_difference(int N);

  // find the identity element of the Pacman recurrent Abelian Sandpile Model;
  static Grid identity_burning(int N)
    requires VerifyOutput::value;

  // whether the grid is the identity: recurrent, and zero by its potential
  bool verify_solution() const
    requires VerifyOutput::value;

private:
  template <bool, typename> friend class Grid;

  // if Odd, only the top half is stored, as the grid is symmetric
  template <typename T> using Matrix = PaddedMatrix<T, Odd>;

  // guess the potential from another grid, with data = L potential in
  // [0, 127]
  Grid(int N, const Grid<true, std::true_type> &source)
    requires VerifyOutput::value;

  const int N;
  const int R; // half if it is odd

  // intervals of the cols that are non-empty
  std::vector<int> lo;
  std::vector<int> hi;

  // whether the configuration is recurrent, by Dhar's burning test
  bool recurrent() const;

  // whether the configuration is zero, i.e. data = L potential
  bool is_zero() const;

  // (L potential) at (row, col) in the full NxN grid
  int32_t laplacian(int row, int col) const;

  // stabilize, sweeping every row
  void stabilize();

  // stabilize, early exit on TileW x TileH
  template <int TileW = 64, int TileH = 4> void stabilize_tiled();

  // topple [row_lo, row_hi) x [col_lo, col_hi) into out;
  // accumulate updates into potential (for use as a certificate)
  bool gather(Matrix<count> &out, int row_lo, int row_hi, int col_lo,
              int col_hi);

  // topple cells [lo, hi) into out, counting the topples in cnt
  static bool gather(int lo, int hi, const count *__restrict prev,
                     const count *__restrict cur, const count *__restrict next,
                     count *__restrict out, uint8_t *__restrict cnt);

  Matrix<count> data;

  // potential g with data = L g, where L is the Laplacian
  // lazily updated from potential - topples using flush
  Matrix<int32_t> potential;
  Matrix<uint8_t> topples;
  void flush();

  // bilinear interpolation, scaled by the squared ratio of sizes, raised by N
  void interpolate(const Grid<true, std::true_type> &source);

  // set data = L potential
  void set_heights();
};
