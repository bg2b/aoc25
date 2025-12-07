// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

pair<int, long> solve() {
  // postion => number of timelines with a beam at that position
  map<int, long> beams;
  string line;
  getline(cin, line);
  // A single timeline with a beam at the start position
  beams.emplace(line.find('S'), 1);
  int num_splits = 0;
  while (getline(cin, line)) {
    assert(line.front() != '^' && line.back() != '^');
    map<int, long> next_beams;
    for (auto [beam, num_timelines] : beams)
      if (line[beam] == '^') {
        // Hit a splitter: go both ways while combining timelines
        ++num_splits;
        next_beams[beam - 1] += num_timelines;
        next_beams[beam + 1] += num_timelines;
      } else
        // Beams pass straight through
        next_beams[beam] += num_timelines;
    beams = next_beams;
  }
  // Add up total timelines
  long num_timelines = 0;
  for (auto [_, nt] : beams)
    num_timelines += nt;
  return {num_splits, num_timelines};
}

void part1() { cout << solve().first << '\n'; }
void part2() { cout << solve().second << '\n'; }

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
