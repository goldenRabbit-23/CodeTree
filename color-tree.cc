#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <vector>
using namespace std;

template<class Fun> class y_combinator_result {
    Fun fun_;
public:
    template<class T> explicit y_combinator_result(T &&fun): fun_(std::forward<T>(fun)) {}
    template<class ...Args> decltype(auto) operator()(Args &&...args) { return fun_(std::ref(*this), std::forward<Args>(args)...); }
};
template<class Fun> decltype(auto) y_combinator(Fun &&fun) { return y_combinator_result<std::decay_t<Fun>>(std::forward<Fun>(fun)); }

struct Node {
    int p_id;
    int color;
    int max_depth;
    int updated;
    vector<int> children;
};

const int COLORS = 5;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int Q;
    cin >> Q;
    map<int, Node> tree;
    vector<int> roots;
    int global_updated = 1;

    // 1. add node
    auto add_node = [&](int m_id, int p_id, int color, int max_depth) -> void {
        int cur_m_id = p_id;  // start checking from the lowest parent node
        int cur_depth = 2;

        while (cur_m_id != -1) {
            // cur_depth exceeds max_depth of any parent nodes
            if (tree[cur_m_id].max_depth < cur_depth)
                return;

            cur_m_id = tree[cur_m_id].p_id;
            cur_depth++;
        }

        tree[m_id] = {p_id, color, max_depth, global_updated, {}};
        global_updated++;  // keep track of global_updated variable

        if (p_id == -1) {
            roots.push_back(m_id);
        } else {
            tree[p_id].children.push_back(m_id);
        }
    };

    // 2. modify color
    auto modify_color = [&](int m_id, int color) -> void {
        tree[m_id].color = color;
        tree[m_id].updated = global_updated;  // update time for color change
        global_updated++;
    };

    // 3. lookup color
    auto lookup_color = [&](int m_id) -> int {
        int cur_m_id = m_id;
        int max_updated = 0, max_updated_color = 0;

        while (cur_m_id != -1) {
            if (max_updated < tree[cur_m_id].updated) {
                max_updated = tree[cur_m_id].updated;
                max_updated_color = tree[cur_m_id].color;
            }

            cur_m_id = tree[cur_m_id].p_id;
        }

        return max_updated_color;
    };

    // recursive function for calculating color counts and scores
    auto compute_score_node = y_combinator([&](auto self, int m_id, int max_color, int max_updated) -> pair<int, array<int, 6>> {
        int cur_updated = tree[m_id].updated;
        int cur_color = tree[m_id].color;

        // cur_updated > max_updated -> maintain current color & updated, pass current color & updated to next recursion
        // cur_updated < max_updated -> overwrite previous color & updated, pass previous color & updated to next recursion
        if (cur_updated > max_updated) {
            max_updated = cur_updated;
            max_color = cur_color;
        }

        pair<int, array<int, 6>> score_info;  // score_info ≜ (score, [color_cnt])
        score_info.second[max_color] = 1;  // initialize color_cnt array

        for (auto &child : tree[m_id].children) {
            auto &&sub_score_info = self(child, max_color, max_updated);  // recursively compute score_info for each children of current node
            score_info.first += sub_score_info.first;  // accumulate each child's score
            for (int clr = 1; clr <= COLORS; clr++)
                score_info.second[clr] += sub_score_info.second[clr];  // accumulate each color count
        }

        // count how many non-zero values are there in color_cnt
        int distinct = int(count_if(score_info.second.begin() + 1, score_info.second.end(), [](int x) { return x > 0; }));
        score_info.first += distinct * distinct;

        return score_info;
    });

    // 4. compute score
    auto compute_score = [&]() -> int {
        int score = 0;

        for (auto &root : roots)
            score += compute_score_node(root, 0, 0).first;

        return score;
    };

    for (int q = 0; q < Q; q++) {
        int com;
        cin >> com;

        if (com == 100) {
            int m_id, p_id, color, max_depth;
            cin >> m_id >> p_id >> color >> max_depth;
            add_node(m_id, p_id, color, max_depth);
        } else if (com == 200) {
            int m_id, color;
            cin >> m_id >> color;
            modify_color(m_id, color);
        } else if (com == 300) {
            int m_id;
            cin >> m_id;
            cout << lookup_color(m_id) << '\n';
        } else if (com == 400) {
            cout << compute_score() << '\n';
        }
    }

    return 0;
}