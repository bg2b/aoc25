// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2
// Optimization helps with part 2 (about 3.5 seconds without)

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <bit>
#include <algorithm>
#include <cmath>
#include <cassert>

using namespace std;

struct machine {
  // Target light configuration, lsb = leftmost light in the input
  int indicator{0};
  // Target joltages
  vector<int> joltages;
  // Buttons, xor with the current light configuration for a press, or
  // add one to the joltages corresponding to the set bits
  vector<int> buttons;

  machine(string const &line);

  // Return the minimum number of button pushes required to configure
  // the lights and the joltages
  int lights() const;
  int power() const;
};

machine::machine(string const &line) {
  stringstream ss(line);
  auto snarf = [&]() { string s; ss >> s; assert(s.length() > 2); return s; };
  auto target = snarf();
  for (size_t i = 1; i + 1 < target.length(); ++i)
    if (target[i] == '#')
      indicator |= 1 << (i - 1);
  while (true) {
    stringstream ss(snarf());
    char type;
    ss >> type;
    assert(type == '(' || type == '{');
    vector<int> vals;
    int v;
    char _;
    while (ss >> v >> _)
      vals.push_back(v);
    if (type == '{') {
      joltages = vals;
      break;
    }
    // Convert to bits and add to list of buttons
    buttons.push_back(0);
    for (auto i : vals)
      buttons.back() |= 1 << i;
  }
}

int machine::lights() const {
  // Only need to press each button at most once, and order is
  // irrelevant, so just scan all the subsets of presses.  The number
  // of lights isn't large enough to warrant trying to go in order of
  // subset size.
  int num_combos = 1 << buttons.size();
  optional<int> min_presses;
  for (int combo = 0; combo < num_combos; ++combo) {
    int result = 0;
    int num_presses = 0;
    for (size_t i = 0; i < buttons.size(); ++i)
      if ((combo & (1 << i)) != 0) {
        // Press the button
        result ^= buttons[i];
        ++num_presses;
      }
    if (result == indicator)
      min_presses = min(min_presses.value_or(num_presses), num_presses);
  }
  return min_presses.value();
}

// How many positions does a button effect?
inline int num_affected(int button) { return popcount(unsigned(button)); }

// Does a button affect the given position?
inline bool affects(int button, int pos) {
  return (button & (1 << pos)) != 0;
}

// Press the given button the given number of times
vector<int> press(int button, int times, vector<int> const &remaining) {
  vector<int> pressed(remaining);
  for (size_t i = 0; i < pressed.size(); ++i)
    if (affects(button, i)) {
      pressed[i] -= times;
      assert(pressed[i] >= 0);
    }
  return pressed;
}

// Part 2 can be expressed as an integer linear program...
//
// Let the number of presses of button j be b_j
// b_j >= 0
// Collect the b_j into a vector b.
//
// Let A be a matrix where row i corresponds to joltage i.  a_ij is 1
// if pressing button j affects joltage i, and 0 otherwise.  Then A*b
// is a vector of all the joltages.  I want A*b to equal the target J.
//
// Minimizing the number of presses corresponds to minimizing sum b_j.
//
// So minimize sum b_j subject to A*b = J
//
// For solving it, I'll do a search and use the continuous LP as a
// bounding heuristic.
//
// If I remembered better how the simplex algorithm worked, likely the
// whole thing could be solved directly with a slight variant in
// significantly less code.  For another time...

// Basic simplex routine for the continuous problem
// Adapted from an implementation in the Stanford ACM-ICPC repo
// https://github.com/jaehyunp/stanfordacm/blob/master/code/Simplex.cc
namespace simplex {
// MIT License
//
// Copyright (c) 2018 Jaehyun Park
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Two-phase simplex algorithm for solving linear programs of the form
//
//     maximize     c^T x
//     subject to   Ax <= b
//                  x >= 0
//
// INPUT: A -- an m x n matrix
//        b -- an m-dimensional vector
//        c -- an n-dimensional vector
//        x -- a vector where the optimal solution will be stored
//
// OUTPUT: value of the optimal solution (infinity if unbounded
//         above, nan if infeasible)
//
// To use this code, create an LPSolver object with A, b, and c as
// arguments.  Then, call Solve(x).

#include <limits>

using namespace std;

typedef double DOUBLE;
typedef vector<DOUBLE> VD;
typedef vector<VD> VVD;
typedef vector<int> VI;

const DOUBLE EPS = 1e-9;

struct LPSolver {
  int m, n;
  VI N, B;
  VVD D;

