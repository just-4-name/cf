#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::cin;
using std::cout;
using std::pair;
using std::set;
using std::vector;

#define int long long
#define vi vector<int>
#define vvi vector<vector<int>>
#define omk cout << "YES\n"
#define nah cout << "NO\n"
#define ssort(v) std::sort(v.begin(), v.end())
#define rsort(v) std::sort(v.begin(), v.end(), std::greater<int>())
#define in(a)                                                                  \
  for (auto &i : a)                                                            \
  cin >> i
#define out(a)                                                                 \
  for (auto i : a)                                                             \
    cout << i << ' ';                                                          \
  cout << '\n'
#define pii std::pair<int, int>
#define NL cout << '\n'

void OptimizeIO() {
  std::ios_base::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
}

int Getk(int n, int pos) { return (bool((1 << pos) & n)); }

void Solve() {
  int n, m;
  cin >> n >> m;
  vector<pair<int, pii>> a(m); // r,l,sum
  for (int i = 0; i < m; ++i) {
    int l, r, s;
    cin >> l >> r >> s;
    a[i] = {r - 1, {l - 1, s}};
  }
  int M = 10;
  ssort(a);
  int curr = 0;
  int next = -1;
  vector<vector<int>> dp(n, vector<int>((1 << M), 1));
  for (int i = 0; i < (1 << M); ++i) {
    vector<int> cur(10);
    for (int k = 0; k < 10; ++k) cur[k] = Getk(i, k);
    int ind = curr;
    vector<int> pref(10);
    pref[0] = cur[0];
    for (int i = 1; i < 10; ++i) pref[i] = pref[i- 1]+cur[i];
    while (ind < m) {
      int r = a[ind].first, l  = a[ind].second.first, s = a[ind].second.second;
      if (r >= 10) {
        next = r;
        break;
      }
      int ss = pref[r];
      if (l > 0) ss -= pref[l - 1];
      if (s != ss) {
        dp[9][i] = 0;
      }
    }
    for (int pos = 9;)

    vector<int> cur =
  }
}

int32_t main() {
  OptimizeIO();

  int t = 1;
  // cin >> t;
  while (t--)
    Solve();
}