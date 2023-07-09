#include "solvers/solver1.h"

#include <algorithm>

#include "data/case.h"

enum die_index { TOP, BOTTOM };

Solver1::Solver1(Case& case_) : Solver(case_) {}

bool Solver1::check_capacity(int index, int die_cell_index) {
  if (index == TOP) {
    return die_util[TOP] +
               case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() <=
           die_max_util[TOP];
  }
  return (die_util[BOTTOM] +
              case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() <=
          die_max_util[BOTTOM]);
}

void Solver1::solve() {
  die_size = case_.size.upper_right_x * case_.size.upper_right_y;
  die_max_util = {case_.top_die.max_util, case_.bottom_die.max_util};
  die_util = {0, 0};
  std::vector<std::string> macro_list = case_.get_macro_list();
  std::vector<std::string> macro_C_index;
  std::vector<std::string> cell_C_index;

  // separate macros and cells
  for (auto& inst : case_.netlist.inst) {
    if (std::find(macro_list.begin(), macro_list.end(), inst.second) !=
        macro_list.end()) {
      macro_C_index.push_back(inst.first);
    } else {
      cell_C_index.push_back(inst.first);
    }
  }

  // place macros
  for (auto& inst_name : macro_C_index) {
    std::string const inst_type = case_.netlist.inst[inst_name];
    int const die_cell_index = case_.get_cell_index(inst_type);

    bool const alter = (die_max_util[TOP] - die_util[TOP]) >
                       (die_max_util[BOTTOM] - die_util[BOTTOM]);

    if (alter && check_capacity(TOP, die_cell_index)) {
      die_util[TOP] +=
          case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
          die_size;
    } else if (!alter && check_capacity(BOTTOM, die_cell_index)) {
      die_util[BOTTOM] +=
          case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
          die_size;
    } else {
      std::cerr << "Die utilization exceeds maximum utilization" << std::endl;
      exit(1);
    }
  }

  // place cells
  for (auto& inst_name : cell_C_index) {
    std::string const inst_type = case_.netlist.inst[inst_name];
    int const die_cell_index = case_.get_cell_index(inst_type);

    bool const alter = (die_max_util[TOP] - die_util[TOP]) >
                       (die_max_util[BOTTOM] - die_util[BOTTOM]);

    if (alter && check_capacity(TOP, die_cell_index)) {
      die_util[TOP] +=
          case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
          die_size;
    } else if (!alter && check_capacity(BOTTOM, die_cell_index)) {
      die_util[BOTTOM] +=
          case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
          die_size;
    } else {
      std::cerr << "Die utilization exceeds maximum utilization" << std::endl;
      exit(1);
    }
  }
}