#include "data/netlist.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

std::istream& operator>>(std::istream& input, Net& net) {
  std::string dummy;
  std::string data;

  for (int i = 0; i < net.num_pins; ++i) {
    input >> dummy >> data;
    const int split_pos = data.find('/');
    net.pins.emplace(data.substr(0, split_pos), data.substr(split_pos + 1));
  }

  return input;
}

std::istream& operator>>(std::istream& input, NetList& net_list) {
  std::string dummy;
  int instance_count;
  input >> dummy >> instance_count;

  std::string inst_name;
  std::string lib_cell_name;
  for (int i = 0; i < instance_count; ++i) {
    input >> dummy >> inst_name >> lib_cell_name;
    net_list.inst_.emplace(inst_name, lib_cell_name);
  }

  int net_count;
  input >> dummy >> net_count;
  std::string net_name;
  int num_pins;
  for (int i = 0; i < net_count; ++i) {
    input >> dummy >> net_name >> num_pins;
    Net net{net_name, num_pins, {}};
    input >> net;
    net_list.nets_.push_back(std::move(net));
  }

  return input;
}
