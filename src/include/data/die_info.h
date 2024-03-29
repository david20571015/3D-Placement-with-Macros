#ifndef SRC_INCLUDE_DATA_DIE_INFO_H_
#define SRC_INCLUDE_DATA_DIE_INFO_H_

#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Pin {
  std::string name;
  struct Location {
    int x;
    int y;
  } loc;
};

struct LibCell {
  bool is_macro;
  std::string name;
  struct Size {
    int x;
    int y;
  } size;
  int pin_count;

  std::vector<Pin> pins;

  int get_cell_size() const { return size.x * size.y; };

  friend std::istream& operator>>(std::istream& input, LibCell& lib_cell);
};

struct Technology {
  std::string tech_name;
  int lib_cell_count;
  std::vector<LibCell> lib_cells;

  int get_lib_cell_index(const std::string& lib_cell_name) const;
  int get_lib_cell_width(int lib_cell_index) const {
    return lib_cells[lib_cell_index].size.x;
  };
  int get_lib_cell_height(int lib_cell_index) const {
    return lib_cells[lib_cell_index].size.y;
  };

  friend std::istream& operator>>(std::istream& input, Technology& tech);
};

struct DieInfo {
  int max_util;

  struct Rows {
    int start_x;
    int start_y;
    int row_length;
    int row_height;
    int repeat_count;
  } rows;

  Technology tech;
};

#endif  // SRC_INCLUDE_DATA_DIE_INFO_H_
