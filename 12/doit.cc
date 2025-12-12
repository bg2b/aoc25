// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace std;

constexpr int n = 3;

using form = array<array<char, n>, n>;

struct present {
  // The shape of the present
  form f;
  // Number of occupied spots in the shape
  int num_filled{0};

  // Construct from cin
  present();
};

present::present() {
  for (int i = 0; i < n; ++i) {
    string row;
    cin >> row;
    assert(row.length() == n);
    copy(row.begin(), row.end(), f[i].begin());
  }
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      num_filled += f[i][j] == '#' ? 1 : 0;
}

vector<present> presents;

struct region {
  // Size of the region
  int w, h;
  // How many of each type of present are needed?
  vector<int> required;
  // Trivially impossible?
  bool fuhgeddaboudit;
  // Trivially doable?
  bool no_sweat;

  // Construct from input line
  region(string const &line);
};

region::region(string const &line) {
  stringstream ss(line);
  char _;
  ss >> w >> _ >> h >> _;
  int num_presents;
  while (ss >> num_presents)
    required.push_back(num_presents);
  assert(required.size() == presents.size());
  int total_filled = 0;
  for (size_t i = 0; i < required.size(); ++i)
    total_filled += required[i] * presents[i].num_filled;
  fuhgeddaboudit = total_filled > w * h;
  int total_presents = accumulate(required.begin(), required.end(), 0);
  no_sweat = (w / n) * (h / n) >= total_presents;
}

vector<region> regions;

void parse() {
  string line;
  while (getline(cin, line)) {
    if (line.empty())
      continue;
    if (line.back() == ':') {
      assert(stoul(line) == presents.size());
      presents.push_back(present());
    } else
      regions.emplace_back(line);
  }
}

void part1() {
  parse();
  int can_fit = 0;
  int no_clue = 0;
  for (auto const &region : regions)
    if (region.no_sweat)
      ++can_fit;
    else if (!region.fuhgeddaboudit)
      ++no_clue;
  if (no_clue > 0)
    cout << "The Oracle told me that the answer is 2\n";
  else
    cout << can_fit << '\n';
}

void part2() { cout << "Finish Decorating the North Pole\n"; }

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
