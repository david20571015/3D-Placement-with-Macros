#include<solvers/btree.h>
#include <algorithm> 
#include <cstdlib>
#include <random>
#include <limits.h>
#include <cstring>
#include <iostream>

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
        else {
            b[now] = pre;
            p[now] = p[pre];
            b[p[now]] = now;
            p[pre] = now;
        }
    }
    else {
        x[now] = x[pre];
        rx[now] = x[now] + pr[now].first;
        if (b[pre] == 0) {
            b[now] = 0;
            p[now] = pre;
            b[pre] = now;
        }
        else {
            p[b[pre]] = now;
            b[now] = b[pre];
            p[now] = pre;
            b[pre] = now;
        }
    }
    long long mx = 0;
    int i;
    for (i = p[now] ; i > 0 ; i = p[i]) {
        mx = std::max(mx, ry[i]);
        if (rx[i] >= rx[now]) {
            if (rx[i] == rx[now]) {
                p[now] = p[i];
                if (p[i] == 0) {
                    b[p[i]] = now;
                }
            }
            else {
                p[now] = i;
                b[i] = now;
            }
            break;
        }
    }
    //cout << now << endl;
    if (i == 0) {
        p[now] = 0;
    }
    y[now] = mx;
    ry[now] = y[now] + pr[now].second;
}
void Btree::dfs(int now, int pre) {
    if (!now) return;
    //cout << now <<' '<<pre << endl;
    place(now, pre);
    dfs(ls[now], now);
    dfs(rs[now], now);
}
//void dfs2(int now, int pre) {
//    if (!now) return;
//    cout << now <<' '<<ls[now]<<' '<<rs[now]<<endl;
//    dfs2(ls[now], now);
//    dfs2(rs[now], now);
//}
long long Btree::getarea(long long &x, long long &y) {
    for(int i = 1; i < n+1; ++i) {
        x = std::max(x, rx[i]);
        y = std::max(y, ry[i]);
    }
    return x * y;
}
// long long Btree::getwire() {
//     long long ret = 0;
//     for (auto &i : net) {
//         int mix = INT_MAX, mxx = INT_MIN, miy = INT_MAX, mxy = INT_MIN;
//         for (auto &j : i) {
//             int xx = x[j] + (rx[j] - x[j]) / 2;
//             int yy = y[j] + (ry[j] - y[j]) / 2;
//             mix = std::min(mix, xx);
//             mxx = std::max(mxx, xx);
//             miy = std::min(miy, yy);
//             mxy = std::max(mxy, yy);
//         }
//         ret += mxx - mix + mxy - miy;
//     }
//     return ret;
// }
bool Btree::upd() {
    for (int i = 1 ; i <= n ; ++i) {
        x[i] = rx[i] = y[i] = ry[i] = 0;
    }
    dfs(root, 0);
    //go();
    long long X = 0, Y = 0;
    long long Area = getarea(X, Y);
    // int Wire = getwire();
    //cout << Area << ' ' << Wire << endl;
    double Ratio = static_cast<double>(X * limy) / static_cast<double>(Y * limx);
    if (Ratio < 1) {
        Ratio = 1 / Ratio;
    }
    if (X > limx) {
        Ratio *= 1.1;
    }
    if (Y > limy) {
        Ratio *= 1.1;
    }
    if (X <= limx && Y <= limy) {
        Ratio *= 0.8;
    }
    double Cost = ((double)Area * Ratio);
    bool force = (double)rand() / RAND_MAX < exp(-1 * (double)Cost / T);
    T *= r;
    if (Cost <= best_cost || force) {
        best_area = Area;
        best_cost = Cost;
        // best_wire = Wire;
        broot = root;
        long long mx = 0, my = 0;
        for(int i = 1; i < n+1; ++i) {
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
        return 1;
    }
    else {
        root = broot;
        for(int i = 1; i < n+1; ++i) {
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
        return 0;
    }
    
}
//void print() {
//    dfs2(root, 0);
//}
void Btree::init_tree() {
    T = 4000000000;
    std::vector<int>v;
    for(int i = 1; i < n+1; ++i) {
        v.push_back(i);
    }
    //random shuffle
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(v), std::end(v), rng);
    // std::random_shuffle(v.begin(), v.end());
    root = v[0];
    // best_cost = best_area = best_wire = 1e9;
    best_cost = best_area = 1e9;
    memset(ls, 0, sizeof(ls));
    memset(rs, 0, sizeof(rs));
    memset(pa, 0, sizeof(pa));
    for (int i = 1 ; i <= n / 2; ++i) {
        if (i * 2 <= n) {
            ls[v[i - 1]] = v[i*2 - 1];
            pa[v[i*2 - 1]] = v[i - 1];
        }
        if (i * 2 + 1 <= n) {
            rs[v[i - 1]] = v[i*2];
            pa[v[i*2]] = v[i - 1];
        }
    }
}
void Btree::remove(int x) {
    int child    = 0;  
    int subchild = 0;   
    int subparent= 0;

    if(ls[x]||rs[x]){
        bool left = rand() % 2;
        if(ls[x] == 0) left = 0;
        if(rs[x] == 0) left = 1;
        if(left){
            child = ls[x];         
            if(rs[x] != 0){
                subchild  = rs[child];
                subparent = rs[x];
                pa[rs[x]] = child; 
                rs[child] = rs[x];
            }
        }
        else{
            child = rs[x];
            if(ls[x] != 0){
                subchild  = ls[child];
                subparent = ls[x];
                pa[ls[x]] = child;
                ls[child] = ls[x];
            }
        }
        pa[child] = pa[x];
    }
  
    if(pa[x] == 0){
        root = child;
    }else{
        if(x == ls[pa[x]])
            ls[pa[x]] = child;
        else
            rs[pa[x]] = child;
    }
    if(subchild != 0){
        while(1){
            // std::cout << "a" << std::endl;
            if(ls[subparent] == 0 || rs[subparent] == 0){
                pa[subchild] = subparent;
                if(ls[subparent]==0) ls[subparent] = subchild;
                else rs[subparent] = subchild;
                break;
            } else{
                subparent = (rand() % 2 ? ls[subparent] : rs[subparent]);
            }
        }
    }
}
void Btree::concat(int now, int pre) {
    if (rand() % 2) {
        pa[now] = pre;
        ls[now] = ls[pre];
        ls[pre] = now;
        rs[now] = 0;
        if (ls[now]) {
            pa[ls[now]] = now;
        }
    }
    else {
        pa[now] = pre;
        rs[now] = rs[pre];
        rs[pre] = now;
        ls[now] = 0;
        if (rs[now]) {
            pa[rs[now]] = now;
        }
    }
}
void Btree::swap1(int n1, int n2) {
    if (pa[n1] == n2) {
        std::swap(n1, n2);
    }
    bool is_left = ls[n1] == n2;
    if (root == n1 )
        root = n2;
    else if ( root == n2 )
        root = n1;
    if (is_left) {
        if (rs[n1] != 0) 
            pa[rs[n1]] = n2;
        if (rs[n2] != 0) 
            pa[rs[n2]] = n1;
        std::swap(rs[n1], rs[n2]);
        if (ls[n2] != 0) 
            pa[ls[n2]] = n1;
        ls[n1] = ls[n2];
        ls[n2] = n1;

    } else {
        if (ls[n1] != 0) 
             pa[ls[n1]] = n2;
        if (ls[n2] != 0) 
             pa[ls[n2]] = n1;
        std::swap(ls[n1], ls[n2]);
        if (rs[n2] != 0) 
            pa[rs[n2]] = n1;
        rs[n1] = rs[n2];
        rs[n2] = n1;
    }
    if (pa[n1] != 0) {
        int p = pa[n1];
        if(ls[p] == n1) 
            ls[p] = n2;
        else    
            rs[p] = n2;
    }
    pa[n2] = pa[n1];
    pa[n1] = n2;
}
void Btree::swap2(int n1, int n2) {
    if (root == n1) {
        root = n2;
    }
    else if (root == n2) {
        root = n1;
    }
    if (ls[n1]) {
        pa[ls[n1]] = n2;
    }
    if (rs[n1]) {
        pa[rs[n1]] = n2;
    }
    if (ls[n2]) {
        pa[ls[n2]] = n1;
    }
    if (rs[n2]) {
        pa[rs[n2]] = n1;
    }
    if (pa[n1]) {
        if (ls[pa[n1]] == n1) {
            ls[pa[n1]] = n2;
        }
        else {
            rs[pa[n1]] = n2;
        }
    }
    if (pa[n2]) {
        if (ls[pa[n2]] == n2) {
            ls[pa[n2]] = n1;
        }
        else {
            rs[pa[n2]] = n1;
        }
    }
    std::swap(ls[n1], ls[n2]);
    std::swap(rs[n1], rs[n2]);
    std::swap(pa[n1], pa[n2]); 
}

void Btree::SA() {
    upd();
    int iter = 100000;
    int fail = 0;
    while(iter--) {
        int r = rand() % 100;
        if (r < 30) {
            int idx = rand() % n + 1;
            std::swap(pr[idx].first, pr[idx].second);        
        }
        if (r < 50) {
            int n1 = rand() % n + 1;
            int n2 = rand() % n + 1;
            while (n2 == n1) {
                n2 = rand() % n + 1;
            }
            if (pa[n1] == n2 || pa[n2] == n1) {
                swap1(n1, n2);
            }
            else {
                swap2(n1, n2);
            }
        }
        else {
            int n1 = rand() % n + 1;
            int n2 = rand() % n + 1;
            while (n2 == n1) {
                n2 = rand() % n + 1;
            }
            remove(n1);
            concat(n1, n2);
        }
        if(upd()) {
            fail++;
        }
        else {
            fail = 0;
        }
        if (fail == 50) {
            return;
        }
    }
}
void Btree::update_final() {
    long long X = 0, Y = 0;
    long long best_area = getarea(X, Y);
    // int best_wire = getwire();
    // long long Cost = (best_area) * alpha + static_cast<long long>(best_wire) * (1.0 - alpha);
    long long Cost = (best_area) * alpha;

    if (X <= limx && Y <= limy && Cost < Final_cost) {
        Final_cost = Cost;
        // Final_wire = best_wire;
        Final_area = best_area;
        for (int i = 1 ; i <= n ; ++i) {
            fx[i] = x[i];
            fy[i] = y[i];
            frx[i] = rx[i];
            fry[i] = ry[i];
        }
    }
}