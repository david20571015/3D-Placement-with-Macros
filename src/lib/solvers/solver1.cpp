#include "solvers/solver1.h"

#include <algorithm>

#include "data/case.h"

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

void Solver1::sort(DIE_INDEX idx, std::vector<std::string>& macro_C_index) {
  std::sort(macro_C_index.begin(), macro_C_index.end(),
            [&](const std::string& a, const std::string& b) {
              const std::string a_type = case_.netlist.inst[a];
              const std::string b_type = case_.netlist.inst[b];
              const int a_die_cell_index = case_.get_cell_index(a_type);
              const int b_die_cell_index = case_.get_cell_index(b_type);
              int a_cell_size_ratio =
                  case_.get_lib_cell_height(idx, a_die_cell_index) /
                  case_.get_lib_cell_width(idx, a_die_cell_index);
              int b_cell_size_ratio =
                  case_.get_lib_cell_height(idx, b_die_cell_index) /
                  case_.get_lib_cell_width(idx, b_die_cell_index);

              if (a_cell_size_ratio < 1) {
                a_cell_size_ratio = 1 / a_cell_size_ratio;
              }
              if (b_cell_size_ratio < 1) {
                b_cell_size_ratio = 1 / b_cell_size_ratio;
              }

              return a_cell_size_ratio >= b_cell_size_ratio;
            });
}

void Solver1::decide_what_die(std::vector<std::string>& top_die,
                              std::vector<std::string>& bottom_die,
                              std::vector<std::string>& macro_C_index) {
  for (auto& inst_name : macro_C_index) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);

    const bool alter = (die_max_util[TOP] - die_util[TOP]) >
                       (die_max_util[BOTTOM] - die_util[BOTTOM]);

    if (alter && check_capacity(TOP, die_cell_index)) {
      die_util[TOP] +=
          case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
          die_size;
      top_die.push_back(inst_name);
    } else if (!alter && check_capacity(BOTTOM, die_cell_index)) {
      die_util[BOTTOM] +=
          case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
          die_size;
      bottom_die.push_back(inst_name);
    } else {
      std::cerr << "Die utilization exceeds maximum utilization" << std::endl;
      return;
    }
  }
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

  // decide what die each macro should be placed
  std::vector<std::string> top_die_macros;
  std::vector<std::string> bottom_die_macros;
  decide_what_die(top_die_macros, bottom_die_macros, macro_C_index);

  // sort by (height / width)
  sort(TOP, top_die_macros);
  sort(BOTTOM, bottom_die_macros);

  // place macros on the top die
  for (auto& inst_name : top_die_macros) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);

    int width = case_.get_lib_cell_width(TOP, die_cell_index);
    int height = case_.get_lib_cell_height(TOP, die_cell_index);
    int rotate = 0;  // 0: no rotate, 1: rotate 90 degree, 2: rotate 180 degree,
                     // 3: rotate 270 degree

    if (width > height) {
      std::swap(width, height);
      rotate = 1;
    }

    // place
  }

  // decide what die each cell should be placed
  std::vector<std::string> top_die_cells;
  std::vector<std::string> bottom_die_cells;
  decide_what_die(top_die_cells, bottom_die_cells, cell_C_index);
}