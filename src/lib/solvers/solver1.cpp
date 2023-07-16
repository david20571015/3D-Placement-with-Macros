#include "solvers/solver1.h"

#include <algorithm>

#include "data/case.h"

Solver1::Solver1(Case& case_) : Solver(case_) {
  line_segment init;
  init.y = 0;
  init.from = 0;
  init.to = case_.size.upper_right_x;
  horizontal_contours[TOP].push_back(init);
  horizontal_contours[BOTTOM].push_back(init);
  die_size = case_.size.upper_right_x * case_.size.upper_right_y;
  die_max_util = {case_.top_die.max_util, case_.bottom_die.max_util};
  die_util = {0, 0};
}

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

void Solver1::separate_macros_cells(std::vector<std::string> macro_list,
                                    std::vector<std::string>& macro_C_index,
                                    std::vector<std::string>& cell_C_index) {
  for (auto& inst : case_.netlist.inst) {
    case_.netlist.placed[inst.first] = false;
    if (std::find(macro_list.begin(), macro_list.end(), inst.second) !=
        macro_list.end()) {
      macro_C_index.push_back(inst.first);
    } else {
      cell_C_index.push_back(inst.first);
    }
  }
}

void Solver1::sort_macro(DIE_INDEX idx, std::vector<std::string>& macro_C_index) {
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

void Solver1::sort_cell(DIE_INDEX idx, std::vector<std::string>& cell_C_index) {
  std::sort(cell_C_index.begin(), cell_C_index.end(),
    [&](const std::string& a, const std::string& b) {
      const std::string a_type = case_.netlist.inst[a];
      const std::string b_type = case_.netlist.inst[b];
      const int a_die_cell_index = case_.get_cell_index(a_type);
      const int b_die_cell_index = case_.get_cell_index(b_type);
      int a_cell_size = case_.get_lib_cell_width(idx, a_die_cell_index);
      int b_cell_size = case_.get_lib_cell_width(idx, b_die_cell_index);

      return a_cell_size >= b_cell_size;
    });
}

void Solver1::decide_what_die(std::vector<std::string> inst_C_index,
                              std::vector<std::string>& top_die,
                              std::vector<std::string>& bottom_die) {
  for (auto& inst_name : inst_C_index) {
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

void Solver1::concat_line_segment(DIE_INDEX idx, int i) {
  std::vector<line_segment>& h_contour = horizontal_contours[idx];
  if (i > 0 && h_contour[i - 1].y == h_contour[i].y) {
    h_contour[i - 1].to = h_contour[i].to;
    h_contour.erase(h_contour.begin() + i);
    --i; // adjust the index
  }
  if (i < h_contour.size() - 1 && h_contour[i + 1].y == h_contour[i].y) {
    h_contour[i].to = h_contour[i + 1].to;
    h_contour.erase(h_contour.begin() + i + 1);
  }
}

void Solver1::place_macro(DIE_INDEX idx, int& x, int& y, const int width, const int height) {
  std::vector<line_segment>& h_contour = horizontal_contours[idx];
  for(int i = 0; i < h_contour.size(); ++i) {
    line_segment& contour = h_contour[i];

    if((contour.y + height <= case_.size.upper_right_y)){
      if(width == (contour.to - contour.from)) {
        x = contour.from;
        y = contour.y;
        contour.y += height;
        concat_line_segment(idx, i);
        return;
      }
      else if(width < (contour.to - contour.from)) {
        x = contour.from;
        y = contour.y;
        line_segment new_contour;
        new_contour.y = contour.y + height;
        new_contour.from = contour.from;
        new_contour.to = contour.from + width;
        h_contour.insert(h_contour.begin() + i, new_contour);
        contour.from = contour.from + width;
        concat_line_segment(idx, i);
        return;
      }
      else {
        // align with the from point
        if(contour.from + width <= case_.size.upper_right_x) {
          bool overlap = false;
          int j = i + 1;
          while(j < h_contour.size() && contour.from + width > h_contour[j].from) {
            if (contour.y < h_contour[j].y) {
              std::cout << "Error: overlap" << std::endl;
              overlap = true;
              break;
            }
            ++j;
          }

          if (!overlap){
            x = contour.from;
            y = contour.y;
            contour.y = contour.y + height;
            contour.to = contour.from + width;
            --j;
            
            // update the last contour partially overlapping with the new contour and erase the overlapping contours
            if (contour.from + width < h_contour[j].to) {
              h_contour[j].from = contour.from + width;
              h_contour.erase(h_contour.begin() + i + 1, h_contour.begin() + j);
            }
            else{
              h_contour.erase(h_contour.begin() + i + 1, h_contour.begin() + j + 1);
            }

            concat_line_segment(idx, i);
            return;
          }
        }
        // align with the to point
        if(contour.to - width >= 0) {
          bool overlap = false;
          int j = i - 1;
          while(j >= 0 && contour.to - width < h_contour[j].to) {
            if (contour.y < h_contour[j].y) {
              std::cout << "Error: overlap" << std::endl;
              overlap = true;
              break;
            }
            --j;
          }

          if (!overlap){
            x = contour.from;
            y = contour.y;
            contour.y = contour.y + height;
            contour.from = contour.to - width;
            ++j;
            
            // update the last contour partially overlapping with the new contour and erase the overlapping contours
            if (contour.to - width > h_contour[j].from) {
              h_contour[j].to = contour.to - width;
              h_contour.erase(h_contour.begin() + j + 1, h_contour.begin() + i);
            }
            else{
              h_contour.erase(h_contour.begin() + j, h_contour.begin() + i);
            }

            concat_line_segment(idx, i);
            return;
          }
        }
      }
    }
  }
}

void Solver1::place_macro_on_die(DIE_INDEX idx, const std::vector<std::string>& macros) {
  for (auto& inst_name : macros) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);

    int width = case_.get_lib_cell_width(idx, die_cell_index);
    int height = case_.get_lib_cell_height(idx, die_cell_index);
    Inst::rotate orientation = Inst::R0;  // 0: no rotate, 1: rotate 90 degree, 2: rotate 180 degree,
                     // 3: rotate 270 degree

    if (width > height) {
      std::swap(width, height);
      orientation = Inst::R90;
    }

    // place
    int x = 0;
    int y = 0;
    place_macro(idx, x, y, width, height);

    // update
    case_.netlist.placed[inst_name] = true;
    Inst inst;
    inst.name = inst_name;
    inst.loc_x = x;
    inst.loc_y = y;
    inst.orientation = orientation;
    if (idx == TOP) {
      solution_.top_die_insts.push_back(inst);
    } else {
      solution_.bottom_die_insts.push_back(inst);
    }
  }
}

void Solver1::solve() {
  // separate macros and cells
  std::vector<std::string> macro_C_index;
  std::vector<std::string> cell_C_index;
  separate_macros_cells(case_.get_macro_list(), macro_C_index, cell_C_index);

  // decide what die each macro should be placed
  std::vector<std::string> top_die_macros;
  std::vector<std::string> bottom_die_macros;
  decide_what_die(macro_C_index, top_die_macros, bottom_die_macros);

  // sort by (height / width)
  sort_macro(TOP, top_die_macros);
  sort_macro(BOTTOM, bottom_die_macros);

  // place macros on the top and bottom die
  place_macro_on_die(TOP, top_die_macros);
  place_macro_on_die(BOTTOM, bottom_die_macros);

  // decide what die each cell should be placed
  std::vector<std::string> top_die_cells;
  std::vector<std::string> bottom_die_cells;
  decide_what_die(cell_C_index, top_die_cells, bottom_die_cells);

  // sort by width
  sort_cell(TOP, top_die_cells);
  sort_cell(BOTTOM, bottom_die_cells);
}