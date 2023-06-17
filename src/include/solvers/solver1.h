#include "solver.h"

class Solver1 : public Solver {
 public:
  explicit Solver1(Case& case_);
  ~Solver1() override = default;
  void solve() override;
};