// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <cassert>

using namespace std;

// This is practically begging for off-by-one errors, so be careful
void solve(bool CLIK) {
  int pointing_at = 50;
  int num_zeros = 0;
  char dir;
  int amount;
  while (cin >> dir >> amount) {
    assert(dir == 'L' || dir == 'R');
    while (amount > 0) {
      if (pointing_at == 0 && dir == 'L')
        // Wrap low
        pointing_at = 100;
      // Turn to next interesting point
      int steps = min(amount, dir == 'L' ? pointing_at : 100 - pointing_at);
      pointing_at += (dir == 'L' ? -1 : +1) * steps;
      amount -= steps;
      if (pointing_at == 100)
        // Wrap high
        pointing_at = 0;
      if (CLIK && pointing_at == 0)
        // Intermediate clicks count
        ++num_zeros;
    }
    if (!CLIK && pointing_at == 0)
      // Only the final click counts
      ++num_zeros;
  }
  cout << num_zeros << '\n';
}

void part1() { solve(false); }
void part2() { solve(true); }

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
