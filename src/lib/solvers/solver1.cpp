#include "solvers/solver1.h"

#include <algorithm>

#include "data/case.h"

#include "fstream"

Solver1::Solver1(Case& case_) : Solver(case_) {
  // initialize horizontal contours for top and bottom die (macros)
  horizontal_contours = {{{0, 0, case_.size.upper_right_x}},
                         {{0, 0, case_.size.upper_right_x}}};

  spared_rows = {std::vector<std::vector<std::pair<int, int>>>(
                     case_.top_die.rows.repeat_count,
                     {{0, case_.top_die.rows.row_length}}),
                 std::vector<std::vector<std::pair<int, int>>>(
                     case_.bottom_die.rows.repeat_count,
                     {{0, case_.bottom_die.rows.row_length}})};

  // initialize die size and utilization
  die_size = case_.size.upper_right_x * case_.size.upper_right_y;
  const int percent = 100;
  die_max_util = {float(case_.top_die.max_util) / percent,
                  float(case_.bottom_die.max_util) / percent};
  die_util = {0, 0};
}

bool Solver1::check_capacity(int index, int die_cell_index) {
  if (index == TOP) {
    return (die_util[TOP] +
               (case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
                   die_size)) <=
           die_max_util[TOP];
  }
  else{
    return (die_util[BOTTOM] +
               (case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
                   die_size)) <=
           die_max_util[BOTTOM];
  }
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

void Solver1::sort_macro(DIE_INDEX idx,
                         std::vector<std::string>& macro_C_index) {
  std::sort(macro_C_index.begin(), macro_C_index.end(),
            [&](const std::string& a, const std::string& b) {
              const std::string a_type = case_.netlist.inst[a];
              const std::string b_type = case_.netlist.inst[b];
              const int a_die_cell_index = case_.get_cell_index(a_type);
              const int b_die_cell_index = case_.get_cell_index(b_type);
              double a_cell_size_ratio =
                  case_.get_lib_cell_height(idx, a_die_cell_index) /
                  double(case_.get_lib_cell_width(idx, a_die_cell_index));
              double b_cell_size_ratio =
                  case_.get_lib_cell_height(idx, b_die_cell_index) /
                  double(case_.get_lib_cell_width(idx, b_die_cell_index));

              if (a_cell_size_ratio < 1) {
                a_cell_size_ratio = 1 / a_cell_size_ratio;
              }
              if (b_cell_size_ratio < 1) {
                b_cell_size_ratio = 1 / b_cell_size_ratio;
              }

              return a_cell_size_ratio > b_cell_size_ratio;
            });
}

void Solver1::sort_cell(DIE_INDEX idx, std::vector<std::string>& cell_C_index) {
  std::sort(
      cell_C_index.begin(), cell_C_index.end(),
      [&](const std::string& a, const std::string& b) {
        const std::string a_type = case_.netlist.inst[a];
        const std::string b_type = case_.netlist.inst[b];
        const int a_die_cell_index = case_.get_cell_index(a_type);
        const int b_die_cell_index = case_.get_cell_index(b_type);
        int a_cell_size = case_.get_lib_cell_width(idx, a_die_cell_index);
        int b_cell_size = case_.get_lib_cell_width(idx, b_die_cell_index);

        return a_cell_size > b_cell_size;
      });
}

void Solver1::decide_what_die(const std::vector<std::string>& inst_C_index,
                              std::vector<std::string>& top_die,
                              std::vector<std::string>& bottom_die) {
  for (const auto& inst_name : inst_C_index) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);

    const bool alter = (die_max_util[TOP] - die_util[TOP]) >
                       (die_max_util[BOTTOM] - die_util[BOTTOM]);

    // std::cout << die_util[TOP] << " " << die_util[BOTTOM] << std::endl;

    if ((alter) && (check_capacity(TOP, die_cell_index))) {
      die_util[TOP] +=
          (case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
          float(die_size));
      top_die.push_back(inst_name);
    } else if ((!alter) && (check_capacity(BOTTOM, die_cell_index))) {
      die_util[BOTTOM] +=
          (case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
          float(die_size));
      bottom_die.push_back(inst_name);
    } else {
      std::cerr << "Die utilization exceeds maximum utilization" << std::endl;
      return;
    }
  }
}

