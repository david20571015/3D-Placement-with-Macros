#include "solver.h"

class Solver1 : public Solver {
  std::vector<int> die_max_util;
  std::vector<int> die_util;
  int die_size;
  
 public:
  explicit Solver1(Case& case_);
  ~Solver1() override = default;
  void solve() override;
  bool check_capacity(int index, int die_cell_index);
  void sort(int, std::vector<std::string>&);
  void decide_what_die(std::vector<std::string>&, std::vector<std::string>&, std::vector<std::string>&);
};