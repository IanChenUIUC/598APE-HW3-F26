#include <cstdint>
#include <string>
#include <vector>

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

  // stabilize
  void stabilize();

  // topple each of the cells in row
  static int gather(int N, int row, const count *__restrict prev,
                    const count *__restrict cur, const count *__restrict next,
                    count *__restrict out);

  // row-major storage of data
  std::vector<count> data;
};
