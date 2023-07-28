#ifndef SRC_INCLUDE_DATA_CASE_H_
#define SRC_INCLUDE_DATA_CASE_H_

#include <array>
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
  enum DieSide { TOP, BOTTOM };
  std::array<DieInfo, 2> die_infos;
  Size size;
  CaseTerminal terminal;
  NetList netlist;

  Case() = default;
  Case(const Case&) = default;
  Case(Case&& case_) noexcept
      : die_infos(std::move(case_.die_infos)),
        size(case_.size),
        terminal(case_.terminal),
        netlist(std::move(case_.netlist)) {}
  Case& operator=(const Case&) = default;
  Case& operator=(Case&& case_) noexcept {
    die_infos = std::move(case_.die_infos);
    size = case_.size;
    terminal = case_.terminal;
    netlist = std::move(case_.netlist);
    return *this;
  }

  std::vector<std::string> get_macro_list() const;
  int get_cell_index(const std::string& type) const;
  int get_lib_cell_width(DieSide side, int lib_cell_index) const;
  int get_lib_cell_height(DieSide side, int lib_cell_index) const;
  int get_lib_cell_size(DieSide side, int lib_cell_index) const;
  int get_die_row_height(DieSide side) const;
  int get_die_row_width(DieSide side) const;
  bool get_is_macro(DieSide side, int lib_cell_index) const;

  friend std::istream& operator>>(std::istream& input, Case& case_);
};

#endif  // SRC_INCLUDE_DATA_CASE_H_
