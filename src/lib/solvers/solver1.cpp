#include "solvers/solver1.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <utility>

#include "data/case.h"

Solver1::Solver1(Case&& case_) : Solver(std::move(case_)) {
  // initialize horizontal contours for top and bottom die (macros)
  horizontal_contours = {{{0, 0, this->case_.size.upper_right_x}},
                         {{0, 0, this->case_.size.upper_right_x}}};

  spared_rows = {
      std::vector<std::vector<std::pair<int, int>>>(
          this->case_.die_infos[DieSide::TOP].rows.repeat_count,
          {{0, this->case_.die_infos[DieSide::TOP].rows.row_length}}),
      std::vector<std::vector<std::pair<int, int>>>(
          this->case_.die_infos[DieSide::BOTTOM].rows.repeat_count,
          {{0, this->case_.die_infos[DieSide::BOTTOM].rows.row_length}})};

  // initialize die size and utilization
  die_size = this->case_.size.upper_right_x * this->case_.size.upper_right_y;
  const double percent = 100.0;
  die_max_util = {this->case_.die_infos[DieSide::TOP].max_util / percent,
                  this->case_.die_infos[DieSide::BOTTOM].max_util / percent};
  die_util = {0.0, 0.0};
}

bool Solver1::check_capacity(DieSide side, int die_cell_index) {
  const double new_util =
      case_.die_infos[side].tech.lib_cells[die_cell_index].get_cell_size() /
      static_cast<double>(die_size);
  return die_util[side] + new_util < die_max_util[side];
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

void Solver1::sort_macro(DieSide side,
                         std::vector<std::string>& macro_C_index) {
  std::sort(macro_C_index.begin(), macro_C_index.end(),
            [&](const std::string& a, const std::string& b) {
              const std::string a_type = case_.netlist.inst[a];
              const std::string b_type = case_.netlist.inst[b];
              const int a_die_cell_index = case_.get_cell_index(a_type);
              const int b_die_cell_index = case_.get_cell_index(b_type);
              double a_cell_size_ratio =
                  case_.get_lib_cell_height(side, a_die_cell_index) /
                  static_cast<double>(
                      case_.get_lib_cell_width(side, a_die_cell_index));
              double b_cell_size_ratio =
                  case_.get_lib_cell_height(side, b_die_cell_index) /
                  static_cast<double>(
                      case_.get_lib_cell_width(side, b_die_cell_index));

              if (a_cell_size_ratio < 1) {
                a_cell_size_ratio = 1 / a_cell_size_ratio;
              }
              if (b_cell_size_ratio < 1) {
                b_cell_size_ratio = 1 / b_cell_size_ratio;
              }

              return a_cell_size_ratio > b_cell_size_ratio;
            });
}

void Solver1::sort_cell(DieSide side, std::vector<std::string>& cell_C_index) {
  std::sort(cell_C_index.begin(), cell_C_index.end(),
            [&](const std::string& a, const std::string& b) {
              const std::string a_type = case_.netlist.inst[a];
              const std::string b_type = case_.netlist.inst[b];
              const int a_die_cell_index = case_.get_cell_index(a_type);
              const int b_die_cell_index = case_.get_cell_index(b_type);
              const int a_cell_size =
                  case_.get_lib_cell_width(side, a_die_cell_index);
              const int b_cell_size =
                  case_.get_lib_cell_width(side, b_die_cell_index);

              return a_cell_size > b_cell_size;
            });
}

void Solver1::decide_what_die(const std::vector<std::string>& inst_C_index,
                              std::vector<std::string>& top_die,
                              std::vector<std::string>& bottom_die) {
  for (const auto& inst_name : inst_C_index) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);

    const bool alter =
        (die_max_util[DieSide::TOP] - die_util[DieSide::TOP]) >
        (die_max_util[DieSide::BOTTOM] - die_util[DieSide::BOTTOM]);

    if (alter && check_capacity(DieSide::TOP, die_cell_index)) {
      die_util[DieSide::TOP] +=
          (case_.get_lib_cell_size(DieSide::TOP, die_cell_index) /
           static_cast<double>(die_size));
      top_die.push_back(inst_name);
    } else if (!alter && check_capacity(DieSide::BOTTOM, die_cell_index)) {
      die_util[DieSide::BOTTOM] +=
          (case_.get_lib_cell_size(DieSide::BOTTOM, die_cell_index) /
           static_cast<double>(die_size));
      bottom_die.push_back(inst_name);
    } else {
      std::cerr << "Macro: Die utilization exceeds maximum utilization"
                << std::endl;
      return;
    }
  }
}

