#include <ctime>
#include <fstream>
#include <utility>

#include "data/case.h"
#include "solvers/solver1.h"

int main(int argc, char** argv) {
  // run time
  const clock_t start = clock();

  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
    return 1;
  }

  std::ifstream in_file(argv[1]);

  Case case_;
  in_file >> case_;
  Solver1 solver(std::move(case_));

  solver.solve();

  std::ofstream out_file(argv[2]);
  solver.dump(out_file);
  const clock_t end = clock();
  std::cout << "Run Times: "
            << static_cast<double>(end - start) / CLOCKS_PER_SEC << "s"
            << std::endl;
  return 0;
}