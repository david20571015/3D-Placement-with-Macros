#include "solver.h"

class Solver1 : public Solver {
  struct line_segment {
    int y;
    int from;
    int to;
  };

  std::vector<double> die_max_util;
  std::vector<double> die_util;
  std::vector<std::vector<line_segment>> horizontal_contours;
  std::vector<std::vector<std::vector<std::pair<int, int>>>> spared_rows;
  int die_size;

  enum DIE_INDEX { TOP, BOTTOM };

 public:
  explicit Solver1(Case& case_);
  ~Solver1() override = default;
  void solve() override;
  void separate_macros_cells(std::vector<std::string> macro_list,
                             std::vector<std::string>& macro_C_index,
                             std::vector<std::string>& cell_C_index);
  bool check_capacity(int index, int die_cell_index);
  void sort_macro(DIE_INDEX idx, std::vector<std::string>& macro_C_index);
  void decide_what_die(const std::vector<std::string>& inst_C_index,
                       std::vector<std::string>& top_die,
                       std::vector<std::string>& bottom_die);
  void place_macro_on_die(DIE_INDEX idx,
                          const std::vector<std::string>& macros);
  void place_macro(DIE_INDEX idx, int& x, int& y, int width, int height);
  void concat_line_segment(DIE_INDEX idx, int i);
  void sort_cell(DIE_INDEX idx, std::vector<std::string>& cell_C_index);
  void update_spared_rows(DIE_INDEX idx, int x, int y, int width, int height);
  void place_cell_on_die(DIE_INDEX idx, const std::vector<std::string>& cells);
  void place_cell(DIE_INDEX idx, int& x, int& y, int width, int height);

  void draw_macro();
};