void Solver1::decide_what_die_cell(const std::vector<std::string>& inst_C_index,
                                   std::vector<std::string>& top_die,
                                   std::vector<std::string>& bottom_die) {
  std::set<std::string> decided;

  for (const auto& net : case_.netlist.nets) {
    for (const auto& pin : net.pins) {
      const std::string inst_name = pin.first;
      if (decided.count(inst_name) == 0 &&
          (std::find(inst_C_index.begin(), inst_C_index.end(), inst_name) !=
           inst_C_index.end())) {
        const std::string inst_type = case_.netlist.inst[inst_name];
        const int die_cell_index = case_.get_cell_index(inst_type);

        if (check_capacity(DieSide::TOP, die_cell_index)) {
          die_util[DieSide::TOP] +=
              (case_.get_lib_cell_size(DieSide::TOP, die_cell_index) /
               static_cast<double>(die_size));
          top_die.push_back(inst_name);
          decided.insert(inst_name);
        } else if (check_capacity(DieSide::BOTTOM, die_cell_index)) {
          die_util[DieSide::BOTTOM] +=
              (case_.get_lib_cell_size(DieSide::BOTTOM, die_cell_index) /
               static_cast<double>(die_size));
          bottom_die.push_back(inst_name);
          decided.insert(inst_name);
        } else {
          std::cerr << "Cell: Die utilization exceeds maximum utilization"
                    << std::endl;
        }
      }
    }
  }
}

void Solver1::concat_line_segment(DieSide side, size_t i) {
  std::vector<LineSegment>& contours = horizontal_contours[side];
  if (i > 0 && contours[i - 1].y == contours[i].y) {
    contours[i - 1].to = contours[i].to;
    contours.erase(contours.begin() + i);
    --i;  // adjust the index
  }
  if (i < contours.size() - 1 && contours[i + 1].y == contours[i].y) {
    contours[i].to = contours[i + 1].to;
    contours.erase(contours.begin() + i + 1);
  }
}

