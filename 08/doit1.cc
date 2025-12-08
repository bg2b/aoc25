// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2
// To run part 1 on input1, set PART1=10 in the environment

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cassert>

// Coarsens coordinates to a smallish grid, then computes pairwise
// distances within self and adjacent grid cells.

using namespace std;

// Number of things to connect for part 1, configurable to be able to
// run test input1 (PART1=10 ./doit1 1 < input1)
int to_connect = atoi(getenv("PART1") ? getenv("PART1") : "1000");

// Use a long for the coordinates since five digits is flirting with
// overflow in distance computations
using coord = array<long, 3>;

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1], c1[2] + c2[2]};
}

coord operator-(coord const &c) { return {-c[0], -c[1], -c[2]}; }
coord operator-(coord const &c1, coord const &c2) { return c1 + (-c2); }

long max(coord const &c) { return max(max(c[0], c[1]), c[2]); }
long min(coord const &c) { return min(min(c[0], c[1]), c[2]); }

coord operator/(coord const &c, long d) {
  return {c[0] / d, c[1] / d, c[2] / d};
}

// Squared Euclidean distance
long distance2(coord const &c1, coord const &c2) {
  auto c = c2 - c1;
  return c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
}

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

// Return a list of points that are at the smallest distances
vector<pair<long, pair<int, int>>>
coarsen(vector<coord> const &cs, int splits) {
  // Get maximum coordinate
  long d = 0;
  for (auto const &c : cs)
    d = max(d, max(c));
  // Compute bin sizes
  int num_bins = splits + 1;
  long bin_size = max((d + 1 + num_bins - 1) / num_bins, 1l);
  // Two points could be dist_limit2 apart and be missed (say one is
  // on the edge of one coarse box and the other is at the edge
  // another coarse box that's two away).  So I have to consider
  // anything that's this large as suspect.
  long dist_limit2 = bin_size * bin_size;
  // I'll flatten out the coordinates in the coarse grid for less loop
  // nesting...
  vector<vector<int>> bins(num_bins * num_bins * num_bins);
  // Index for a coarse point
  auto bin_index = [&](coord const &coarse) {
    assert(min(coarse) >= 0 && max(coarse) < num_bins);
    return ((coarse[0] * num_bins) + coarse[1]) * num_bins + coarse[2];
  };
  // Index for a point
  auto coarse_index = [&](coord const &c) { return bin_index(c / bin_size); };
  int n = cs.size();
  for (int i = 0; i < n; ++i)
    bins[coarse_index(cs[i])].push_back(i);
  // Offsets to adjacent boxes
  vector<coord> deltas;
  for (int i = -1; i <= +1; ++i)
    for (int j = -1; j <= +1; ++j)
      for (int k = -1; k <= +1; ++k)
        deltas.push_back({i, j, k});
  vector<pair<long, pair<int, int>>> result;
  // Scan the coarse grid
  for (int i = 0; i < num_bins; ++i)
    for (int j = 0; j < num_bins; ++j)
      for (int k = 0; k < num_bins; ++k) {
        auto const &bin2 = bins[bin_index({i, j, k})];
        if (bin2.empty())
          continue;
        // Look at the adjacent boxes
        for (auto const &delta : deltas) {
          auto coarse = coord{i, j, k} + delta;
          if (min(coarse) < 0 || max(coarse) >= num_bins)
            // Out of bounds
            continue;
          for (auto i1 : bins[bin_index(coarse)])
            for (auto i2 : bin2)
              if (i1 < i2) {
                auto d12 = distance2(cs[i1], cs[i2]);
                if (d12 < dist_limit2)
                  // Close enough to be guaranteed valid
                  result.push_back({d12, {i1, i2}});
              }
        }
      }
  return result;
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
  int n = cs.size();
  // Assuming a uniform random distribution and wanting about two
  // boxes per coarse cell...
  int splits = int(cbrt(n / 2));
  while (true) {
    assert(splits >= 0);
    auto distances = coarsen(cs, splits);
    sort(distances.begin(), distances.end());
    sets circuits(n);
    int max_k = distances.size();
    for (int k = 0; k < max_k; ) {
      auto [i, j] = distances[k++].second;
      int sz = circuits.onion(i, j);
      if (part1 && k == to_connect) {
        auto biggest = circuits.biggest(3);
        return biggest[0] * biggest[1] * biggest[2];
      }
      if (!part1 && sz == n)
        return cs[i][0] * cs[j][0];
    }
    // Didn't collect enough edges, try again
    --splits;
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
