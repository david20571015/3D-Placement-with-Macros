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

class Case {
 public:
  friend std::istream& operator>>(std::istream& input, Case& case_);

 private:
  DieInfo top_die_, bottom_die_;
  Size size_;
  CaseTerminal terminal_;
  NetList netlist_;
};

#endif  // SRC_INCLUDE_DATA_CASE_H_
