#ifndef SRC_INCLUDE_DATA_NETLIST_H_
#define SRC_INCLUDE_DATA_NETLIST_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Net {
  std::string name;
  int num_pins;
  std::unordered_map<std::string, std::string> pins;

  friend std::istream& operator>>(std::istream& input, Net& net);
};

struct NetList {
  std::unordered_map<std::string, std::string> inst_;
  std::vector<Net> nets_;

  friend std::istream& operator>>(std::istream& input, NetList& net);
};

#endif  // SRC_INCLUDE_DATA_NETLIST_H_
