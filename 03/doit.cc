// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <cassert>

using namespace std;

void solve(size_t num_bat) {
  string line;
  long long total_joltage = 0;
  while (getline(cin, line)) {
    assert(line.length() >= num_bat);
    // Highest combo found, with 0's indicating "unknown".  There's
    // one guard digit to simplify bounds checks.
    string joltage(num_bat + 1, '0');
    for (size_t i = 0; i < line.length(); ++i) {
      // Shift battery line[i] through the chain, but when nearing the
      // end be careful not to go too far.  The shifting leaves 0's
      // behind, indicating "unknown".
      size_t limit = num_bat - min(num_bat, line.length() - i);
      for (size_t j = num_bat; j-- > limit; )
        if (line[i] > joltage[j]) {
          joltage[j] = line[i];
          joltage[j + 1] = '0';
        }
    }
    // Drop guard digit
    joltage.pop_back();
    total_joltage += stoll(joltage);
  }
  cout << total_joltage << '\n';
}

void part1() { solve(2); }
void part2() { solve(12); }

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
