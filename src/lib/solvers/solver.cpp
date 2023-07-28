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
  output << "TopDiePlacement "
         << solution.die_insts[Solution::DieSide::TOP].size() << '\n';
  for (auto& inst : solution.die_insts[Solution::DieSide::TOP]) {
    output << inst;
  }
  output << "BottomDiePlacement "
         << solution.die_insts[Solution::DieSide::BOTTOM].size() << '\n';
  for (auto& inst : solution.die_insts[Solution::DieSide::BOTTOM]) {
    output << inst;
  }
  output << "NumTerminals " << solution.terminals.size() << '\n';
  for (auto& terminal : solution.terminals) {
    output << terminal;
  }
  return output;
}

void Solver::dump(std::ostream& output) { output << solution_; }
