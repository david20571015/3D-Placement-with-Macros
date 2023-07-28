#include "data/case.h"

#include <utility>

std::vector<std::string> Case::get_macro_list() const {
  std::vector<std::string> macro_list;

  for (const auto& lib_cell : die_infos[TOP].tech.lib_cells) {
    if (lib_cell.is_macro) {
      macro_list.push_back(lib_cell.name);
    }
  }

  return macro_list;
}

int Case::get_cell_index(const std::string& type) const {
  return die_infos[TOP].tech.get_lib_cell_index(type);
}

int Case::get_lib_cell_width(DieSide side, int lib_cell_index) const {
  return die_infos[side].tech.lib_cells[lib_cell_index].size.x;
};

int Case::get_lib_cell_height(DieSide side, int lib_cell_index) const {
  return die_infos[side].tech.lib_cells[lib_cell_index].size.y;
};

int Case::get_lib_cell_size(DieSide side, int lib_cell_index) const {
  return die_infos[side].tech.lib_cells[lib_cell_index].get_cell_size();
};

int Case::get_die_row_height(DieSide side) const {
  return die_infos[side].rows.row_height;
}
int Case::get_die_row_width(DieSide side) const {
  return die_infos[side].rows.row_length;
}

bool Case::get_is_macro(DieSide side, int lib_cell_index) const {
  return die_infos[side].tech.lib_cells[lib_cell_index].is_macro;
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
  input >> case_.die_infos[Case::DieSide::TOP].max_util;
  input >> dummy;
  input >> case_.die_infos[Case::DieSide::BOTTOM].max_util;

  input >> dummy;
  input >> case_.die_infos[Case::DieSide::TOP].rows.start_x >>
      case_.die_infos[Case::DieSide::TOP].rows.start_y >>
      case_.die_infos[Case::DieSide::TOP].rows.row_length >>
      case_.die_infos[Case::DieSide::TOP].rows.row_height >>
      case_.die_infos[Case::DieSide::TOP].rows.repeat_count;
  input >> dummy;
  input >> case_.die_infos[Case::DieSide::BOTTOM].rows.start_x >>
      case_.die_infos[Case::DieSide::BOTTOM].rows.start_y >>
      case_.die_infos[Case::DieSide::BOTTOM].rows.row_length >>
      case_.die_infos[Case::DieSide::BOTTOM].rows.row_height >>
      case_.die_infos[Case::DieSide::BOTTOM].rows.repeat_count;

  std::string top_tech_name;
  std::string bottom_tech_name;
  input >> dummy >> top_tech_name;
  input >> dummy >> bottom_tech_name;

  for (auto& tech : techs) {
    if (tech.tech_name == top_tech_name) {
      case_.die_infos[Case::DieSide::TOP].tech = tech;
    }
    if (tech.tech_name == bottom_tech_name) {
      case_.die_infos[Case::DieSide::BOTTOM].tech = tech;
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