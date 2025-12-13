// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cassert>

// Part 2 is still brute force, but I've done some preprocessing to
// organize the segments reasonably.  I've got vertical segments
// sorted by y and horizontal segments sorted by x.  Then I can
// quickly find the segments in a range of x's or y's, and since the
// orientation is known, the checks are simpler.
//
// The idea is still to look for a pair of points where all the
// corners of the corresponding rectangle are all inside the polygon
// or on the boundary.  For each such pair, see if any points are in
// the interior of the rectange or if any segments cut through the
// rectangle.  If not, then it's a valid rectangle.
//
// Compared to the original solution, this is about 10x faster
// unoptimized, and probably even more so optimized; "time doit1"
// doesn't have great resolution to really say for sure.

using namespace std;

using coord = pair<int, int>;

vector<coord> read() {
  vector<coord> red;
  int x, y;
  char comma;
  while (cin >> x >> comma >> y)
    red.emplace_back(x, y);
  // read() read red ;-)
  return red;
}

void part1(vector<coord> const &red) {
  long max_area = 0;
  for (size_t i = 0; i < red.size(); ++i) {
    auto [xi, yi] = red[i];
    for (auto j = i + 1; j < red.size(); ++j) {
      auto [xj, yj] = red[j];
      long dx = abs(xi - xj) + 1;
      long dy = abs(yi - yj) + 1;
      max_area = max(max_area, dx * dy);
    }
  }
  cout << max_area << '\n';
}

// Data for faster geometric predicates for part 2

using endpoints = pair<int, int>;

// Vertical segments (x, (y1, y2)) sorted by x
vector<pair<int, endpoints>> vertical;
// Horizontal segments (y, (x1, x2)) sorted by y
vector<pair<int, endpoints>> horizontal;

constexpr int infinity = 1000000;

void organize_segs(vector<coord> const &red) {
  // Collect all the segments
  for (size_t i = 0; i < red.size(); ++i) {
    auto [xi, yi] = red[i];
    auto [xi1, yi1] = red[(i + 1) % red.size()];
    assert(xi == xi1 || yi == yi1);
    if (xi == xi1)
      // Vertical
      vertical.push_back({xi, {min(yi, yi1), max(yi, yi1)}});
    else
      // Horizontal
      horizontal.push_back({yi, {min(xi, xi1), max(xi, xi1)}});
  }
  assert(!vertical.empty() && !horizontal.empty());
  // Order
  sort(vertical.begin(), vertical.end());
  sort(horizontal.begin(), horizontal.end());
  // Add guards at the end so I don't have to do bounds checking
  vertical.push_back({vertical.back().first + infinity, {0, 0}});
  horizontal.push_back({horizontal.back().first + infinity, {0, 0}});
}

// Find the [start, end) for scanning segments with coordinate x (the
// segments can be either horizontal or vertical, so sometimes x means
// y...)
inline pair<int, int> find(int x, vector<pair<int, endpoints>> const &segs) {
  auto l = lower_bound(segs.begin(), segs.end(), x,
                       [](pair<int, endpoints> const &seg, int x) {
                         return seg.first < x;
                       });
  auto u = upper_bound(segs.begin(), segs.end(), x,
                       [](int x, pair<int, endpoints> const &seg) {
                         return x < seg.first;
                       });
  return {l - segs.begin(), u - segs.begin()};
}

// Is c on the boundary of the polygon?
bool is_on(coord const &c) {
  auto [xbegin, xend] = find(c.first, vertical);
  for (auto i = xbegin; i < xend; ++i) {
    auto const &vi = vertical[i];
    assert(vi.first == c.first);
    if (vi.second.first <= c.second && c.second <= vi.second.second)
      return true;
  }
  auto [ybegin, yend] = find(c.second, horizontal);
  for (auto i = ybegin; i < yend; ++i) {
    auto const &hi = horizontal[i];
    assert(hi.first == c.second);
    if (hi.second.first <= c.first && c.first <= hi.second.second)
      return true;
  }
  return false;
}

// Is c inside the polygon or on the boundary?
bool is_inside_or_on(coord const &c) {
  if (is_on(c))
    return true;
  // Now I don't have to worry about the boundary; only strict cutting
  // counts
  auto xbegin = find(c.first + 1, vertical).first;
  auto xend = find(infinity - 1, vertical).second;
  int counts = 0;
  for (int i = xbegin; i < xend; ++i) {
    auto const &vi = vertical[i];
    assert(c.first < vi.first);
    if (vi.second.first < c.second && c.second < vi.second.second)
      ++counts;
  }
  return counts % 2 == 1;
}

// Is there anything strictly inside the bounding box (ll, ur)?  That
// is, is there a point or a part of a segment strictly inside?
bool anything_inside(coord const &ll, coord const &ur) {
  auto xbegin = find(ll.first + 1, vertical).first;
  auto xend = find(ur.first - 1, vertical).second;
  for (int i = xbegin; i < xend; ++i) {
    auto const &vi = vertical[i];
    assert(ll.first < vi.first && vi.first < ur.first);
    if (vi.second.second <= ll.second || ur.second <= vi.second.first)
      continue;
    // A vertical segment cuts through the box
    return true;
  }
  auto ybegin = find(ll.second + 1, horizontal).first;
  auto yend = find(ur.second - 1, horizontal).second;
  for (int i = ybegin; i < yend; ++i) {
    auto const &hi = horizontal[i];
    assert(ll.second < hi.first && hi.first < ur.second);
    if (hi.second.second <= ll.first || ur.first <= hi.second.first)
      continue;
    // A horizontal segment cuts through the box
    return true;
  }
  return false;
}

void part2(vector<coord> const &red) {
  organize_segs(red);
  long max_area = 0;
  for (size_t i = 0; i < red.size(); ++i)
    for (size_t j = i + 1; j < red.size(); ++j) {
      auto [xi, yi] = red[i];
      auto [xj, yj] = red[j];
      // Get bounding box
      if (xi > xj)
        swap(xi, xj);
      if (yi > yj)
        swap(yi, yj);
      coord ll{xi, yi};
      coord ur{xj, yj};
      long dx = ur.first - ll.first + 1;
      long dy = ur.second - ll.second + 1;
      long area = dx * dy;
      if (area <= max_area)
        // Can't be an improvement
        continue;
      if (!is_inside_or_on({xi, yj}) || !is_inside_or_on({xj, yi}))
        // One of the other corners is outside the polygon
        continue;
      // Check all points for inside and edges for intersection
      if (!anything_inside(ll, ur))
        max_area = area;
    }
  cout << max_area << '\n';
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  if (*argv[1] == '1')
    part1(read());
  else
    part2(read());
  return 0;
}
