#ifndef SRC_INCLUDE_DATA_CASE_H_
#define SRC_INCLUDE_DATA_CASE_H_

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "die_info.h"
#include "netlist.h"

struct Size {
  int lower_left_x;
  int lower_left_y;
  int upper_right_x;
  int upper_right_y;
};

struct CaseTerminal {
  int size_x;
  int size_y;
  int spacing;
  int cost;
};

struct Case {
  DieInfo top_die, bottom_die;
  Size size;
  CaseTerminal terminal;
  NetList netlist;

  std::vector<std::string> get_macro_list() const;
  int get_cell_index(const std::string& type) const;

  friend std::istream& operator>>(std::istream& input, Case& case_);
};

#endif  // SRC_INCLUDE_DATA_CASE_H_
