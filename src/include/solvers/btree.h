#ifndef SRC_INCLUDE_SOLVERS_BTREE_H_
#define SRC_INCLUDE_SOLVERS_BTREE_H_

#include <string>
#include <vector>
#include <map>
#include<algorithm>
const int maxn = 205;
class Btree {
    public:
        int n, m, netnum;
        int root;
        long long best_area;
        long long tot_area;
        // int best_wire;
        long long limx, limy;
        long long Final_area;
        // long long Final_area, Final_wire;

        long long x[maxn] = {0}, rx[maxn] = {0}, y[maxn] = {0}, ry[maxn] = {0};
        long long b[maxn] = {0}, p[maxn] = {0};
        long long ls[maxn] = {0}, rs[maxn] = {0}, pa[maxn] = {0};
        long long bpa[maxn] = {0}, bls[maxn] = {0}, brs[maxn] = {0};
        long long bx[maxn] = {0}, brx[maxn] = {0}, by[maxn] = {0}, bry[maxn] = {0}, broot;
        long long fx[maxn] = {0}, frx[maxn] = {0}, fy[maxn] = {0}, fry[maxn] = {0};

        double T = 4000000000;
        const double r = 0.85;
        double best_cost;
        double Final_cost;
        double alpha=1;

        std::string block_name[maxn];

        std::pair<int, int> pr[maxn];
        std::pair<int, int> bpr[maxn];
        std::map<std::string, int> mp;
        std::vector<std::vector<int>>net;
        // functions
        void place(int now, int pre);
        void dfs(int now, int pre);
        //void dfs2(int now, int pre);
        //void print();
        long long getarea(long long &x, long long &y);
        long long getwire();
        // tree
        void init_tree();
        void remove(int x);
        void concat(int now, int pre);
        void swap1(int n1, int n2);
        void swap2(int n1, int n2);
        // algo
        bool upd();
        void SA();
        void update_final();
};

#endif