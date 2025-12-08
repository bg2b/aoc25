// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2
// To run part 1 on input1, set PART1=10 in the environment

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cassert>

// Brute force pairwise computation of distances.  I thought about
// trying to do something smarter, but with only 1000 points it's kind
// of pointless ;-)

using namespace std;

// Number of things to connect for part 1, configurable to be able to
// run test input1 (PART1=10 ./doit 1 < input1)
int to_connect = atoi(getenv("PART1") ? getenv("PART1") : "1000");

// Use a long for the coordinates since five digits is flirting with
// overflow in distance computations
using coord = array<long, 3>;

coord operator-(coord const &c1, coord const &c2) {
  return {c1[0] - c2[0], c1[1] - c2[1], c1[2] - c2[2]};
}

long norm2(coord const &c) {
  return c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
}

// Squared Euclidean distance
long distance2(coord const &c1, coord const &c2) { return norm2(c2 - c1); }

// Generic union/find.  Often I won't bother keeping track of the
// sizes, but here I need them anyway.
struct sets {
  vector<int> parent;
  vector<int> size;

  sets(int n);

  // Representative for set containing i
  int find(int i);
  // Join sets, return size of result
  int onion(int i, int j);

  // Sizes of the k biggest sets
  vector<int> biggest(unsigned k) const;
};

sets::sets(int n) {
  parent.reserve(n);
  for (int i = 0; i < n; ++i)
    parent.push_back(i);
  size.assign(n, 1);
}

int sets::find(int i) {
  if (i == parent[i])
    return i;
  return parent[i] = find(parent[i]);
}

int sets::onion(int i, int j) {
  i = find(i);
  j = find(j);
  if (i != j) {
    if (size[i] < size[j])
      swap(i, j);
    parent[j] = i;
    size[i] += size[j];
  }
  return size[i];
}

vector<int> sets::biggest(unsigned k) const {
  vector<int> rep_sizes;
  int n = parent.size();
  for (int i = 0; i < n; ++i)
    if (i == parent[i])
      rep_sizes.push_back(size[i]);
  assert(rep_sizes.size() >= k);
  sort(rep_sizes.begin(), rep_sizes.end());
  return vector<int>(rep_sizes.end() - k, rep_sizes.end());
}

struct boxes {
  vector<coord> cs;

  // Construct from cin
  boxes();

  // Connect, either for a given number of steps (part1) or until
  // forming a complete circuit (!part1).  Return either product of
  // biggest 3 parts (part1) or product of x coordinates of last made
  // connection (!part1)
  long connect(bool part1);
};

boxes::boxes() {
  coord c;
  char comma;
  while (cin >> c[0] >> comma >> c[1] >> comma >> c[2])
    cs.push_back(c);
}

long boxes::connect(bool part1) {
  vector<pair<long, pair<int, int>>> distances;
  int n = cs.size();
  for (int i = 0; i < n; ++i)
    for (int j = i + 1; j < n; ++j)
      distances.push_back({distance2(cs[i], cs[j]), {i, j}});
  sort(distances.begin(), distances.end());
  sets circuits(n);
  for (int k = 0; ; ) {
    auto [i, j] = distances[k++].second;
    int sz = circuits.onion(i, j);
    if (!part1 && sz == n)
      return cs[i][0] * cs[j][0];
    if (part1 && k == to_connect) {
      auto biggest = circuits.biggest(3);
      return biggest[0] * biggest[1] * biggest[2];
    }
  }
}

void part1() { cout << boxes().connect(true) << '\n'; }
void part2() { cout << boxes().connect(false) << '\n'; }

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
