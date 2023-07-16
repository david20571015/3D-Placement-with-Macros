#include "data/case.h"

#include <utility>

std::vector<std::string> Case::get_macro_list() const {
  std::vector<std::string> macro_list;

  for (const auto& lib_cell : top_die.tech.lib_cells) {
    if (lib_cell.is_macro) {
      macro_list.push_back(lib_cell.name);
    }
  }

  return macro_list;
}

int Case::get_cell_index(const std::string& type) const {
  return top_die.tech.get_lib_cell_index(type);
}

int Case::get_lib_cell_width(int die_index, int lib_cell_index) const {
  return (die_index == 0) ? top_die.tech.lib_cells[lib_cell_index].size.x
                          : bottom_die.tech.lib_cells[lib_cell_index].size.x;
};

int Case::get_lib_cell_height(int die_index, int lib_cell_index) const {
  return (die_index == 0) ? top_die.tech.lib_cells[lib_cell_index].size.y
                          : bottom_die.tech.lib_cells[lib_cell_index].size.y;
};

int Case::get_die_row_height(int die_index) const {
  return (die_index == 0) ? top_die.rows.row_height : bottom_die.rows.row_height;
}

std::istream& operator>>(std::istream& input, Case& case_) {
  std::string dummy;
  int num_tech;

  input >> dummy >> num_tech;

  std::vector<Technology> techs(num_tech);
  for (auto& tech : techs) {
    input >> tech;
  }

  input >> dummy;
  input >> case_.size.lower_left_x >> case_.size.lower_left_y >>
      case_.size.upper_right_x >> case_.size.upper_right_y;

  input >> dummy;
  input >> case_.top_die.max_util;
  input >> dummy;
  input >> case_.bottom_die.max_util;

  input >> dummy;
  input >> case_.top_die.rows.start_x >> case_.top_die.rows.start_y >>
      case_.top_die.rows.row_length >> case_.top_die.rows.row_height >>
      case_.top_die.rows.repeat_count;
  input >> dummy;
  input >> case_.bottom_die.rows.start_x >> case_.bottom_die.rows.start_y >>
      case_.bottom_die.rows.row_length >> case_.bottom_die.rows.row_height >>
      case_.bottom_die.rows.repeat_count;

  std::string top_tech_name;
  std::string bottom_tech_name;
  input >> dummy >> top_tech_name;
  input >> dummy >> bottom_tech_name;

  for (auto& tech : techs) {
    if (tech.tech_name == top_tech_name) {
      case_.top_die.tech = tech;
    }
    if (tech.tech_name == bottom_tech_name) {
      case_.bottom_die.tech = tech;
    }
  }

  input >> dummy;
  input >> case_.terminal.size_x >> case_.terminal.size_y;
  input >> dummy;
  input >> case_.terminal.spacing;
  input >> dummy;
  input >> case_.terminal.cost;

  input >> case_.netlist;

  return input;
}