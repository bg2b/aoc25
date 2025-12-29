// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <utility>
#include <set>
#include <cassert>

using namespace std;

struct printing_dept {
  vector<string> grid;

  // Construct from cin
  printing_dept();

  // Width, height, and access
  int w() const { return grid.front().length(); }
  int h() const { return grid.size(); }
  char at(int x, int y) const { return grid[y][x]; }
  char &at(int x, int y) { return grid[y][x]; }

  // Scan around (x, y) and call fn for any rolls
  void scan(int x, int y, function<void(int, int)> const &fn) const;
  // Is (x, y) a removable roll?
  bool is_removable(int x, int y) const;
  // Scan the whole grid and find all removable rolls
  set<pair<int, int>> all_removable() const;
  // Remove all possible rolls, return total removed
  int clean_up();
};

printing_dept::printing_dept() {
  string line;
  while (getline(cin, line)) {
    grid.push_back(line);
    assert(grid.back().length() == grid.front().length());
  }
}

void printing_dept::scan(int x, int y,
                         function<void(int, int)> const &fn) const {
  for (int dx = -1; dx <= +1; ++dx)
    if (x + dx >= 0 && x + dx < w())
      for (int dy = -1; dy <= +1; ++dy)
        if (y + dy >= 0 && y + dy < h())
          if ((dx != 0 || dy != 0) && at(x + dx, y + dy) == '@')
            fn(x + dx, y + dy);
}

bool printing_dept::is_removable(int x, int y) const {
  if (at(x, y) != '@')
    return false;
  int num_rolls = 0;
  scan(x, y, [&](int, int) { ++num_rolls; });
  return num_rolls < 4;
}

set<pair<int, int>> printing_dept::all_removable() const {
  set<pair<int, int>> result;
  for (int x = 0; x < w(); ++x)
    for (int y = 0; y < h(); ++y)
      if (is_removable(x, y))
        result.emplace(x, y);
  return result;
}

int printing_dept::clean_up() {
  int result = 0;
  for (auto to_remove = all_removable(); !to_remove.empty();) {
    auto [x, y] = *to_remove.begin();
    to_remove.erase(to_remove.begin());
    grid[y][x] = '.';
    ++result;
    scan(x, y, [&](int x1, int y1) {
      if (is_removable(x1, y1))
        to_remove.emplace(x1, y1);
    });
  }
  return result;
}

void part1() { cout << printing_dept().all_removable().size() << '\n'; }
void part2() { cout << printing_dept().clean_up() << '\n'; }

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
