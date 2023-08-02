#include <solvers/btree.h>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>

void Btree::place(int now, int pre) {
  if (pre == 0) {
    x[now] = 0;
    rx[now] = pr[now].first;
    y[now] = 0;
    ry[now] = pr[now].second;
    p[now] = b[now] = 0;
    return;
  }
  if (ls[pre] == now) {
    x[now] = rx[pre];
    rx[now] = x[now] + pr[now].first;
    if (p[pre] == 0) {
      y[now] = 0;
      ry[now] = y[now] + pr[now].second;
      p[pre] = now;
      b[now] = pre;
      p[now] = 0;
      return;
    }
    b[now] = pre;
    p[now] = p[pre];
    b[p[now]] = now;
    p[pre] = now;
  } else {
    x[now] = x[pre];
    rx[now] = x[now] + pr[now].first;
    if (b[pre] == 0) {
      b[now] = 0;
      p[now] = pre;
      b[pre] = now;
    } else {
      p[b[pre]] = now;
      b[now] = b[pre];
      p[now] = pre;
      b[pre] = now;
    }
  }
  int64_t mx = 0;
  int i;
  for (i = p[now]; i > 0; i = p[i]) {
    mx = std::max(mx, ry[i]);
    if (rx[i] >= rx[now]) {
      if (rx[i] == rx[now]) {
        p[now] = p[i];
        if (p[i] == 0) {
          b[p[i]] = now;
        }
      } else {
        p[now] = i;
        b[i] = now;
      }
      break;
    }
  }
  if (i == 0) {
    p[now] = 0;
  }
  y[now] = mx;
  ry[now] = y[now] + pr[now].second;
}
void Btree::dfs(int now, int pre) {
  if (now == 0) {
    return;
  }

  place(now, pre);
  dfs(ls[now], now);
  dfs(rs[now], now);
}

int64_t Btree::getarea(int64_t &x, int64_t &y) {
  for (int i = 1; i < n + 1; ++i) {
    x = std::max(x, rx[i]);
    y = std::max(y, ry[i]);
  }
  return x * y;
}
bool Btree::upd() {
  for (int i = 1; i <= n; ++i) {
    x[i] = rx[i] = y[i] = ry[i] = 0;
  }
  dfs(root, 0);
  int64_t X = 0;
  int64_t Y = 0;
  const int64_t area = getarea(X, Y);
  double ratio = static_cast<double>(X * limy) / static_cast<double>(Y * limx);
  if (ratio < 1) {
    ratio = 1 / ratio;
  }
  if (X > limx) {
    ratio *= 1.1;
  }
  if (Y > limy) {
    ratio *= 1.1;
  }
  if (X <= limx && Y <= limy) {
    ratio *= 0.8;
  }
  const double cost = area * ratio;
  const bool force =
      static_cast<double>(rand()) / RAND_MAX < exp(-1 * cost / T);
  T *= r;
  if (cost <= best_cost || force) {
    best_area = area;
    best_cost = cost;
    broot = root;
    int64_t mx = 0;
    int64_t my = 0;
    for (int i = 1; i < n + 1; ++i) {
      bpa[i] = pa[i];
      bls[i] = ls[i];
      brs[i] = rs[i];
      bx[i] = x[i];
      brx[i] = rx[i];
      by[i] = y[i];
      bry[i] = ry[i];
      bpr[i].first = pr[i].first;
      bpr[i].second = pr[i].second;
      mx = std::max(mx, rx[i]);
      my = std::max(my, ry[i]);
    }
    return true;
  }
  root = broot;
  for (int i = 1; i < n + 1; ++i) {
    pa[i] = bpa[i];
    ls[i] = bls[i];
    rs[i] = brs[i];
    x[i] = bx[i];
    rx[i] = brx[i];
    y[i] = by[i];
    ry[i] = bry[i];
    pr[i].first = bpr[i].first;
    pr[i].second = bpr[i].second;
  }
  return false;
}