void Solver1::concat_line_segment(DIE_INDEX idx, int i) {
  std::vector<line_segment>& contours = horizontal_contours[idx];
  if ((i > 0) && (contours[i - 1].y == contours[i].y)) {
    contours[i - 1].to = contours[i].to;
    contours.erase(contours.begin() + i);
    --i;  // adjust the index
  }
  if ((i < contours.size() - 1) && (contours[i + 1].y == contours[i].y)) {
    std::cout << 'a' << std::endl;
    contours[i].to = contours[i + 1].to;
    contours.erase(contours.begin() + i + 1);
  }
}

bool Solver1::place_macro(DIE_INDEX idx, int& x, int& y, const int width,
                          const int height) {
  std::vector<line_segment>& contours = horizontal_contours[idx];
  for (int i = 0; i < contours.size(); ++i) {
    if ((contours[i].y + height <= case_.size.upper_right_y)) {
      if (width == (contours[i].to - contours[i].from)) {
        x = contours[i].from;
        y = contours[i].y;
        contours[i].y += height;
        concat_line_segment(idx, i);
        return true;
      }
      if (width < (contours[i].to - contours[i].from)) {
        x = contours[i].from;
        y = contours[i].y;
        line_segment new_contour;
        new_contour.y = contours[i].y;
        new_contour.from = contours[i].from + width;
        new_contour.to = contours[i].to;
        contours.insert(contours.begin() + i + 1, new_contour);
        contours[i].y += height;
        contours[i].to = contours[i].from + width;
        concat_line_segment(idx, i);
        return true;
      }

      // align with the from point
      if (contours[i].from + width <= case_.size.upper_right_x) {
        bool overlap = false;
        int j = i + 1;
        while (j < contours.size() && contours[i].from + width > contours[j].from) {
          if (contours[i].y < contours[j].y) {
            std::cout << "Error: overlap with other macros" << std::endl;
            overlap = true;
            break;
          }
          ++j;
        }

        if (!overlap) {
          x = contours[i].from;
          y = contours[i].y;
          contours[i].y = contours[i].y + height;
          contours[i].to = contours[i].from + width;
          --j;

          // update the last contour partially overlapping with the new
          // contour and erase the overlapping contours
          if (contours[i].from + width < contours[j].to) {
            contours[j].from = contours[i].from + width;
            contours.erase(contours.begin() + i + 1, contours.begin() + j);
          } else {
            contours.erase(contours.begin() + i + 1, contours.begin() + j + 1);
          }

          concat_line_segment(idx, i);
          return true;
        }
      }

      // align with the to point
      if (contours[i].to - width >= 0) {
        bool overlap = false;
        int j = i - 1;
        while (j >= 0 && contours[i].to - width < contours[j].to) {
          if (contours[i].y < contours[j].y) {
            std::cout << "Error: overlap with other macros" << std::endl;
            overlap = true;
            break;
          }
          --j;
        }

        if (!overlap) {
          x = contours[i].from;
          y = contours[i].y;
          contours[i].y = contours[i].y + height;
          contours[i].from = contours[i].to - width;
          ++j;

          // update the last contour partially overlapping with the new
          // contour and erase the overlapping contours
          if (contours[i].to - width > contours[j].from) {
            contours[j].to = contours[i].to - width;
            contours.erase(contours.begin() + j + 1, contours.begin() + i);
          } else {
            contours.erase(contours.begin() + j, contours.begin() + i);
          }

          concat_line_segment(idx, i);
          return true;
        }
      }
    }
  }

  std::cout << "Error: no available contour" << std::endl;
  return false;
}

