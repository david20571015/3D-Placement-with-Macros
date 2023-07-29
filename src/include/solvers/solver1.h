#include <utility>
#include "btree.h"
#include "solver.h"

class Solver1 : public Solver {
  struct LineSegment {
    int y;
    int from;
    int to;
  };
  
  Btree btree;
  std::vector<double> die_max_util;
  std::vector<double> die_util;
  std::vector<std::vector<LineSegment>> horizontal_contours;
  std::vector<std::vector<std::vector<std::pair<int, int>>>> spared_rows;
  long long die_size;

  using DieSide = Case::DieSide;

 public:
  explicit Solver1(Case& case_) : Solver1(std::move(case_)){};
  explicit Solver1(Case&& case_);
  ~Solver1() override = default;
  void solve() override;
  void separate_macros_cells(std::vector<std::string> macro_list,
                             std::vector<std::string>& macro_C_index,
                             std::vector<std::string>& cell_C_index);
  bool check_capacity(DieSide side, int die_cell_index);
  void sort_macro(DieSide side, std::vector<std::string>& macro_C_index);
  void decide_what_die(const std::vector<std::string>& inst_C_index,
                       std::vector<std::string>& top_die,
                       std::vector<std::string>& bottom_die);
  void decide_what_die_cell(const std::vector<std::string>& inst_C_index,
                            std::vector<std::string>& top_die,
                            std::vector<std::string>& bottom_die);
  void place_macro_on_die(DieSide side, const std::vector<std::string>& macros);
  bool place_macro(DieSide side, int& x, int& y, int width, int height);
  void concat_line_segment(DieSide side, size_t i);
  void sort_cell(DieSide side, std::vector<std::string>& cell_C_index);
  void update_spared_rows(DieSide side, int x, size_t y, int width, int height);
  void place_cell_on_die(DieSide side, const std::vector<std::string>& cells);
  bool place_cell(DieSide side, int& x, int& y, int width, int height);
  void get_inst_that_not_placed(DieSide side,
                                const std::vector<std::string>& inst_C_index,
                                std::vector<std::string>& not_placed);
  void place_terminal();
  void add_terminal(std::string net_name, int terminal_index);

  void draw_macro();
  void draw_terminal();

  bool check_macro_numbers(size_t macro_numbers);

  void Btree_place_macro(Btree &btree, DieSide side, std::vector<std::string> top_die_macros, int die_width, int die_height);
  bool check_cell_numbers(std::vector<std::string>& cell_C_index);

  void initialize_macro();
  void place_macro_on_die_version2(DieSide side, const std::vector<std::string>& macros);
};