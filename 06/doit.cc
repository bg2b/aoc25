// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <cassert>

using namespace std;

struct problem {
  char op;
  vector<string> rep;

  // Peel off one problem from the ends of the lines (mutates lines).
  // All lines are the same length initially, and remain the same
  // length afterwards.
  problem(vector<string> &lines);

  // Extract numbers row by row
  vector<long> by_row() const;
  // Extract numbers column by column
  vector<long> by_column() const;

  // Evaluate with the appropriate number extraction rule
  long solve(bool part1) const;
};

problem::problem(vector<string> &lines) {
  auto pos = lines.back().find_last_of("+*");
  assert(pos != string::npos);
  for (auto &line : lines) {
    rep.push_back(line.substr(pos));
    line.resize(pos);
  }
  op = rep.back()[0];
  assert(op == '+' || op == '*');
  rep.pop_back();
}

vector<long> problem::by_row() const {
  vector<long> result;
  for (auto const &s : rep)
    result.push_back(stol(s));
  return result;
}

vector<long> problem::by_column() const {
  auto cols = rep;
  vector<long> result;
  while (!cols.front().empty()) {
    // Read down and remove the last column, skipping spaces
    string col;
    for (auto &s : cols) {
      if (s.back() != ' ')
        col.push_back(s.back());
      s.pop_back();
    }
    // An empty column is possible and is part of the separation
    // between problems
    if (!col.empty())
      result.push_back(stol(col));
  }
  return result;
}

long problem::solve(bool part1) const {
  auto nums = part1 ? by_row() : by_column();
  if (op == '+')
    return accumulate(nums.begin(), nums.end(), 0l);
  else
    return accumulate(nums.begin(), nums.end(), 1l,
                      [](long n1, long n2) { return n1 * n2; });
}

void solve(bool part1) {
  vector<string> lines;
  string line;
  while (getline(cin, line))
    lines.emplace_back(line);
  assert(!lines.empty());
  long total = 0;
  while (!lines.front().empty())
    total += problem(lines).solve(part1);
  cout << total << '\n';
}

void part1() { solve(true); }
void part2() { solve(false); }

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
