#include "solvers/solver1.h"
#include "data/case.h"
#include <algorithm>

enum die_index{TOP, BOTTOM};

Solver1::Solver1(Case& case_): Solver(case_) {}

bool Solver1::check_capacity(int index, int die_cell_index) {
    if (index == TOP) {
        return die_util[TOP] + case_.top_die_.tech.lib_cells[die_cell_index].get_cell_size() <= die_max_util[TOP];
    }
    else {
        return (die_util[BOTTOM] + case_.bottom_die_.tech.lib_cells[die_cell_index].get_cell_size() <= die_max_util[BOTTOM]);
    }
}

void Solver1::solve() {
    die_size = case_.size_.upper_right_x * case_.size_.upper_right_y;
    die_max_util = {case_.top_die_.max_util, case_.bottom_die_.max_util};
    die_util = {0, 0};
    std::vector<std::string> macro_list = case_.get_macro_list();
    std::vector<std::string> macro_C_index, cell_C_index;

    // separate macros and cells
    for (auto& inst: case_.netlist_.inst_) {
        if (std::find(macro_list.begin(), macro_list.end(), inst.second) != macro_list.end()){
            macro_C_index.push_back(inst.first);
        }
        else {
            cell_C_index.push_back(inst.first);
        }
    } 

    // place macros
    for (auto& inst: macro_C_index) {
        std::string inst_name = inst;
        std::string inst_type = case_.netlist_.inst_[inst_name];
        int die_cell_index = case_.get_cell_index(inst_type);

        bool alter = ((die_max_util[TOP] - die_util[TOP]) > (die_max_util[BOTTOM] - die_util[BOTTOM]))? true: false;

        if(alter && check_capacity(TOP, die_cell_index)) {
            die_util[TOP] += case_.top_die_.tech.lib_cells[die_cell_index].get_cell_size() / die_size;
        }
        else if(!alter && check_capacity(BOTTOM, die_cell_index)) {
            die_util[BOTTOM] += case_.bottom_die_.tech.lib_cells[die_cell_index].get_cell_size() / die_size;
        }
        else {
            std::cout << "Error: Die utilization exceeds maximum utilization" << std::endl;
            exit(1);
        }
    }

    // place cells
    for (auto& inst: cell_C_index) {
        std::string inst_name = inst;
        std::string inst_type = case_.netlist_.inst_[inst_name];
        int die_cell_index = case_.get_cell_index(inst_type);

        bool alter = ((die_max_util[TOP] - die_util[TOP]) > (die_max_util[BOTTOM] - die_util[BOTTOM]))? true: false;

        if(alter && check_capacity(TOP, die_cell_index)) {
            die_util[TOP] += case_.top_die_.tech.lib_cells[die_cell_index].get_cell_size() / die_size;
        }
        else if(!alter && check_capacity(BOTTOM, die_cell_index)) {
            die_util[BOTTOM] += case_.bottom_die_.tech.lib_cells[die_cell_index].get_cell_size() / die_size;
        }
        else {
            std::cout << "Error: Die utilization exceeds maximum utilization" << std::endl;
            exit(1);
        }
    }
}