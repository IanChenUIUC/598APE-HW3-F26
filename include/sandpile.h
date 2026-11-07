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

  // render to an image; return whether writing succeeded
  bool render(const std::string &filename) const;

  // find the identity element of the NxN recurrent Abelian Sandpile Model
  static Grid find_identity(int N);

private:
  const int N;
  const int R; // half if it is odd

  // stabilize
  void stabilize();

  // row-major storage of data
  std::vector<count> data;
};