void Btree::init_tree() {
  T = 4000000000;
  std::vector<int> v;
  for (int i = 1; i < n + 1; ++i) {
    v.push_back(i);
  }
  // random shuffle
  auto rng = std::default_random_engine{};
  std::shuffle(std::begin(v), std::end(v), rng);
  root = v[0];
  best_cost = best_area = 1e9;
  ls.fill(0);
  rs.fill(0);
  pa.fill(0);
  for (int i = 1; i <= n / 2; ++i) {
    if (i * 2 <= n) {
      ls[v[i - 1]] = v[i * 2 - 1];
      pa[v[i * 2 - 1]] = v[i - 1];
    }
    if (i * 2 + 1 <= n) {
      rs[v[i - 1]] = v[i * 2];
      pa[v[i * 2]] = v[i - 1];
    }
  }
}
void Btree::remove(int x) {
  int child = 0;
  int subchild = 0;
  int subparent = 0;

  if (ls[x] != 0 || rs[x] != 0) {
    bool left = rand() % 2;
    if (ls[x] == 0) {
      left = false;
    }
    if (rs[x] == 0) {
      left = true;
    }
    if (left) {
      child = ls[x];
      if (rs[x] != 0) {
        subchild = rs[child];
        subparent = rs[x];
        pa[rs[x]] = child;
        rs[child] = rs[x];
      }
    } else {
      child = rs[x];
      if (ls[x] != 0) {
        subchild = ls[child];
        subparent = ls[x];
        pa[ls[x]] = child;
        ls[child] = ls[x];
      }
    }
    pa[child] = pa[x];
  }

  if (pa[x] == 0) {
    root = child;
  } else {
    if (x == ls[pa[x]]) {
      ls[pa[x]] = child;
    } else {
      rs[pa[x]] = child;
    }
  }
  if (subchild != 0) {
    while (true) {
      if (ls[subparent] == 0 || rs[subparent] == 0) {
        pa[subchild] = subparent;
        if (ls[subparent] == 0) {
          ls[subparent] = subchild;
        } else {
          rs[subparent] = subchild;
        }
        break;
      }
      subparent = (rand() % 2 ? ls[subparent] : rs[subparent]);
    }
  }
}
void Btree::concat(int now, int pre) {
  if (rand() % 2) {
    pa[now] = pre;
    ls[now] = ls[pre];
    ls[pre] = now;
    rs[now] = 0;
    if (ls[now] != 0) {
      pa[ls[now]] = now;
    }
  } else {
    pa[now] = pre;
    rs[now] = rs[pre];
    rs[pre] = now;
    ls[now] = 0;
    if (rs[now] != 0) {
      pa[rs[now]] = now;
    }
  }
}
void Btree::swap1(int n1, int n2) {
  if (pa[n1] == n2) {
    std::swap(n1, n2);
  }
  const bool is_left = ls[n1] == n2;
  if (root == n1) {
    root = n2;
  } else if (root == n2) {
    root = n1;
  }
  if (is_left) {
    if (rs[n1] != 0) {
      pa[rs[n1]] = n2;
    }
    if (rs[n2] != 0) {
      pa[rs[n2]] = n1;
    }
    std::swap(rs[n1], rs[n2]);
    if (ls[n2] != 0) {
      pa[ls[n2]] = n1;
    }
    ls[n1] = ls[n2];
    ls[n2] = n1;

  } else {
    if (ls[n1] != 0) {
      pa[ls[n1]] = n2;
    }
    if (ls[n2] != 0) {
      pa[ls[n2]] = n1;
    }
    std::swap(ls[n1], ls[n2]);
    if (rs[n2] != 0) {
      pa[rs[n2]] = n1;
    }
    rs[n1] = rs[n2];
    rs[n2] = n1;
  }
  if (pa[n1] != 0) {
    const int p = pa[n1];

    ls[p] == n1 ? ls[p] = n2 : rs[p] = n2;
  }
  pa[n2] = pa[n1];
  pa[n1] = n2;
}
void Btree::swap2(int n1, int n2) {
  if (root == n1) {
    root = n2;
  } else if (root == n2) {
    root = n1;
  }
  if (ls[n1] != 0) {
    pa[ls[n1]] = n2;
  }
  if (rs[n1] != 0) {
    pa[rs[n1]] = n2;
  }
  if (ls[n2] != 0) {
    pa[ls[n2]] = n1;
  }
  if (rs[n2] != 0) {
    pa[rs[n2]] = n1;
  }
  if (pa[n1] != 0) {
    ls[pa[n1]] == n1 ? ls[pa[n1]] = n2 : rs[pa[n1]] = n2;
  }
  if (pa[n2] != 0) {
    ls[pa[n2]] == n2 ? ls[pa[n2]] = n1 : rs[pa[n2]] = n1;
  }
  std::swap(ls[n1], ls[n2]);
  std::swap(rs[n1], rs[n2]);
  std::swap(pa[n1], pa[n2]);
}

void Btree::SA() {
  upd();
  int iter = 100000;
  int fail = 0;
  while (iter--) {
    const int r = rand() % 100;
    if (r < 30) {
      const int idx = rand() % n + 1;
      std::swap(pr[idx].first, pr[idx].second);
    }
    if (r < 50) {
      const int n1 = rand() % n + 1;
      int n2 = rand() % n + 1;
      while (n2 == n1) {
        n2 = rand() % n + 1;
      }
      if (pa[n1] == n2 || pa[n2] == n1) {
        swap1(n1, n2);
      } else {
        swap2(n1, n2);
      }
    } else {
      const int n1 = rand() % n + 1;
      int n2 = rand() % n + 1;
      while (n2 == n1) {
        n2 = rand() % n + 1;
      }
      remove(n1);
      concat(n1, n2);
    }
    if (upd()) {
      fail++;
    } else {
      fail = 0;
    }
    if (fail == 50) {
      return;
    }
  }
}
void Btree::update_final() {
  int64_t X = 0;
  int64_t Y = 0;
  const int64_t best_area = getarea(X, Y);
  const int64_t cost = best_area * alpha;

  if (X <= limx && Y <= limy && cost < final_cost) {
    final_cost = cost;
    final_area = best_area;
    for (int i = 1; i <= n; ++i) {
      fx[i] = x[i];
      fy[i] = y[i];
      frx[i] = rx[i];
      fry[i] = ry[i];
    }
  }
}