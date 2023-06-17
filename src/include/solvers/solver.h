#ifndef SRC_INCLUDE_SOLVERS_SOLVER_H_
#define SRC_INCLUDE_SOLVERS_SOLVER_H_

#include <ostream>
#include <string>
#include <vector>

#include "data/case.h"

struct Inst {
  std::string name;
  int loc_x;
  int loc_y;
  enum { R0, R90, R180, R270 } orientation;

  friend std::ostream& operator<<(std::ostream& output, Inst& inst);
};

struct SoluTerminal {
  std::string net_name;
  int loc_x;
  int loc_y;

  friend std::ostream& operator<<(std::ostream& output, SoluTerminal& terminal);
};

struct Solution {
  std::vector<Inst> top_die_insts;
  std::vector<Inst> bottom_die_insts;
  std::vector<SoluTerminal> terminals;

  friend std::ostream& operator<<(std::ostream& output, Solution& solution);
};

class Solver {
 public:
  explicit Solver(Case& case_);
  virtual ~Solver() = default;
  virtual void solve() = 0;
  void dump(std::ostream& output);

 protected:
  const Case& case_;
  Solution solution_;
};

#endif  // SRC_INCLUDE_SOLVERS_SOLVER_H_