bool Solver1::place_macro(DieSide side, int& x, int& y, const int width,
                          const int height) {
  std::vector<LineSegment>& contours = horizontal_contours[side];
  for (size_t i = 0; i < contours.size(); ++i) {
    if (contours[i].y + height <= case_.size.upper_right_y) {
      if (width == (contours[i].to - contours[i].from)) {
        x = contours[i].from;
        y = contours[i].y;
        contours[i].y += height;
        concat_line_segment(side, i);
        return true;
      }
      if (width < (contours[i].to - contours[i].from)) {
        x = contours[i].from;
        y = contours[i].y;
        contours.insert(contours.begin() + i + 1,
                        LineSegment{contours[i].y, contours[i].from + width,
                                    contours[i].to});
        contours[i].y += height;
        contours[i].to = contours[i].from + width;
        concat_line_segment(side, i);
        return true;
      }

      // align with the from point
      if (contours[i].from + width <= case_.size.upper_right_x) {
        bool overlap = false;
        size_t j = i + 1;
        while (j < contours.size() &&
               contours[i].from + width > contours[j].from) {
          if (contours[i].y < contours[j].y) {
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

          concat_line_segment(side, i);
          return true;
        }
      }

      // align with the to point
      if (contours[i].to - width >= 0) {
        int j = i - 1;
        while (j >= 0 && contours[i].to - width < contours[j].to) {
          if (contours[i].y < contours[j].y) {
            std::cerr << "Overlap with other macros" << std::endl;
            return false;
          }
          --j;
        }

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

        concat_line_segment(side, i);
        return true;
      }
    }
  }

  std::cerr << "No available contour" << std::endl;
  return false;
}

void Solver1::place_macro_on_die(DieSide side,
                                 const std::vector<std::string>& macros) {
  for (const auto& inst_name : macros) {
    const std::string& inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    int width = case_.get_lib_cell_width(side, die_cell_index);
    int height = case_.get_lib_cell_height(side, die_cell_index);
    Inst::Rotate orientation = Inst::R0;
    case_.netlist.placed[inst_name] = false;

    if (width > height) {
      std::swap(width, height);
      orientation = Inst::R90;
    }

    // place
    int x = 0;
    int y = 0;
    const bool success = place_macro(side, x, y, width, height);

    if (success) {
      // reverese the placement of macro
      y = case_.size.upper_right_y - y - height;

      const LineSegment& segment = horizontal_contours[side].back();
      if (segment.from >= segment.to) {
        horizontal_contours[side].pop_back();
      }

      // update spared rows
      update_spared_rows(side, x, y, width, height);

      // update
      case_.netlist.placed[inst_name] = true;
      case_.netlist.inst_top_or_bottom[inst_name] = side;

      // update solution
      solution_.die_insts[side].emplace_back(inst_name, x, y, orientation);
    }
  }
}

void Solver1::update_spared_rows(DieSide side, const int x, const size_t y,
                                 const int width, const int height) {
  const int row_height = case_.get_die_row_height(side);
  int top_row_index = (y + height) / row_height;
  int bottom_row_index = y / row_height;

  if (y + height > row_height * spared_rows[side].size()) {
    top_row_index = spared_rows[side].size() - 1;
  } else if ((y + height) % row_height == 0) {
    top_row_index -= 1;
  }

  if (y % row_height == 0 && y > 0) {
    bottom_row_index -= 1;
  }

  const int left = x;
  const int right = x + width;
  for (int i = bottom_row_index; i <= top_row_index; ++i) {
    size_t j = 0;
    while (j < spared_rows[side][i].size() - 1 &&
           left > spared_rows[side][i][j].second) {
      ++j;
    }

    if (left == spared_rows[side][i][j].first &&
        right == spared_rows[side][i][j].second) {
      spared_rows[side][i].erase(spared_rows[side][i].begin() + j);
    } else if (left == spared_rows[side][i][j].first) {
      spared_rows[side][i][j].first = right;
    } else if (right == spared_rows[side][i][j].second) {
      spared_rows[side][i][j].second = left;
    } else {
      std::pair<int, int> new_pair = {right, spared_rows[side][i][j].second};
      spared_rows[side][i][j].second = left;
      if (j == spared_rows[side][i].size() - 1) {
        spared_rows[side][i].push_back(std::move(new_pair));
      } else {
        spared_rows[side][i].insert(spared_rows[side][i].begin() + j + 1,
                                    std::move(new_pair));
      }
    }
  }
}

bool Solver1::place_cell(DieSide side, int& x, int& y, const int width,
                         const int height) {
  std::vector<std::vector<std::pair<int, int>>>& rows = spared_rows[side];
  for (size_t i = 0; i < rows.size(); ++i) {
    if (rows[i].empty()) {
      continue;
    }

    for (size_t j = 0; j < rows[i].size(); ++j) {
      if (rows[i][j].second - rows[i][j].first == width) {
        x = rows[i][j].first;
        y = i * height;
        rows[i].erase(rows[i].begin() + j);
        return true;
      }
      if (width < rows[i][j].second - rows[i][j].first) {
        x = rows[i][j].first;
        y = i * height;
        rows[i][j].first = rows[i][j].first + width;
        return true;
      }
    }
  }

  std::cerr << "No available row" << std::endl;
  return false;
}

void Solver1::place_cell_on_die(DieSide side,
                                const std::vector<std::string>& cells) {
  for (const auto& inst_name : cells) {
    const std::string inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    const int width = case_.get_lib_cell_width(side, die_cell_index);
    const int height = case_.get_lib_cell_height(side, die_cell_index);
    case_.netlist.placed[inst_name] = false;

    int x = 0;
    int y = 0;
    const bool success = place_cell(side, x, y, width, height);

    if (success) {
      case_.netlist.placed[inst_name] = true;
      case_.netlist.inst_top_or_bottom[inst_name] = side;

      // update solution
      solution_.die_insts[side].emplace_back(inst_name, x, y, Inst::R0);
    }
  }
}

void Solver1::draw_macro() {
  ///////////TOP
  std::ofstream draw_macro_cell_file;
  draw_macro_cell_file.open(std::string("top_die_draw_macro_and_cell.txt"));
  // inst number
  //  draw_macro_cell_file << case_.netlist.inst.size() << std::endl;
  draw_macro_cell_file << case_.die_infos[DieSide::TOP].rows.repeat_count +
                              solution_.die_insts[DieSide::TOP].size()
                       << std::endl;
  draw_macro_cell_file << case_.size.upper_right_x << " "
                       << case_.size.upper_right_y << std::endl;
  int count = -1;
  // draw row
  for (int i = 0; i < case_.die_infos[DieSide::TOP].rows.repeat_count; i++) {
    draw_macro_cell_file << count << " " << 0 << " "
                         << i * case_.get_die_row_height(DieSide::TOP) << " "
                         << case_.get_die_row_width(DieSide::TOP) << " "
                         << case_.get_die_row_height(DieSide::TOP) << std::endl;
    count--;
  }
  count = 1;
  // draw macro cell
  for (auto& inst : solution_.die_insts[DieSide::TOP]) {
    const std::string inst_type = case_.netlist.inst[inst.name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    const int width = case_.get_lib_cell_width(DieSide::TOP, die_cell_index);
    const int height = case_.get_lib_cell_height(DieSide::TOP, die_cell_index);
    const bool is_macro =
        case_.get_is_macro(DieSide::TOP, die_cell_index);  // 0:cell 1:macro
    if (is_macro) {
      draw_macro_cell_file << 0 << " ";
    } else {
      draw_macro_cell_file << count << " ";
    }
    if (inst.orientation == Inst::R0) {
      draw_macro_cell_file << inst.loc_x << " " << inst.loc_y << " " << width
                           << " " << height << std::endl;
    } else {
      draw_macro_cell_file << inst.loc_x << " " << inst.loc_y << " " << height
                           << " " << width << std::endl;
    }
    count++;
  }
  draw_macro_cell_file.close();

  //////////BOTTOM
  draw_macro_cell_file.open(std::string("bottom_die_draw_macro_and_cell.txt"));
  // inst number
  //  draw_macro_cell_file << case_.netlist.inst.size() << std::endl;
  draw_macro_cell_file << case_.die_infos[DieSide::BOTTOM].rows.repeat_count +
                              solution_.die_insts[DieSide::BOTTOM].size()
                       << std::endl;
  draw_macro_cell_file << case_.size.upper_right_x << " "
                       << case_.size.upper_right_y << std::endl;
  count = -1;
  // draw row
  for (int i = 0; i < case_.die_infos[DieSide::BOTTOM].rows.repeat_count; i++) {
    draw_macro_cell_file << count << " " << 0 << " "
                         << i * case_.get_die_row_height(DieSide::BOTTOM) << " "
                         << case_.get_die_row_width(DieSide::BOTTOM) << " "
                         << case_.get_die_row_height(DieSide::BOTTOM)
                         << std::endl;
    count--;
  }
  count = 1;
  // draw macro cell
  for (auto& inst : solution_.die_insts[DieSide::BOTTOM]) {
    const std::string inst_type = case_.netlist.inst[inst.name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    const int width = case_.get_lib_cell_width(DieSide::BOTTOM, die_cell_index);
    const int height =
        case_.get_lib_cell_height(DieSide::BOTTOM, die_cell_index);
    const bool is_macro =
        case_.get_is_macro(DieSide::BOTTOM, die_cell_index);  // 0:cell 1:macro
    if (is_macro) {
      draw_macro_cell_file << 0 << " ";
    } else {
      draw_macro_cell_file << count << " ";
    }
    if (inst.orientation == Inst::R0) {
      draw_macro_cell_file << inst.loc_x << " " << inst.loc_y << " " << width
                           << " " << height << std::endl;
    } else {
      draw_macro_cell_file << inst.loc_x << " " << inst.loc_y << " " << height
                           << " " << width << std::endl;
    }
    count++;
  }
}

void Solver1::get_inst_that_not_placed(
    DieSide side, const std::vector<std::string>& inst_C_index,
    std::vector<std::string>& not_placed) {
  for (const auto& inst_name : inst_C_index) {
    if (!case_.netlist.placed[inst_name]) {
      not_placed.push_back(inst_name);
      const std::string inst_type = case_.netlist.inst[inst_name];
      const int die_cell_index = case_.get_cell_index(inst_type);

      if (side == DieSide::TOP) {
        die_util[side] -=
            (case_.get_lib_cell_size(DieSide::TOP, die_cell_index) /
             static_cast<double>(die_size));
        if (check_capacity(DieSide::BOTTOM, die_cell_index)) {
          die_util[DieSide::BOTTOM] +=
              (case_.get_lib_cell_size(DieSide::BOTTOM, die_cell_index) /
               static_cast<double>(die_size));
        } else {
          std::cerr
              << "Has a inst that can't be place on the top nor bottom die"
              << std::endl;
        }
      } else {
        die_util[side] -=
            (case_.get_lib_cell_size(DieSide::BOTTOM, die_cell_index) /
             static_cast<double>(die_size));
        if (check_capacity(DieSide::TOP, die_cell_index)) {
          die_util[DieSide::TOP] +=
              (case_.get_lib_cell_size(DieSide::TOP, die_cell_index) /
               static_cast<double>(die_size));
        } else {
          std::cerr
              << "Has a inst that can't be place on the top nor bottom die"
              << std::endl;
        }
      }
    }
  }
}

void Solver1::add_terminal(std::string net_name, int terminal_index) {
  const int terminal_size = case_.terminal.size_x;
  const int virtual_terminal_size =
      case_.terminal.size_x + case_.terminal.spacing;

  const int virtual_max_width =
      case_.size.upper_right_x - 2 * case_.terminal.spacing;
  // const int virtual_max_height =
  //     case_.size.upper_right_y - 2 * case_.terminal.spacing;

  int max_num_x = virtual_max_width / virtual_terminal_size;
  if (terminal_size <= (virtual_max_width % virtual_terminal_size) - 0.5) {
    max_num_x++;
  }
  // int max_num_y = virtual_max_height / virtual_terminal_size;
  // if (terminal_size <= (virtual_max_height % virtual_terminal_size) - 0.5) {
  //   max_num_y++;
  // }

  // std::cout << "max_num_x: " << max_num_x << std::endl;
  // std::cout << "max_num_y: " << max_num_y << std::endl;

  const int terminal_col_index = terminal_index % max_num_x;
  const int terminal_row_index = terminal_index / max_num_x;

  const int loc_x = case_.terminal.spacing +
                    terminal_col_index * virtual_terminal_size +
                    terminal_size / 2;
  const int loc_y = case_.terminal.spacing +
                    terminal_row_index * virtual_terminal_size +
                    terminal_size / 2;
  solution_.terminals.emplace_back(std::move(net_name), loc_x, loc_y);
}

void Solver1::place_terminal() {
  int index = 0;

  // place terminal
  for (auto& net : case_.netlist.nets) {
    std::array<bool, 2> status = {false, false};
    for (auto& pin : net.pins) {
      const std::string inst_name = pin.first;
      const int pre_status = case_.netlist.inst_top_or_bottom[inst_name];
      status[pre_status] = true;
      // both die have this net
      if (status[DieSide::TOP] && status[DieSide::BOTTOM]) {
        add_terminal(net.name, index);
        index++;
        break;
      }
    }
  }

  // check terminal size even or odd
  if (case_.terminal.size_x % 2 == 1) {
    for (auto& t : solution_.terminals) {
      ++t.loc_x;
      ++t.loc_y;
    }
  }
}

void Solver1::draw_terminal() {
  std::ofstream draw_terminal_file;
  draw_terminal_file.open(std::string("draw_terminal.txt"));
  draw_terminal_file << solution_.terminals.size() << std::endl;
  draw_terminal_file << case_.size.upper_right_x << " "
                     << case_.size.upper_right_y << std::endl;
  int count = 1;
  for (auto& t : solution_.terminals) {
    draw_terminal_file << count << " " << t.loc_x << " " << t.loc_y << " "
                       << case_.terminal.size_x << " " << case_.terminal.size_x
                       << std::endl;
    count++;
  }

  draw_terminal_file.close();
}

bool Solver1::check_macro_numbers(size_t macro_numbers) {
  return solution_.die_insts[DieSide::TOP].size() +
             solution_.die_insts[DieSide::BOTTOM].size() ==
         macro_numbers;
}

void Solver1::solve() {
  // separate macros and cells
  std::vector<std::string> macro_C_index;
  std::vector<std::string> cell_C_index;
  separate_macros_cells(case_.get_macro_list(), macro_C_index, cell_C_index);

  // macro
  // decide what die each macro should be placed
  std::vector<std::string> top_die_macros;
  std::vector<std::string> bottom_die_macros;
  decide_what_die(macro_C_index, top_die_macros, bottom_die_macros);

  // sort by (height / width)
  sort_macro(DieSide::TOP, top_die_macros);
  sort_macro(DieSide::BOTTOM, bottom_die_macros);

  // place macros on the top and bottom die
  place_macro_on_die(DieSide::TOP, top_die_macros);
  place_macro_on_die(DieSide::BOTTOM, bottom_die_macros);

  // get macros that are not placed
  std::vector<std::string> top_not_placed_macros;
  std::vector<std::string> bottom_not_placed_macros;
  get_inst_that_not_placed(DieSide::TOP, top_die_macros, top_not_placed_macros);
  get_inst_that_not_placed(DieSide::BOTTOM, bottom_die_macros,
                           bottom_not_placed_macros);

  // sort by (height / width)
  if (!bottom_not_placed_macros.empty()) {
    sort_macro(DieSide::TOP, bottom_not_placed_macros);
  }
  if (!top_not_placed_macros.empty()) {
    sort_macro(DieSide::BOTTOM, top_not_placed_macros);
  }

  // place them on the other die
  if (!bottom_not_placed_macros.empty()) {
    place_macro_on_die(DieSide::TOP, bottom_not_placed_macros);
  }
  if (!top_not_placed_macros.empty()) {
    place_macro_on_die(DieSide::BOTTOM, top_not_placed_macros);
  }

  // cell
  // decide what die each cell should be placed
  std::vector<std::string> top_die_cells;
  std::vector<std::string> bottom_die_cells;
  decide_what_die_cell(cell_C_index, top_die_cells, bottom_die_cells);

  // sort by width
  sort_cell(DieSide::TOP, top_die_cells);
  sort_cell(DieSide::BOTTOM, bottom_die_cells);

  // place cells on the top and bottom die
  place_cell_on_die(DieSide::TOP, top_die_cells);
  place_cell_on_die(DieSide::BOTTOM, bottom_die_cells);

  // get cells that are not placed
  std::vector<std::string> top_not_placed_cells;
  std::vector<std::string> bottom_not_placed_cells;
  get_inst_that_not_placed(DieSide::TOP, top_die_cells, top_not_placed_cells);
  get_inst_that_not_placed(DieSide::BOTTOM, bottom_die_cells,
                           bottom_not_placed_cells);

  // sort by width
  if (!bottom_not_placed_cells.empty()) {
    sort_cell(DieSide::TOP, bottom_not_placed_cells);
  }
  if (!top_not_placed_cells.empty()) {
    sort_cell(DieSide::BOTTOM, top_not_placed_cells);
  }

  // place them on the other die
  if (!bottom_not_placed_macros.empty()) {
    place_cell_on_die(DieSide::TOP, bottom_not_placed_cells);
  }
  if (!top_not_placed_cells.empty()) {
    place_cell_on_die(DieSide::BOTTOM, top_not_placed_cells);
  }

  // terminal
  place_terminal();
}