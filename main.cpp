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
#include <iomanip>

using std::cin;
using std::cout;
using std::pair;
using std::set;
using std::vector;
using std::map;
using std::string;

#define int long long
#define ld long double
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



static const int MOD = 998244353;

void OptimizeIO() {
  std::ios_base::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
}



void Solve() {

}

int32_t main() {
  OptimizeIO();
  int t = 1;
  //cin >> t;
  while (t--)
    Solve();
}