  LPSolver(const VVD &A, const VD &b, const VD &c) :
    m(b.size()), n(c.size()), N(n + 1), B(m), D(m + 2, VD(n + 2)) {
    for (int i = 0; i < m; i++) for (int j = 0; j < n; j++) D[i][j] = A[i][j];
    for (int i = 0; i < m; i++) { B[i] = n + i; D[i][n] = -1; D[i][n + 1] = b[i]; }
    for (int j = 0; j < n; j++) { N[j] = j; D[m][j] = -c[j]; }
    N[n] = -1; D[m + 1][n] = 1;
  }

  void Pivot(int r, int s) {
    double inv = 1.0 / D[r][s];
    for (int i = 0; i < m + 2; i++) if (i != r)
      for (int j = 0; j < n + 2; j++) if (j != s)
        D[i][j] -= D[r][j] * D[i][s] * inv;
    for (int j = 0; j < n + 2; j++) if (j != s) D[r][j] *= inv;
    for (int i = 0; i < m + 2; i++) if (i != r) D[i][s] *= -inv;
    D[r][s] = inv;
    swap(B[r], N[s]);
  }

  bool Simplex(int phase) {
    int x = phase == 1 ? m + 1 : m;
    while (true) {
      int s = -1;
      for (int j = 0; j <= n; j++) {
        if (phase == 2 && N[j] == -1) continue;
        if (s == -1 || D[x][j] < D[x][s] || (D[x][j] == D[x][s] && N[j] < N[s])) s = j;
      }
      if (D[x][s] > -EPS) return true;
      int r = -1;
      for (int i = 0; i < m; i++) {
        if (D[i][s] < EPS) continue;
        if (r == -1 || D[i][n + 1] / D[i][s] < D[r][n + 1] / D[r][s] ||
          ((D[i][n + 1] / D[i][s]) == (D[r][n + 1] / D[r][s]) && B[i] < B[r])) r = i;
      }
      if (r == -1)
        return false;
      Pivot(r, s);
    }
  }

  DOUBLE Solve(VD &x) {
    int r = 0;
    for (int i = 1; i < m; i++) if (D[i][n + 1] < D[r][n + 1]) r = i;
    if (D[r][n + 1] < -EPS) {
      Pivot(r, n);
      if (!Simplex(1) || D[m + 1][n + 1] < -EPS) return -numeric_limits<DOUBLE>::infinity();
      for (int i = 0; i < m; i++) if (B[i] == -1) {
        int s = -1;
        for (int j = 0; j <= n; j++)
          if (s == -1 || D[i][j] < D[i][s] || (D[i][j] == D[i][s] && N[j] < N[s])) s = j;
        Pivot(i, s);
      }
    }
    if (!Simplex(2)) return numeric_limits<DOUBLE>::infinity();
    x = VD(n);
    for (int i = 0; i < m; i++) if (B[i] < n) x[B[i]] = D[i][n + 1];
    return D[m][n + 1];
  }
};
}

// Compute a lower bound for the number of presses based on the
// continuous LP.  If the solution happens to be all integer, the
// second return value is true to indicate that the bound is exact.
pair<int, bool> lower_bound(vector<int> const &buttons,
                            vector<int> const &remaining) {
  // There's one column per button (number of presses), and two rows
  // per nonzero element of remaining (to get an equality constraint).
  // The goal is to minimize the sum of the number of presses.
  vector<vector<double>> A;
  vector<double> b;
  for (size_t i = 0; i < remaining.size(); ++i) {
    // Row has zero at column j if buttons[j] does not affect this
    // position, 1 if it does
    A.emplace_back();
    for (size_t j = 0; j < buttons.size(); ++j)
      A.back().emplace_back(affects(buttons[j], i) ? 1.0 : 0.0);
    b.emplace_back(remaining[i]);
    // Dup with negative signs to get equality
    A.emplace_back(A.back());
    for (auto &a : A.back())
      a = -a;
    b.emplace_back(-b.back());
  }
  // Want to minimize the sum of button presses, so maximize the
  // negative
  vector<double> c(buttons.size(), -1.0);
  vector<double> x;
  auto min_presses = -simplex::LPSolver(A, b, c).Solve(x);
  if (x.empty())
    // Infeasible, i.e., it's in a situation where there's no
    // combination of the buttons that can satisfy all the equality
    // constraints.  I want to cut off, so I'll use -1 with an exact
    // indication to say "no can do".
    return {-1, true};
  assert(finite(min_presses));
  // Check for integer solution and exact match
  assert(x.size() == buttons.size());
  vector<int> from_lp(remaining.size(), 0);
  bool all_int = true;
  int exact_presses = 0;
  for (size_t i = 0; i < buttons.size(); ++i) {
    int presses(round(x[i]));
    exact_presses += presses;
    if (fabs(x[i] - presses) > 1e-6)
      all_int = false;
    for (size_t j = 0; j < remaining.size(); ++j)
      if (affects(buttons[i], j))
        from_lp[j] += presses;
  }
  if (all_int && from_lp == remaining)
    return {exact_presses, true};
  // Throw in a little slop in case of rounding errors
  return {int(floor(min_presses - 1e-6)), false};
}

