#include "solver.h"

class Solver1 : public Solver {
  std::vector<int> die_max_util;
  std::vector<int> die_util;
  int die_size;

  enum DIE_INDEX { TOP, BOTTOM };

 public:
  explicit Solver1(Case& case_);
  ~Solver1() override = default;
  void solve() override;
  bool check_capacity(int index, int die_cell_index);
  void sort(DIE_INDEX idx, std::vector<std::string>& macro_C_index);
  void decide_what_die(std::vector<std::string>& top_die,
                       std::vector<std::string>& bottom_die,
                       std::vector<std::string>& macro_C_index);
};