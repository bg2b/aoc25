// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <optional>
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace std;

// Sequential invalid IDs are like 123_123, 124_124, 125_125...  The
// step between them is uniform, and this function returns that step.
//   rep_size = number of digits in the repeated unit
//   num_digits = number of digits in the ID
// A null result means there's no such step (e.g., rep size 3 when
// there are 4 digits).
optional<long> rep_step(int rep_size, int num_digits) {
  int num_reps = num_digits / rep_size;
  if (rep_size * num_reps != num_digits || num_reps < 2)
    return nullopt;
  long result = 1;
  while (--num_reps > 0) {
    for (int _ = 0; _ < rep_size; ++_)
      result *= 10;
    ++result;
  }
  return result;
}

// Sum of the invalid IDs in [from, to], with validity determined by
// the part2 flag
long invalid(long from, long to, bool part2) {
  assert(from <= to);
  auto sfrom = to_string(from);
  if (sfrom.length() < to_string(to).length()) {
    // Split, since I don't want to deal with boundary crossing
    long split = stol(string(sfrom.length(), '9'));
    return invalid(from, split, part2) + invalid(split + 1, to, part2);
  }
  // There's overlap between invalid IDs if considering different
  // sizes, e.g., 111111 could be either 1_1_1_1_1_1 or 11_11_11 or
  // 111_111.  These could be handled with inclusion/exclusion, but I
  // think it's clearer to just collect everything and then uniquify.
  int max_rep_size = (sfrom.length() + 1) / 2;
  int min_rep_size = part2 ? 1 : max_rep_size;
  vector<long> invalid;
  for (int rep_size = min_rep_size; rep_size <= max_rep_size; ++rep_size)
    if (auto step = rep_step(rep_size, sfrom.length()); step.has_value()) {
      long first_invalid = ((from + *step - 1) / *step) * *step;
      assert(first_invalid >= from);
      for (long id = first_invalid; id <= to; id += *step)
        invalid.push_back(id);
    }
  // Filter duplicates and sum
  sort(invalid.begin(), invalid.end());
  invalid.erase(unique(invalid.begin(), invalid.end()), invalid.end());
  return accumulate(invalid.begin(), invalid.end(), long(0));
}

void solve(bool part2) {
  string line;
  getline(cin, line);
  stringstream ss(line + ',');
  long from, to;
  char _;
  long total_invalid = 0;
  while (ss >> from >> _ >> to >> _)
    total_invalid += invalid(from, to, part2);
  cout << total_invalid << '\n';
}

void part1() { solve(false); }
void part2() { solve(true); }

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  for (int i = 1; i < 10; ++i)
    rep_step(i, 9);
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