void Solver1::place_macro_on_die(DIE_INDEX idx,
                                 const std::vector<std::string>& macros) {
  for (const auto& inst_name : macros) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    int width = case_.get_lib_cell_width(idx, die_cell_index);
    int height = case_.get_lib_cell_height(idx, die_cell_index);
    Inst::rotate orientation = Inst::R0;
    case_.netlist.placed[inst_name] = false;

    if (width > height) {
      std::swap(width, height);
      orientation = Inst::R90;
    }

    // place
    int x = 0;
    int y = 0;
    bool success = place_macro(idx, x, y, width, height);
    // std::cout << idx << " " << inst_name << " " << x << " " << y << " "
    //           << horizontal_contours[BOTTOM].size() << std::endl;

    if (success) {
      std::cout << idx << " " << inst_name << " " << x << " " << y << std::endl;
      // update spared rows
      update_spared_rows(idx, x, y, width, height);

      // update
      case_.netlist.placed[inst_name] = true;
      case_.netlist.inst_top_or_bottom[inst_name] = idx;

      // update solution
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
}

void Solver1::update_spared_rows(DIE_INDEX idx, const int x, const int y,
                                 const int width, const int height) {
  const int row_height = case_.get_die_row_height(idx);
  int top_row_index = (y + height) / row_height;
  int bottom_row_index = y / row_height;

  if ((((y + height) % row_height) == 0) ||
      ((y + height) > row_height * (spared_rows[idx].size()))) {
    top_row_index -= 1;
  }
  if (((y % row_height) == 0) && (y != 0)) {
    bottom_row_index -= 1;
  }

  const int left = x;
  const int right = x + width;
  for (int i = bottom_row_index; i <= top_row_index; ++i) {
    int j = 0;
    while ((j < spared_rows[idx][i].size() - 1) && (left > spared_rows[idx][i][j].second)) {
      ++j;
    }
    if ((left == spared_rows[idx][i][j].first) && (right == spared_rows[idx][i][j].second)) {
      spared_rows[idx][i].erase(spared_rows[idx][i].begin() + j);
    } else if (left == spared_rows[idx][i][j].first) {
      spared_rows[idx][i][j].first = right;
    } else if (right == spared_rows[idx][i][j].second) {
      spared_rows[idx][i][j].second = left;
    } else {
      std::pair<int, int> new_pair;
      new_pair.first = right;
      new_pair.second = spared_rows[idx][i][j].second;
      spared_rows[idx][i][j].second = left;
      spared_rows[idx][i].insert(spared_rows[idx][i].begin() + j + 1, new_pair);
    }
  }
}

bool Solver1::place_cell(DIE_INDEX idx, int& x, int& y, const int width,
                         const int height) {
  std::vector<std::vector<std::pair<int, int>>>& rows = spared_rows[idx];
  for (int i = 0; i < rows.size(); ++i) {
    if (rows[i].empty()) {
      continue;
    }

    for (int j = 0; j < rows[i].size(); ++j) {
      if ((rows[i][j].second - rows[i][j].first) == width) {
        x = rows[i][j].first;
        y = i * height;
        rows[i].erase(rows[i].begin() + j);
        return true;
      }
      if (width < (rows[i][j].second - rows[i][j].first)) {
        x = rows[i][j].first;
        y = i * height;
        rows[i][j].first = rows[i][j].first + width;
        return true;
      }
    }
  }

  std::cout << "Error: no available row" << std::endl;
  return false;
}

void Solver1::place_cell_on_die(DIE_INDEX idx,
                                const std::vector<std::string>& cells) {
  for (const auto& inst_name : cells) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    const int width = case_.get_lib_cell_width(idx, die_cell_index);
    const int height = case_.get_lib_cell_height(idx, die_cell_index);
    case_.netlist.placed[inst_name] = false;

    int x = 0;
    int y = 0;
    bool success = place_cell(idx, x, y, width, height);
    // std::cout << idx << " " << inst_name << " " << x << " " << y << std::endl;

    if (success) {
      // std::cout << idx << " " << inst_name << " " << x << " " << y << std::endl;
      // update
      case_.netlist.placed[inst_name] = true;
      case_.netlist.inst_top_or_bottom[inst_name] = idx;

      // update solution
      Inst inst;
      inst.name = inst_name;
      inst.loc_x = x;
      inst.loc_y = y;
      inst.orientation = Inst::R0;
      if (idx == TOP) {
        solution_.top_die_insts.push_back(inst);
      } else {
        solution_.bottom_die_insts.push_back(inst);
      }
    }
  }
}

