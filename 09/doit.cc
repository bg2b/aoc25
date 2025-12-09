// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2
// Optimization would help with part 2 (it's 2+ seconds without)

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>

// Part 2 is brute force, but it does get the right answer in not
// toooo much time...
//
// The idea is to look for a pair of points where all the corners of
// the corresponding rectangle are all inside the polygon or on the
// boundary.  For each such pair, see if any points are in the
// interior of the rectange or if any segments cut through the
// rectangle.  If not, then it's a valid rectangle.
//
// Ideas to do better...
// 1. Keep the same approach and do some basic stuff to beat on the
//    constant, like make separate lists of vertical and horizontal
//    segments sorted by x and y.  Basically just make the primitives
//    faster in obvious ways.
// 2. Toss on a quadtree to partition points and edges spatially, and
//    use that to prune away irrelevant bits based on the rectangle.
// 3. Do a sweep initially to make a representation of the in/out
//    parts of the polygon, and save the sweepline state at each
//    transition to check a given rectangle.
//
// Based on the particular form of the input, one cheap trick is to
// rotate so that one of the two big nearly-bisecting edges is at the
// start.  That cuts it to about a second with no other optimization
// since the check for edge intersection immediately prunes most
// rectangles that are on either side of that dividing line.

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

// Is point a strictly inside a bounding box?
bool inside(coord const &ll, coord const &ur, coord const &a) {
  return (ll.first < a.first && a.first < ur.first &&
          ll.second < a.second && a.second < ur.second);
}

// Do two segments cross in their interior?
bool cross(coord a, coord b, coord c, coord d) {
  if ((a.second == b.second) == (c.second == d.second))
    // If orientations are the same, don't cross
    return false;
  if (a.second != b.second) {
    // Want first segment horizontal
    swap(a, c);
    swap(b, d);
  }
  // Order segments
  if (a.first > b.first)
    swap(a, b);
  if (c.second > d.second)
    swap(c, d);
  // Cross iff strict containment in both directions
  return (a.first < c.first && c.first < b.first &&
          c.second < a.second && a.second < d.second);
}

// Does segment a-b cut through a bounding box?
bool cuts(coord const &ll, coord const &ur, coord const &a, coord const &b) {
  coord lu{ur.first, ll.second};
  coord ul{ll.first, ur.second};
  return (cross(ll, lu, a, b) || cross(lu, ur, a, b) ||
          cross(ur, ul, a, b) || cross(ul, ll, a, b));
}

// Is point c on the segment a-b (including endpoints)?
bool is_on(coord const &c, coord const &a, coord const &b) {
  if (a.first == b.first)
    return c.first == a.first &&
      min(a.second, b.second) <= c.second && c.second <= max(a.second, b.second);
  else
    return c.second == a.second &&
      min(a.first, b.first) <= c.first && c.first <= max(a.first, b.first);
}

// Is point c inside or on the boundary of the polygon?
bool is_inside_or_on(coord const &c, vector<coord> const &red) {
  // Point infinity away horizontally
  coord cinf{1000000, c.second};
  int counts = 0;
  for (size_t i = 0; i < red.size(); ++i) {
    coord const &a = red[i];
    coord const &b = red[(i + 1) % red.size()];
    if (is_on(c, a, b))
      return true;
    if (cross(c, cinf, a, b))
      ++counts;
  }
  return counts % 2 == 1;
}

void part2(vector<coord> const &red) {
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
      if (!is_inside_or_on({xi, yj}, red) || !is_inside_or_on({xj, yi}, red))
        // One of the other corners is outside the polygon
        continue;
      // Check all points for inside and edges for intersection
      bool valid = true;
      for (size_t k = 0; valid && k < red.size(); ++k)
        if (inside(ll, ur, red[k]) ||
            cuts(ll, ur, red[k], red[(k + 1) % red.size()]))
          valid = false;
      if (valid)
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
