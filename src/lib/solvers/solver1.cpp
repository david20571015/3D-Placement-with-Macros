#include "solvers/solver1.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <set>
#include <utility>

#include "data/case.h"
#include "solvers/btree.h"

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
  die_size = static_cast<int64_t>(this->case_.size.upper_right_x) *
             (this->case_.size.upper_right_y);
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
        (die_max_util[DieSide::TOP] - die_util[DieSide::TOP]) >=
        (die_max_util[DieSide::BOTTOM] - die_util[DieSide::BOTTOM]);

    if ((alter) && (check_capacity(DieSide::TOP, die_cell_index))) {
      die_util[DieSide::TOP] +=
          (case_.get_lib_cell_size(DieSide::TOP, die_cell_index) /
           static_cast<double>(die_size));
      top_die.push_back(inst_name);
    } else if ((!alter) && (check_capacity(DieSide::BOTTOM, die_cell_index))) {
      die_util[DieSide::BOTTOM] +=
          (case_.get_lib_cell_size(DieSide::BOTTOM, die_cell_index) /
           static_cast<double>(die_size));
      bottom_die.push_back(inst_name);
    } else {
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
      const std::string inst_type = case_.netlist.inst[inst_name];
      const int die_cell_index = case_.get_cell_index(inst_type);

      if ((case_.get_is_macro(DieSide::TOP, die_cell_index))) {
        continue;
      }

      if (decided.count(inst_name) == 0) {
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
        }
      }
    }
  }

  if (decided.size() != inst_C_index.size()) {
    for (const auto& inst_name : inst_C_index) {
      const std::string inst_type = case_.netlist.inst[inst_name];
      const int die_cell_index = case_.get_cell_index(inst_type);

      if (decided.count(inst_name) == 0) {
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

  if (y + height >
      static_cast<int64_t>(row_height) * spared_rows[side].size()) {
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
    size_t left_j = 0;
    while ((left < spared_rows[side][i].size() - 1) &&
           (left > spared_rows[side][i][left_j].second)) {
      ++left_j;
    }
    size_t right_j = left_j;
    while ((right < spared_rows[side][i].size() - 1) &&
           (right > spared_rows[side][i][right_j].second)) {
      ++right_j;
    }

    for (size_t k = left_j; k <= right_j; ++k) {
      if ((left > spared_rows[side][i][k].first) &&
          (right < spared_rows[side][i][k].second)) {
        const int tmp = spared_rows[side][i][k].second;
        spared_rows[side][i][k].second = left;
        spared_rows[side][i].insert(spared_rows[side][i].begin() + k,
                                    {right, tmp});
        ++k;
        ++right_j;
      } else if ((left > spared_rows[side][i][k].first) ||
                 (right == spared_rows[side][i][k].second)) {
        spared_rows[side][i][k].second = left;
      } else if ((right < spared_rows[side][i][k].second) ||
                 (left == spared_rows[side][i][k].first)) {
        spared_rows[side][i][k].first = right;
      } else if ((left == spared_rows[side][i][k].first) &&
                 (right == spared_rows[side][i][k].second)) {
        spared_rows[side][i].erase(spared_rows[side][i].begin() + k);
        --k;
        --right_j;
      }
    }
  }
}

void Solver1::Btree_place_macro(Btree& btree, DieSide side,
                                std::vector<std::string> die_macros,
                                int die_width, int die_height) {
  double diff_time = 0;
  const std::string trash;
  clock_t lim_start;
  clock_t lim_end;
  btree.limx = die_width;
  btree.limy = die_height;
  const int macros_number = die_macros.size();
  btree.n = macros_number;
  btree.m = 0;
  btree.netnum = 0;

  for (int i = 1; i <= macros_number; ++i) {
    // macro name
    const std::string& s = die_macros[i - 1];
    const std::string& inst_type = case_.netlist.inst[s];
    const int die_cell_index = case_.get_cell_index(inst_type);
    const int width = case_.get_lib_cell_width(side, die_cell_index);
    const int height = case_.get_lib_cell_height(side, die_cell_index);

    btree.pr[i].first = width;
    btree.pr[i].second = height;
    btree.block_name[i] = s;
    btree.mp[s] = i;
    btree.tot_area += btree.pr[i].first * btree.pr[i].second;
  }

  btree.final_cost = 2e9;

  while (diff_time / CLOCKS_PER_SEC < 30) {
    lim_start = clock();
    btree.init_tree();
    btree.SA();
    btree.update_final();
    lim_end = clock();
    diff_time += (lim_end - lim_start);
  }

  for (int i = 1; i < macros_number + 1; ++i) {
    const std::string& inst_name = die_macros[i - 1];
    const std::string& inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    const int width = case_.get_lib_cell_width(side, die_cell_index);
    const int height = case_.get_lib_cell_height(side, die_cell_index);

    const int x = btree.fx[i];
    const int y = case_.size.upper_right_y - btree.fy[i] - height;
    const Inst::Rotate orientation =
        (x + width == btree.frx[i]) ? Inst::R0 : Inst::R90;

    // update spared rows
    update_spared_rows(side, x, y, width, height);

    // update
    case_.netlist.placed[inst_name] = true;
    case_.netlist.inst_top_or_bottom[inst_name] = side;

    // update solution
    solution_.die_insts[side].emplace_back(inst_name, x, y, orientation);
  }

  std::ofstream btree_draw_macro;
  btree_draw_macro.open(std::string("btree_draw_macro.txt"));
  btree_draw_macro << macros_number + spared_rows[0].size() << std::endl;
  btree_draw_macro << die_width << " " << die_height << std::endl;
  int count = -1;
  for (int i = 0; i < case_.die_infos[DieSide::BOTTOM].rows.repeat_count; i++) {
    btree_draw_macro << count << " " << 0 << " "
                     << i * case_.get_die_row_height(DieSide::BOTTOM) << " "
                     << case_.get_die_row_width(DieSide::BOTTOM) << " "
                     << case_.get_die_row_height(DieSide::BOTTOM) << std::endl;
    count--;
  }
  count = 1;
  // draw macro cell
  for (int i = 1; i < macros_number + 1; ++i) {
    btree_draw_macro << count << " " << btree.fx[i] << " " << btree.fy[i] << " "
                     << btree.frx[i] - btree.fx[i] << " "
                     << btree.fry[i] - btree.fy[i] << std::endl;
    count++;
  }
  btree_draw_macro.close();
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
  // TOP
  std::ofstream draw_macro_cell_file;
  draw_macro_cell_file.open(std::string("top_die_draw_macro_and_cell.txt"));
  // inst number
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

  // BOTTOM
  draw_macro_cell_file.open(std::string("bottom_die_draw_macro_and_cell.txt"));
  // inst number
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
        }
      } else {
        die_util[side] -=
            (case_.get_lib_cell_size(DieSide::BOTTOM, die_cell_index) /
             static_cast<double>(die_size));
        if (check_capacity(DieSide::TOP, die_cell_index)) {
          die_util[DieSide::TOP] +=
              (case_.get_lib_cell_size(DieSide::TOP, die_cell_index) /
               static_cast<double>(die_size));
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

  int max_num_x = virtual_max_width / virtual_terminal_size;
  if (terminal_size <= (virtual_max_width % virtual_terminal_size) - 0.5) {
    max_num_x++;
  }

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

bool Solver1::check_cell_numbers(std::vector<std::string>& cell_C_index) {
  return std::all_of(cell_C_index.begin(), cell_C_index.end(),
                     [&](const std::string& inst_name) {
                       return case_.netlist.placed[inst_name];
                     });
}

void Solver1::initialize_macro() {
  // initailize macro
  solution_.die_insts[DieSide::TOP].clear();
  solution_.die_insts[DieSide::BOTTOM].clear();
  horizontal_contours[DieSide::TOP].clear();
  horizontal_contours[DieSide::BOTTOM].clear();
  spared_rows[DieSide::TOP].clear();
  spared_rows[DieSide::BOTTOM].clear();

  case_.netlist.placed.clear();
  case_.netlist.inst_top_or_bottom.clear();

  horizontal_contours = {{{0, 0, this->case_.size.upper_right_x}},
                         {{0, 0, this->case_.size.upper_right_x}}};

  spared_rows = {
      std::vector<std::vector<std::pair<int, int>>>(
          this->case_.die_infos[DieSide::TOP].rows.repeat_count,
          {{0, this->case_.die_infos[DieSide::TOP].rows.row_length}}),
      std::vector<std::vector<std::pair<int, int>>>(
          this->case_.die_infos[DieSide::BOTTOM].rows.repeat_count,
          {{0, this->case_.die_infos[DieSide::BOTTOM].rows.row_length}})};
}

void Solver1::place_macro_on_die_version2(
    DieSide side, const std::vector<std::string>& macros) {
  for (const auto& inst_name : macros) {
    const std::string& inst_type = case_.netlist.inst[inst_name];
    const int die_cell_index = case_.get_cell_index(inst_type);
    int width = case_.get_lib_cell_width(side, die_cell_index);
    int height = case_.get_lib_cell_height(side, die_cell_index);
    Inst::Rotate orientation = Inst::R0;
    case_.netlist.placed[inst_name] = false;

    if (width < height) {
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
    } else {
      // rotate macro and place again
      if (orientation == Inst::R0) {
        orientation = Inst::R90;
      } else {
        orientation = Inst::R0;
      }
      std::swap(width, height);
      x = 0;
      y = 0;
      const bool success_ = place_macro(side, x, y, width, height);

      if (success_) {
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
}

void Solver1::solve() {
  // separate macros and cells
  std::vector<std::string> macro_C_index;
  std::vector<std::string> cell_C_index;
  separate_macros_cells(case_.get_macro_list(), macro_C_index, cell_C_index);

  // macro
  // first version
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
    // bottom_not_placed_macros.size() << std::endl;
    place_macro_on_die(DieSide::TOP, bottom_not_placed_macros);
  }
  if (!top_not_placed_macros.empty()) {
    place_macro_on_die(DieSide::BOTTOM, top_not_placed_macros);
  }

  // second version
  if (!check_macro_numbers(macro_C_index.size())) {
    initialize_macro();
    place_macro_on_die_version2(DieSide::TOP, top_die_macros);
    place_macro_on_die_version2(DieSide::BOTTOM, bottom_die_macros);
  }

  // third version
  if (!check_macro_numbers(macro_C_index.size())) {
    initialize_macro();

    Btree btree;
    Btree_place_macro(btree, DieSide::TOP, top_die_macros,
                      case_.size.upper_right_x, case_.size.upper_right_y);

    Btree btree1;
    Btree_place_macro(btree1, DieSide::BOTTOM, bottom_die_macros,
                      case_.size.upper_right_x, case_.size.upper_right_y);
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
  if (!bottom_not_placed_cells.empty()) {
    place_cell_on_die(DieSide::TOP, bottom_not_placed_cells);
  }
  if (!top_not_placed_cells.empty()) {
    place_cell_on_die(DieSide::BOTTOM, top_not_placed_cells);
  }

  // terminal
  place_terminal();
}