// Search for the minimum number of presses
// num_presses = presses so far
// buttons = the possible buttons to push
// remaining = target joltages
// best = minimum number of presses found so far
void min_presses(int num_presses, vector<int> const &buttons,
                 vector<int> const &remaining, optional<int> &best) {
  if (best.has_value() && num_presses >= best)
    // Can't possibly do better
    return;
  if (*max_element(remaining.begin(), remaining.end()) == 0) {
    // Goal reached!
    best = num_presses;
    return;
  }
  if (buttons.empty())
    // Nothing left to press
    return;
  // See how many times each button could be pressed, and how many
  // buttons affect each position
  vector<int> max_presses(buttons.size(), 0);
  vector<int> affected(remaining.size(), 0);
  for (size_t i = 0; i < buttons.size(); ++i) {
    optional<int> presses_i;
    for (size_t j = 0; j < remaining.size(); ++j)
      if (affects(buttons[i], j)) {
        presses_i = min(presses_i.value_or(remaining[j]), remaining[j]);
        ++affected[j];
      }
    assert(presses_i.has_value());
    if (*presses_i == 0) {
      // If a button cannot be pressed, toss it
      auto other_buttons = buttons;
      other_buttons.erase(other_buttons.begin() + i);
      min_presses(num_presses, other_buttons, remaining, best);
      return;
    }
    max_presses[i] = *presses_i;
  }
  // See if there are any positions that cannot be affected
  for (size_t i = 0; i < remaining.size(); ++i)
    if (remaining[i] > 0 && affected[i] == 0)
      // Nothing affects this position, so no solution
      return;
  // Try the continuous LP
  auto [from_lp, exact] = lower_bound(buttons, remaining);
  if (exact) {
    if (from_lp == -1)
      // No solution
      return;
    // Add on the presses so far; that's the best possible from here
    from_lp += num_presses;
    best = min(best.value_or(from_lp), from_lp);
    return;
  }
  // LP solution wasn't exact
  if (best.has_value() && from_lp >= *best)
    // Cannot beat the best, even assuming the freedom of continuous
    // presses
    return;
  // See if there's any position that is only affected by one button.
  // That position completely determines the presses of the
  // corresponding button.
  optional<int> single_choice;
  for (size_t i = 0; i < remaining.size(); ++i)
    if (remaining[i] > 0 && affected[i] == 1)
      // Find the button to drive this position to 0
      for (size_t j = 0; j < buttons.size(); ++j)
        if (affects(buttons[j], i)) {
          if (max_presses[j] != remaining[i])
            // Cannot press this button enough to get all the way
            return;
          single_choice = j;
        }
  int index;
  if (single_choice.has_value())
    index = *single_choice;
  else {
    // Pick whatever button affects the most things, breaking ties by
    // whichever button can be pressed the least.  The idea is that by
    // pressing a button that affects lots of things, I'll economize
    // on presses, and also help to determine things by reducing the
    // number of buttons that affect lots of stuff.  Least presses is
    // just to reduce the branching factor.
    index = 0;
    for (size_t i = 1; i < buttons.size(); ++i) {
      auto ci = num_affected(buttons[i]);
      auto cidx = num_affected(buttons[index]);
      if (ci > cidx || (ci == cidx && max_presses[i] < max_presses[index]))
        index = i;
    }
  }
  assert(!single_choice.has_value() || index == *single_choice);
  auto other_buttons = buttons;
  other_buttons.erase(other_buttons.begin() + index);
  // Try all ways to press the button
  int min_times = single_choice.has_value() ? max_presses[index] : 0;
  for (int times = max_presses[index]; times >= min_times; --times) {
    auto next_remaining = press(buttons[index], times, remaining);
    min_presses(num_presses + times, other_buttons, next_remaining, best);
  }
}

int machine::power() const {
  optional<int> best;
  min_presses(0, buttons, joltages, best);
  assert(best.has_value());
  return *best;
}

void configure(int (machine::*what)() const) {
  string line;
  int total = 0;
  while (getline(cin, line))
    total += (machine(line).*what)();
  cout << total << '\n';
}

void part1() { configure(&machine::lights); }
void part2() { configure(&machine::power); }

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
