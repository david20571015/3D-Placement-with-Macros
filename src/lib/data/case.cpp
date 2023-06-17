#include "data/case.h"

std::istream& operator>>(std::istream& input, Case& case_) {
  std::string dummy;
  int num_tech;

  input >> dummy >> num_tech;

  std::vector<Technology> techs(num_tech);
  for (auto& tech : techs) {
    input >> tech;
  }

  input >> dummy;
  input >> case_.size_.lower_left_x >> case_.size_.lower_left_y >>
      case_.size_.upper_right_x >> case_.size_.upper_right_y;

  input >> dummy;
  input >> case_.top_die_.max_util;
  input >> dummy;
  input >> case_.bottom_die_.max_util;

  input >> dummy;
  input >> case_.top_die_.rows.start_x >> case_.top_die_.rows.start_y >>
      case_.top_die_.rows.row_length >> case_.top_die_.rows.row_height >>
      case_.top_die_.rows.repeat_count;
  input >> dummy;
  input >> case_.bottom_die_.rows.start_x >> case_.bottom_die_.rows.start_y >>
      case_.bottom_die_.rows.row_length >> case_.bottom_die_.rows.row_height >>
      case_.bottom_die_.rows.repeat_count;

  std::string top_tech_name;
  std::string bottom_tech_name;
  input >> dummy >> top_tech_name;
  input >> dummy >> bottom_tech_name;

  for (auto& tech : techs) {
    if (tech.tech_name == top_tech_name) {
      case_.top_die_.tech = tech;
    }
    if (tech.tech_name == bottom_tech_name) {
      case_.bottom_die_.tech = tech;
    }
  }

  input >> dummy;
  input >> case_.terminal_.size_x >> case_.terminal_.size_y;
  input >> dummy;
  input >> case_.terminal_.spacing;
  input >> dummy;
  input >> case_.terminal_.cost;

  input >> case_.netlist_;

  return input;
}