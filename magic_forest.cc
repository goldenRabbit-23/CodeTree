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

const int DR[] = {-1, 0, 1, 0};
const int DC[] = {0, 1, 0, -1};

const int OFF = 3;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C, K;
    cin >> R >> C >> K;
    vector<vector<int>> forest(R + OFF, vector<int>(C, 0));  // 0: empty, g: golem #, g + 1000: exit of golem g

    auto reset = [&]() -> void {
        for (int r = 0; r < R + OFF; r++)
            for (int c = 0; c < C; c++)
                forest[r][c] = 0;
    };

    auto simulate = [&](int start_c, int start_dir) -> tuple<int, int, int> {
        int r = -2 + OFF, c = start_c, dir = start_dir;  // fairy position and exit dir

        while (true) {
            // arrived at the end
            if (r >= R + OFF - 2)
                break;

            // go south
            if (forest[r + 1][c - 1] == 0 &&
                  forest[r + 2][c] == 0 &&
                  forest[r + 1][c + 1] == 0) {
                r++;
                continue;
            }

            // turn left and go south
            if (c > 1 &&
                  forest[r - 1][c - 1] == 0 &&
                  forest[r][c - 2] == 0 &&
                  forest[r + 1][c - 1] == 0 &&
                  forest[r + 1][c - 2] == 0 &&
                  forest[r + 2][c - 1] == 0) {
                r++;
                c--;
                dir = (dir + 3) % 4;  // rotate counter-clockwise
                continue;
            }

            // turn right and go south
            if (c < C - 2 &&
                  forest[r - 1][c + 1] == 0 &&
                  forest[r][c + 2] == 0 &&
                  forest[r + 1][c + 1] == 0 &&
                  forest[r + 1][c + 2] == 0 &&
                  forest[r + 2][c + 1] == 0) {
                r++;
                c++;
                dir = (dir + 1) % 4;  // rotate clockwise
                continue;
            }

            break;
        }

        return {r, c, dir};
    };

    auto go_south = [&](int start_r, int start_c) -> int {
        vector<vector<bool>> visited(R, vector<bool>(C, false));
        queue<pair<int, int>> q;
        q.emplace(start_r, start_c);
        int max_row = start_r;

        while (!q.empty()) {
            auto [r, c] = q.front(); q.pop();
            visited[r][c] = true;
            max_row = max(max_row, r);

            for (int dir = 0; dir < 4; dir++) {
                int nr = r + DR[dir];
                int nc = c + DC[dir];

                // out of bounds
                if (nr < 0 || R <= nr || nc < 0 || C <= nc)
                    continue;

                // can't go outside golem
                if (forest[nr + OFF][nc] == 0)
                    continue;

                // already visited
                if (visited[nr][nc])
                    continue;

                // can't transfer to another golem if current position is not exit
                if (forest[r + OFF][c] <= 1000) {
                    int cur_golem = forest[r + OFF][c];
                    int neigh_golem = (forest[nr + OFF][nc] > 1000 ? forest[nr + OFF][nc] - 1000 : forest[nr + OFF][nc]);

                    if (cur_golem != neigh_golem)
                        continue;
                }

                q.emplace(nr, nc);
            }
        }

        return max_row + 1;
    };

    int ans = 0;

    for (int g = 1; g <= K; g++) {
        // d = [0: North, 1: East, 2: South, 3: West]
        int c, d;
        cin >> c >> d;
        c--;

        auto [end_r, end_c, end_dir] = simulate(c, d);

        if (end_r <= OFF) {
            reset();
            continue;
        }

        forest[end_r - 1][end_c]
          = forest[end_r][end_c + 1]
          = forest[end_r + 1][end_c]
          = forest[end_r][end_c - 1]
          = forest[end_r][end_c]
          = g;
        forest[end_r + DR[end_dir]][end_c + DC[end_dir]] = g + 1000;

        ans += go_south(end_r - OFF, end_c);
    }

    cout << ans << '\n';

    return 0;
}