#include <string>
#include <vector>
#include <map>
#include<algorithm>
const int maxn = 205;
class Btree {
    public:
        int n, m, netnum;
        int root;
        int best_area;
        int tot_area;
        int best_wire;
        int limx, limy;
        int Final_area, Final_wire;

        int x[maxn], rx[maxn], y[maxn], ry[maxn];
        int b[maxn], p[maxn];
        int ls[maxn], rs[maxn], pa[maxn];
        int bpa[maxn], bls[maxn], brs[maxn], bx[maxn], brx[maxn], by[maxn], bry[maxn], broot;
        int fx[maxn], frx[maxn], fy[maxn], fry[maxn];

        const double T = 4000000000;
        const double r = 0.85;
        double best_cost;
        double Final_cost;
        double alpha;

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
        int getarea(int &x, int &y);
        int getwire();
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