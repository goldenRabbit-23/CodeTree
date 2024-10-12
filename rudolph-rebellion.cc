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

struct Santa {
    int sr = 0, sc = 0;
    int score = 0;
    int faint = 0;
    bool out = false;
};

// N, NE, E, SE, S, SW, W, NW
const int DIRS = 8;
const int DR[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int DC[] = {0, 1, 1, 1, 0, -1, -1, -1};

const int INF = int(1e9) + 5;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, P, C, D, RR, RC;
    cin >> N >> M >> P >> C >> D >> RR >> RC;
    RR--; RC--;
    vector<vector<int>> grid(N, vector<int>(N, 0));
    grid[RR][RC] = -1;
    vector<Santa> santas(P + 1);

    for (int iter = 0; iter < P; iter++) {
        int n;
        cin >> n >> santas[n].sr >> santas[n].sc;
        santas[n].sr--; santas[n].sc--;
        grid[santas[n].sr][santas[n].sc] = n;
    }

    // bound check
    auto in_bound = [&](int r, int c) -> bool {
        return 0 <= r && r < N && 0 <= c && c < N;
    };

    // check if game is over
    auto game_over = [&]() -> bool {
        return all_of(santas.begin() + 1, santas.end(), [](Santa &s) { return s.out; });
    };

    // return index of nearest santa
    auto find_nearest_santa = [&]() -> int {
        int min_dist = INF;
        vector<int> min_dist_santas;

        for (int s = 1; s <= P; s++) {
            // ignore eliminated santa
            if (santas[s].out)
                continue;

            int dr = santas[s].sr - RR;
            int dc = santas[s].sc - RC;
            int dist = dr * dr + dc * dc;

            if (dist < min_dist) {
                min_dist = dist;
                min_dist_santas = {s};  // reset list with santa that updates min_dist
            } else if (dist == min_dist) {
                min_dist_santas.push_back(s);  // append list with santa that has same min_dist
            }
        }

        sort(min_dist_santas.begin(), min_dist_santas.end(), [&](int s1, int s2) -> bool {
            if (santas[s1].sr == santas[s2].sr)
                return santas[s1].sc > santas[s2].sc;
            return santas[s1].sr > santas[s2].sr;
        });

        return min_dist_santas[0];
    };

    // return direction in which rudolph will move
    auto find_rudolph_dir = [&](int s) -> int {
        int min_dist = INF;
        int min_dist_dir = -1;

        for (int d = 0; d < DIRS; d++) {
            int nrr = RR + DR[d];
            int nrc = RC + DC[d];

            // out of bounds
            if (!in_bound(nrr, nrc))
                continue;

            int dr = santas[s].sr - nrr;
            int dc = santas[s].sc - nrc;
            int dist = dr * dr + dc * dc;

            if (dist < min_dist) {
                min_dist = dist;
                min_dist_dir = d;
            }
        }

        return min_dist_dir;
    };

    // return direction in which santa will move
    auto find_santa_dir = [&](int sr, int sc) -> int {
        // current distance from rudolph
        int min_dist = (RR - sr) * (RR - sr) + (RC - sc) * (RC - sc);
        int min_dist_dir = -1;

        for (int d = 0; d < DIRS; d += 2) {
            int nsr = sr + DR[d];
            int nsc = sc + DC[d];

            // out of bounds OR someone else are already there
            if (!in_bound(nsr, nsc) || grid[nsr][nsc] > 0)
                continue;

            int dr = RR - nsr;
            int dc = RC - nsc;
            int dist = dr * dr + dc * dc;

            // updated distance should be less than current distance
            if (dist < min_dist) {
                min_dist = dist;
                min_dist_dir = d;
            }
        }

        return min_dist_dir;
    };

    // implementation of hit chain
    auto hit_chain = [&](int sr, int sc, int dir, int n) -> void {
        int curr = sr, curc = sc;

        while (true) {
            if (!in_bound(curr, curc) || grid[curr][curc] == 0 || grid[curr][curc] == n)
                return;

            Santa &next = santas[grid[curr][curc]];
            next.sr += DR[dir];
            next.sc += DC[dir];

            if (!in_bound(next.sr, next.sc))
                next.out = true;

            curr = next.sr;
            curc = next.sc;
        }
    };

    // implementation of rudolph move
    auto move_rudolph = [&]() -> void {
        vector<vector<int>> ngrid(N, vector<int>(N, 0));

        // rudolph moves to nearest santa
        int nearest_santa_idx = find_nearest_santa();
        int rudolph_dir = find_rudolph_dir(nearest_santa_idx);
        RR += DR[rudolph_dir];
        RC += DC[rudolph_dir];

        // rudolph hits santa
        if (grid[RR][RC] > 0) {
            Santa &ns = santas[grid[RR][RC]];
            ns.score += C;
            ns.sr += DR[rudolph_dir] * C;
            ns.sc += DC[rudolph_dir] * C;
            ns.faint = 2;

            // if santa goes out of bounds, he is eliminated
            if (!in_bound(ns.sr, ns.sc)) {
                ns.out = true;
            } else {  // otherwise, hit chain may occur
                hit_chain(ns.sr, ns.sc, rudolph_dir, grid[RR][RC]);
            }
        }

        // update new grid with new coordinates
        ngrid[RR][RC] = -1;
        for (int s = 1; s <= P; s++)
            if (!santas[s].out)
                ngrid[santas[s].sr][santas[s].sc] = s;

        grid.swap(ngrid);
    };

    auto move_santas = [&]() -> void {
        for (int s = 1; s <= P; s++) {
            if (santas[s].faint > 0 || santas[s].out)
                continue;

            // each santa moves to rudolph
            int santa_dir = find_santa_dir(santas[s].sr, santas[s].sc);

            // this santa can't go anywhere
            if (santa_dir == -1)
                continue;

            vector<vector<int>> ngrid(N, vector<int>(N, 0));

            Santa &cs = santas[s];
            cs.sr += DR[santa_dir];
            cs.sc += DC[santa_dir];

            // santa hits rudolph
            if (grid[cs.sr][cs.sc] == -1) {
                int rev_dir = (santa_dir + 4) % 8;
                cs.score += D;
                cs.sr += DR[rev_dir] * D;
                cs.sc += DC[rev_dir] * D;
                cs.faint = 2;

                // if santa goes out of bounds, he is eliminated
                if (!in_bound(cs.sr, cs.sc)) {
                    cs.out = true;
                } else {  // otherwise, hit chain may occur
                    hit_chain(cs.sr, cs.sc, rev_dir, s);
                }
            }

            // update new grid with new coordinates
            ngrid[RR][RC] = -1;
            for (int s = 1; s <= P; s++)
                if (!santas[s].out)
                    ngrid[santas[s].sr][santas[s].sc] = s;

            grid.swap(ngrid);
        }
    };

    for (int turn = 0; turn < M; turn++) {
        // finish game early if no one survived
        if (game_over())
            break;

        // all santas faint decrements by one
        for (int s = 1; s <= P; s++)
            if (!santas[s].out)
                santas[s].faint = max(santas[s].faint - 1, 0);

        // move rudolph -> move_santa
        move_rudolph();
        move_santas();

        // extra 1 point for survivors
        for (int s = 1; s <= P; s++)
            if (!santas[s].out)
                santas[s].score++;
    }

    for (int s = 1; s <= P; s++)
        cout << santas[s].score << ' ';

    return 0;
}