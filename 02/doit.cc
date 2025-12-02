// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>

using namespace std;

// Simple brute-force approach

bool valid(string const &id, size_t max_reps) {
  max_reps = min(id.length(), max_reps);
  for (size_t reps = 2; reps <= max_reps; ++reps)
    if (id.length() % reps == 0) {
      auto rep = id.substr(0, id.length() / reps);
      auto repeated = true;
      for (size_t i = 1; repeated && i < reps; ++i)
        repeated = rep == id.substr(i * rep.length(), rep.length());
      if (repeated)
        return false;
    }
  return true;
}

void solve(int max_reps) {
  string line;
  getline(cin, line);
  stringstream ss(line + ',');
  long from, to;
  char _;
  long total_invalid = 0;
  while (ss >> from >> _ >> to >> _)
    for (auto id = from; id <= to; ++id)
      if (!valid(to_string(id), max_reps))
        total_invalid += id;
  cout << total_invalid << '\n';
}

void part1() { solve(2); }
void part2() { solve(999); }

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