void Solver1::draw_macro(){

    ///////////TOP
    std::ofstream draw_macro_cell_file;
    draw_macro_cell_file.open(std::string("top_die_draw_macro_and_cell.txt"));
    //inst number
    // draw_macro_cell_file << case_.netlist.inst.size() << std::endl;
    draw_macro_cell_file << case_.top_die.rows.repeat_count + solution_.top_die_insts.size() << std::endl;
    draw_macro_cell_file << case_.size.upper_right_x << " "  << case_.size.upper_right_y << std::endl;
    int count = -1;
    //draw row
    for(int i = 0;i < case_.top_die.rows.repeat_count;i++){
      draw_macro_cell_file << count << " " << 0 << " " << i* case_.get_die_row_height(TOP) << " " << case_.get_die_row_width(TOP) << " " << case_.get_die_row_height(TOP) << std::endl;
      count --;
    }  
    count = 1;
    //draw macro cell
    for(auto& inst : solution_.top_die_insts){
      std::string inst_type = case_.netlist.inst[inst.name];
      const int die_cell_index = case_.get_cell_index(inst_type);
      const int width = case_.get_lib_cell_width(TOP, die_cell_index);
      const int height = case_.get_lib_cell_height(TOP, die_cell_index);
      const int is_macro = case_.get_is_macro(TOP, die_cell_index); //0:cell 1:macro
      if(is_macro == 1)
        draw_macro_cell_file << 0 << " ";
      else
        draw_macro_cell_file << count << " ";
      if (inst.orientation == Inst::R0)
        draw_macro_cell_file  << inst.loc_x << " " << inst.loc_y  << " " <<  width << " " << height << std::endl;
      else 
        draw_macro_cell_file  << inst.loc_x << " " << inst.loc_y  << " " <<  height << " " << width << std::endl;
      count ++;
    }
    draw_macro_cell_file.close();

    //////////BOTTOM
    draw_macro_cell_file.open(std::string("bottom_die_draw_macro_and_cell.txt"));
    //inst number
    // draw_macro_cell_file << case_.netlist.inst.size() << std::endl;
    draw_macro_cell_file << case_.bottom_die.rows.repeat_count + solution_.bottom_die_insts.size() << std::endl;
    draw_macro_cell_file << case_.size.upper_right_x << " "  << case_.size.upper_right_y << std::endl;
    count = -1;
    //draw row
    for(int i = 0;i < case_.bottom_die.rows.repeat_count;i++){
      draw_macro_cell_file << count << " " << 0 << " " << i* case_.get_die_row_height(BOTTOM) << " " << case_.get_die_row_width(BOTTOM) << " " << case_.get_die_row_height(BOTTOM) << std::endl;
      count --;
    }
    count = 1;
    //draw macro cell
    for(auto& inst : solution_.bottom_die_insts){
      std::string inst_type = case_.netlist.inst[inst.name];
      const int die_cell_index = case_.get_cell_index(inst_type);
      const int width = case_.get_lib_cell_width(BOTTOM, die_cell_index);
      const int height = case_.get_lib_cell_height(BOTTOM, die_cell_index);
      const int is_macro = case_.get_is_macro(BOTTOM, die_cell_index); //0:cell 1:macro
      if(is_macro == 1)
        draw_macro_cell_file << 0 << " ";
      else
        draw_macro_cell_file << count << " ";
      if (inst.orientation == Inst::R0)
        draw_macro_cell_file << inst.loc_x << " " << inst.loc_y  << " " <<  width << " " << height << std::endl;
      else 
        draw_macro_cell_file << inst.loc_x << " " << inst.loc_y  << " " <<  height << " " << width << std::endl;
      count ++;
    }
}

