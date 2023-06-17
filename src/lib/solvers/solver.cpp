#include "solvers/solver.h"

#include <ostream>

#include "data/case.h"

std::ostream& operator<<(std::ostream& output, Inst& inst) {
  output << "Inst " << inst.name << " " << inst.loc_x << " " << inst.loc_y
         << " ";
  switch (inst.orientation) {
    case Inst::R0:
      output << "R0";
      break;
    case Inst::R90:
      output << "R90";
      break;
    case Inst::R180:
      output << "R180";
      break;
    case Inst::R270:
      output << "R270";
      break;
  }
  output << '\n';
  return output;
}

std::ostream& operator<<(std::ostream& output, SoluTerminal& terminal) {
  output << "Terminal " << terminal.net_name << " " << terminal.loc_x << " "
         << terminal.loc_y << '\n';
  return output;
}

std::ostream& operator<<(std::ostream& output, Solution& solution) {
  output << "TopDiePlacement " << solution.top_die_insts.size() << '\n';
  for (auto& inst : solution.top_die_insts) {
    output << inst;
  }
  output << "BottomDiePlacement " << solution.bottom_die_insts.size() << '\n';
  for (auto& inst : solution.bottom_die_insts) {
    output << inst;
  }
  output << "NumTerminals " << solution.terminals.size() << '\n';
  for (auto& terminal : solution.terminals) {
    output << terminal;
  }
  return output;
}

Solver::Solver(Case& case_) : case_(std::move(case_)) {}

void Solver::dump(std::ostream& output) { output << solution_; }
