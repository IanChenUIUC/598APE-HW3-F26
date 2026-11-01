#include <cstdint>
#include <string>
#include <vector>

class Grid {
public:
  int N;

  using count = int;
  Grid(int N);

  // 2-D array indexing
  const count &operator[](int row, int col) const;
  count &operator[](int row, int col);

  // render to an image; return whether writing succeeded
  bool render(const std::string &filename) const;

  // find the identity element of the NxN recurrent Abelian Sandpile Model
  static Grid find_identity(int N);

private:
  // stabilize
  void stabilize();

  // row-major storage of data
  std::vector<count> data;
};
