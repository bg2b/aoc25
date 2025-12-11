// -*- C++ -*-
// g++ -std=c++20 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cassert>

using namespace std;

struct devices {
  // Numbering of the devices for edge storage
  map<string, int> indexes;
  // outputs[i] = whatever device i outputs to
  vector<vector<int>> outputs;

  // Construct from cin
  devices();

  int index(string const &s) const { return indexes.at(s); }
  int index(string const &s);
  int num_devices() const { return outputs.size(); }

  // Number of paths between two devices
  long num_paths(string const &from, string const &to) const;
};

devices::devices() {
  string line;
  while (getline(cin, line)) {
    stringstream ss(line);
    string device;
    ss >> device;
    assert(device.length() > 1 && device.back() == ':');
    device.pop_back();
    int from = index(device);
    while (ss >> device) {
      // Careful with the sequencing, since outputs can be resized by
      // index(device)
      int to = index(device);
      outputs[from].push_back(to);
    }
  }
}

int devices::index(string const &s) {
  if (auto p = indexes.find(s); p != indexes.end())
    return p->second;
  int result = num_devices();
  indexes.emplace(s, result);
  outputs.emplace_back();
  return result;
}

long devices::num_paths(string const &from, string const &to) const {
  // Cache of number of paths to to ;-).  -1 means "not computed"
  vector<long> to_to(num_devices(), -1);
  to_to[index(to)] = 1;
  // Calculate how many paths there are from device d to to
  auto search = [&](int d, auto &self) -> long {
    if (to_to[d] == -1) {
      to_to[d] = 0;
      for (auto output : outputs[d])
        to_to[d] += self(output, self);
    }
    return to_to[d];
  };
  return search(index(from), search);
}

void part1() { cout << devices().num_paths("you", "out") << '\n'; }

void part2() {
  devices d;
  string dev1("fft");
  string dev2("dac");
  // I'm assuming that there aren't cycles, which is true for my
  // input.  I initially was thinking that there could possibly be a
  // loop involving fft and dac.  But if you have to visit each
  // exactly once, the constraint could effectively break all cycles.
  // For example,
  //   srv: fft
  //   srv: dac
  //   dac: fft
  //   dac: out
  //   fft: dac
  //   fft: out
  // That would have two paths, srv-fft-dac-out and srv-dac-fft-out.
  // Anyway, my input is such that something like this doesn't happen.
  auto d12 = d.num_paths(dev1, dev2);
  if (d12 == 0) {
    swap(dev1, dev2);
    d12 = d.num_paths(dev1, dev2);
  }
  assert(d.num_paths(dev2, dev1) == 0);
  // Ways to go from svr to dev1, then to dev2, then to out
  cout << d.num_paths("svr", dev1) * d12 * d.num_paths(dev2, "out") << '\n';
}

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
