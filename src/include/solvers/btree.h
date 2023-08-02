#ifndef SRC_INCLUDE_SOLVERS_BTREE_H_
#define SRC_INCLUDE_SOLVERS_BTREE_H_

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

const int maxn = 205;

class Btree {
 public:
  int n, m, netnum;
  int root;
  int64_t broot;
  int64_t best_area;
  int64_t tot_area;
  int64_t limx, limy;
  int64_t final_area;

  std::array<int64_t, maxn> x = {0}, rx = {0}, y = {0}, ry = {0};
  std::array<int64_t, maxn> b = {0}, p = {0};
  std::array<int64_t, maxn> ls = {0}, rs = {0}, pa = {0};
  std::array<int64_t, maxn> bpa = {0}, bls = {0}, brs = {0};
  std::array<int64_t, maxn> bx = {0}, brx = {0}, by = {0}, bry = {0};
  std::array<int64_t, maxn> fx = {0}, frx = {0}, fy = {0}, fry = {0};

  double T = 4000000000;
  const double r = 0.85;
  double best_cost;
  double final_cost;
  double alpha = 1;

  std::array<std::string, maxn> block_name;

  std::array<std::pair<int, int>, maxn> pr;
  std::array<std::pair<int, int>, maxn> bpr;
  std::map<std::string, int> mp;
  std::vector<std::vector<int>> net;

  void place(int now, int pre);
  void dfs(int now, int pre);

  int64_t getarea(int64_t &x, int64_t &y);
  // int64_t getwire();

  void init_tree();
  void remove(int x);
  void concat(int now, int pre);
  void swap1(int n1, int n2);
  void swap2(int n1, int n2);

  bool upd();
  void SA();
  void update_final();
};

#endif