void Solver1::get_inst_that_not_placed(DIE_INDEX idx, const std::vector<std::string>& inst_C_index
                                        , std::vector<std::string>& not_placed) {
  for (const auto& inst_name : inst_C_index) {
    if (!case_.netlist.placed[inst_name]) {
      not_placed.push_back(inst_name);
      const std::string inst_type = case_.netlist.inst[inst_name];
      const int die_cell_index = case_.get_cell_index(inst_type);

      if (idx == TOP){
        die_util[idx] -= (case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
                            float(die_size));
        if (check_capacity(BOTTOM, die_cell_index)){
          die_util[BOTTOM] += (case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
                            float(die_size));
        }
        else {
          std::cerr << "Has a macro that can't be place on the top nor bottom die" << std::endl;
        }
      }
      else {
        die_util[idx] -= (case_.bottom_die.tech.lib_cells[die_cell_index].get_cell_size() /
                            float(die_size));
        if (check_capacity(TOP, die_cell_index)){
          die_util[TOP] += (case_.top_die.tech.lib_cells[die_cell_index].get_cell_size() /
                            float(die_size));
        }
        else {
          std::cerr << "Has a macro that can't be place on the top nor bottom die" << std::endl;
        }
      }
      
    }
  }
}

void Solver1::place_terminal() {
  // to-do
}

void Solver1::solve() {
  // separate macros and cells
  std::vector<std::string> macro_C_index;
  std::vector<std::string> cell_C_index;
  separate_macros_cells(case_.get_macro_list(), macro_C_index, cell_C_index);

  // macro
  std::cout << "macro" << std::endl;
  // decide what die each macro should be placed
  std::vector<std::string> top_die_macros;
  std::vector<std::string> bottom_die_macros;
  decide_what_die(macro_C_index, top_die_macros, bottom_die_macros); 
  // error: case2: there are macros that can't be placed on the top nor bottom die

  // sort by (height / width)
  sort_macro(TOP, top_die_macros);
  sort_macro(BOTTOM, bottom_die_macros);

  // place macros on the top and bottom die
  std::cout << "place macro" << std::endl; // error: case3: core dumped
  place_macro_on_die(TOP, top_die_macros);
  place_macro_on_die(BOTTOM, bottom_die_macros);
  std::cout << "place macro done" << std::endl;

  // get macros that are not placed
  std::vector<std::string> top_not_placed_macros;
  std::vector<std::string> bottom_not_placed_macros;
  get_inst_that_not_placed(TOP, top_die_macros, top_not_placed_macros);
  get_inst_that_not_placed(BOTTOM, bottom_die_macros, bottom_not_placed_macros);

  // sort by (height / width)
  sort_macro(TOP, bottom_not_placed_macros);
  sort_macro(BOTTOM, top_not_placed_macros);

  // place them on the other die
  place_macro_on_die(TOP, bottom_not_placed_macros);
  place_macro_on_die(BOTTOM, top_not_placed_macros);

  // cell
  std::cout << "cell" << std::endl;
  // decide what die each cell should be placed
  std::vector<std::string> top_die_cells;
  std::vector<std::string> bottom_die_cells;
  decide_what_die(cell_C_index, top_die_cells, bottom_die_cells);

  // sort by width
  sort_cell(TOP, top_die_cells);
  sort_cell(BOTTOM, bottom_die_cells);

  // place cells on the top and bottom die
  place_cell_on_die(TOP, top_die_cells);
  place_cell_on_die(BOTTOM, bottom_die_cells);

  // get cells that are not placed
  std::vector<std::string> top_not_placed_cells;
  std::vector<std::string> bottom_not_placed_cells;
  get_inst_that_not_placed(TOP, top_die_cells, top_not_placed_cells);
  get_inst_that_not_placed(BOTTOM, bottom_die_cells, bottom_not_placed_cells);

  // sort by width
  sort_cell(TOP, bottom_not_placed_cells);
  sort_cell(BOTTOM, top_not_placed_cells);

  // place them on the other die
  place_cell_on_die(TOP, bottom_not_placed_cells);
  place_cell_on_die(BOTTOM, top_not_placed_cells);

  // terminal
  place_terminal();

  //draw macro
  draw_macro();
}