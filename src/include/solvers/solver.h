#ifndef SRC_INCLUDE_SOLVERS_SOLVER_H_
#define SRC_INCLUDE_SOLVERS_SOLVER_H_

#include <array>
#include <ostream>
#include <string>
#include <vector>

#include "data/case.h"

struct Inst {
  std::string name;
  int loc_x;
  int loc_y;
  enum Rotate { R0, R90, R180, R270 };
  Rotate orientation = R0;

  Inst() = default;
  Inst(std::string name_, int loc_x_, int loc_y_, Rotate orientation_)
      : name(std::move(name_)),
        loc_x(loc_x_),
        loc_y(loc_y_),
        orientation(orientation_) {}

  friend std::ostream& operator<<(std::ostream& output, Inst& inst);
};

struct SoluTerminal {
  std::string net_name;
  int loc_x;
  int loc_y;

  SoluTerminal() = default;
  SoluTerminal(std::string net_name_, int loc_x_, int loc_y_)
      : net_name(std::move(net_name_)), loc_x(loc_x_), loc_y(loc_y_) {}

  friend std::ostream& operator<<(std::ostream& output, SoluTerminal& terminal);
};

struct Solution {
  using DieSide = Case::DieSide;
  std::array<std::vector<Inst>, 2> die_insts;
  std::vector<SoluTerminal> terminals;

  friend std::ostream& operator<<(std::ostream& output, Solution& solution);
};

class Solver {
 public:
  explicit Solver(Case& case_) : case_(std::move(case_)){};
  explicit Solver(Case&& case_) : case_(std::move(case_)){};
  virtual ~Solver() = default;
  virtual void solve() = 0;
  void dump(std::ostream& output);

 protected:
  Case case_;
  Solution solution_;
};

#endif  // SRC_INCLUDE_SOLVERS_SOLVER_H_
