#include <fstream>

#include "data/case.h"
#include "solvers/solver1.h"
#include "data/netlist.h"

int main(int argc, char** argv) {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
    return 1;
  }

  std::ifstream in_file(argv[1]);

  Case case_;
  in_file >> case_;
  Solver1 solver(case_);

  solver.solve();

  std::ofstream out_file(argv[2]);
  solver.dump(out_file);
  return 0;
}