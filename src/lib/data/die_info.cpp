#include "data/die_info.h"

#include <iostream>
#include <utility>

std::istream& operator>>(std::istream& input, LibCell& lib_cell) {
  char is_macro;
  std::string dummy;

  input >> dummy >> is_macro >> lib_cell.name >> lib_cell.size.x >>
      lib_cell.size.y >> lib_cell.pin_count;

  lib_cell.is_macro = (is_macro == 'Y');

  for (int i = 0; i < lib_cell.pin_count; ++i) {
    Pin pin;
    input >> dummy >> pin.name >> pin.loc.x >> pin.loc.y;
    lib_cell.pins.push_back(std::move(pin));
  }

  return input;
}

std::istream& operator>>(std::istream& input, Technology& tech) {
  std::string dummy;

  input >> dummy;
  input >> tech.tech_name >> tech.lib_cell_count;

  for (int i = 0; i < tech.lib_cell_count; ++i) {
    LibCell lib_cell;
    input >> lib_cell;
    tech.lib_cells.push_back(std::move(lib_cell));
  }

  return input;
}