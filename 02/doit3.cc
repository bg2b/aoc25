// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit3 doit3.cc
// ./doit3 1 < input  # part 1
// ./doit3 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <optional>
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

// As a base case, given an initial invalid ID, the step, and the
// range, the invalid IDs are a finite arithmetic series and can be
// summed analytically.  The remaining issue is how to handle overlaps
// between series with different repetition sizes.
//
// First consider a situation like 8 digits.  Possible repetition
// sizes are the divisors 1, 2, and 4, and the corresponding number of
// repeated units is 8, 4, and 2.  The cases with 8 units and with 4
// units can also be viewed as repetitions with 2 units (e.g.,
// 12_12_12_12 can also be counted as 1212_1212).  So when counting, I
// can just do the 2-unit case and ignore the others.  This applies to
// situations when the number of repeated units has a prime factor
// that appears more than once.
//
// Now consider 6 digits.  Repetition sizes are 1, 2, and 3, giving
// number of repeated units 6, 3, and 2.  An ID like 111111 will be
// counted twice when considering 2-unit (111_111) and 3-unit
// (11_11_11) repetitions.  So if I sum the results from 2-unit and
// 3-unit, I'll be a bit too high.  The duplicate here is a 6-unit
// repetition (1_1_1_1_1_1), so I can correct by subtracting the
// 6-unit case.
//
// If the numbers were long enough, say 2*3*5 = 30 digits, then I
// could sum the invalid IDs by doing 2-unit + 3-unit + 5-unit.  Then
// 2-unit and 3-unit overlaps are corrected by subtracting 6-unit, and
// similarly 10-unit and 15-unit cases should be subtracted.  But then
// the 30-unit case has been subtracted entirely, so it would have to
// be added back.  The final result would be:
// 2 + 3 + 5 - 6 - 10 - 15 + 30

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
  int max_rep_size = (sfrom.length() + 1) / 2;
  int min_rep_size = part2 ? 1 : max_rep_size;
  // Signs for various numbers of repetitions, as above.
  // num reps =  0, 1,  2,  3, 4,  5,  6,  7, 8, 9, 10
  int signs[] = {0, 0, +1, +1, 0, +1, -1, +1, 0, 0, -1};
  assert(sfrom.length() / min_rep_size < size(signs));
  long result = 0;
  for (int rep_size = min_rep_size; rep_size <= max_rep_size; ++rep_size)
    if (int sgn = signs[sfrom.length() / rep_size]; sgn != 0)
      if (auto step = rep_step(rep_size, sfrom.length()); step.has_value()) {
        long first_invalid = ((from + *step - 1) / *step) * *step;
        long last_invalid = (to / *step) * *step;
        if (first_invalid <= last_invalid) {
          assert(from <= first_invalid && last_invalid <= to);
          assert(first_invalid - *step < from);
          assert(last_invalid + *step > to);
          // Want the sum of [first_invalid, last_invalid] stepping by
          // *step.  If n is the number of terms, then that's (the sum
          // of 0 to n - 1) times *step, plus n time the initial term.
          long n = (last_invalid - first_invalid) / *step + 1;
          long sum = (n * (n - 1)) / 2 * *step + n * first_invalid;
          result += sgn * sum;
        }
      }
  return result;
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
