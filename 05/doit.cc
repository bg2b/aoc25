// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cassert>

using namespace std;

using num = unsigned long;

// Inventory Management System
struct IMS {
  // Ranges are sorted lexicographically in the constructor
  vector<pair<num, num>> ranges;

  // Construct from cin
  IMS();

  // Is the ingredient with the given ID fresh?
  bool is_fresh(num id) const;

  // Total number of fresh ingredients
  num num_fresh() const;
};

IMS::IMS() {
  string line;
  while (getline(cin, line) && !line.empty()) {
    num from, to;
    char dash;
    stringstream(line) >> from >> dash >> to;
    assert(dash == '-' && from <= to);
    ranges.emplace_back(from, to);
  }
  sort(ranges.begin(), ranges.end());
}

bool IMS::is_fresh(num id) const {
  for (auto [from, to] : ranges)
    if (from <= id && id <= to)
      return true;
  return false;
}

num IMS::num_fresh() const {
  if (ranges.empty())
    return 0;
  // The number of fresh ingredients found so far
  num so_far = 0;
  // The range that's being accumulated
  auto current = ranges.front();
  for (auto [from, to] : ranges)
    if (current.second < from) {
      // No overlap, accumulate more fresh items
      so_far += current.second - current.first + 1;
      // Start a new range
      current = {from, to};
    } else
      current.second = max(current.second, to);
  return so_far + current.second - current.first + 1;
}

void part1() {
  IMS ims;
  int result = 0;
  num id;
  while (cin >> id)
    if (ims.is_fresh(id))
      ++result;
  cout << result << '\n';
}

void part2() { cout << IMS().num_fresh() << '\n'; }

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
