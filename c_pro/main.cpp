/*************************
Copyright 2026 Charles Gretton

This file is part of Dagster toolchain.

Dagster is free software; you can redistribute it 
and/or modify it under the terms of the GNU General 
Public License as published by the Free Software 
Foundation; either version 2 of the License, or
(at your option) any later version.

Dagster is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE. See the GNU General Public 
License for more details.

You should have received a copy of the GNU General 
Public License along with Dagster.
If not, see <http://www.gnu.org/licenses/>.
*************************/


#include "parsing.h"
#include "up.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <utility>

// Utility function for writing clause and variable ranges to DAG files
std::string format_ranges(std::vector<int>& indices) {
  if (indices.empty()) return "";
  std::sort(indices.begin(), indices.end());
  std::stringstream ss;
  for (size_t i = 0
	 ; i < indices.size()
	 ; i++) {
    int start = indices[i];
    while (i + 1 < indices.size() && indices[i + 1] == indices[i] + 1) {
      i++;
    }
    int end = indices[i];
    if (start == end) { ss<<start;}
    else {ss<<start<<"-"<<end;}

    
    if (i + 1 < indices.size()) {ss<<",";}
  }
  return ss.str();
}

std::vector<std::pair<int, int>> rank_vars__optimised(const CNF& cnf) {
  int epoch = 1;
  std::vector<std::vector<int>> adj(cnf.num_vars + 1); // variable's adjacency in clauses
  std::vector<int> seen(cnf.num_vars + 1, 0); // from variable to epoch

  for (int ci = 0
	 ; ci < cnf.num_clauses
	 ; ci++) {
    size_t begin = cnf.clause_offsets[ci];
    size_t end = cnf.clause_offsets[ci + 1];
        
    static std::vector<int> vars_in_clause;
    vars_in_clause.clear();
    for (size_t k = begin
	   ; k < end
	   ; k++) {
      int v = std::abs(cnf.lits[k]);
      if (seen[v] != epoch) {
	seen[v] = epoch;
	vars_in_clause.push_back(v);
      }
    }
    
    for (size_t i = 0
	   ; i < vars_in_clause.size()
	   ; i++) {
      for (size_t j = i + 1
	     ; j < vars_in_clause.size()
	     ; j++) {
	int u = vars_in_clause[i];
	int v = vars_in_clause[j];
	adj[u].push_back(v);
	adj[v].push_back(u);
      }
    }
    epoch++;
  }

  std::vector<std::pair<int, int>> ranked;
  for (int v = 1
	 ; v <= cnf.num_vars
	 ; v++) {
    auto& neighbors = adj[v];
    if (neighbors.empty()) { /*std::cerr<<"Variable "<<v<<" has no neighbors."<<std::endl;*/ /*assert (0);*/ continue;}
    
    std::sort(neighbors.begin(), neighbors.end());
    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
    
    ranked.push_back({v, (int)neighbors.size()});
  }

  std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
    if (a.second != b.second) return a.second > b.second;
    return a.first < b.first;
  });

  return ranked; // I assume Named Return Value Optimisation
}

void generate_dagster_input_files(const CNF& cnf, int N, const std::string& prefix) {
  auto ranks = rank_vars__optimised(cnf);
  int count = 1;
  std::vector<int> top_vars;
  for (const auto& [var_id, degree] : ranks) {
    top_vars.push_back(var_id);
    count++;
    if (count > N) break;
  }
  
  std::vector<bool> is_top_var(cnf.num_vars + 1, false);
  for (int v : top_vars) is_top_var[v] = true;

  std::vector<int> node0_indices;
  for (int c = 0
	 ; c < cnf.num_clauses
	 ; c++) {
    for (size_t k = cnf.clause_offsets[c]
	   ; k < cnf.clause_offsets[c + 1]
	   ; k++) {
      if (is_top_var[std::abs(cnf.lits[k])]) {
	node0_indices.push_back(c);
	break;
      }
    }
  }

  std::ofstream cnf_out(prefix + ".cnf");
  cnf_out<<"p cnf "<<cnf.num_vars<<" "<<cnf.num_clauses<<"\n";
  for (int c = 0
	 ; c < cnf.num_clauses
	 ; c++) {
    for (size_t k = cnf.clause_offsets[c]
	   ; k < cnf.clause_offsets[c+1]
	   ; k++) {
      cnf_out<<cnf.lits[k]<<" ";
    }
    cnf_out<<"0\n";
  }
  cnf_out.close();

  std::ofstream dag_out(prefix + ".dag");
  dag_out<<"DAG-FILE\n";
  dag_out<<"NODES:2\n";
  dag_out<<"GRAPH:\n";
  dag_out<<"0->1:";
  for (size_t i = 0
	 ; i < top_vars.size()
	 ; i++) {
    dag_out<<top_vars[i]<<((i == top_vars.size() - 1)?"":",");
  }
  dag_out<<"\n";
  
  dag_out<<"CLAUSES:\n";
  dag_out<<"0:"<<format_ranges(node0_indices)<<"\n";
  dag_out<<"1:0-"<<(cnf.num_clauses - 1)<<"\n";
  dag_out<<"REPORTING:\n";
  dag_out<<"1-"<<cnf.num_vars<<"\n";
  dag_out.close();

  
  std::ofstream trivial_dag_out("_" + prefix + ".dag");
  trivial_dag_out<<"DAG-FILE\n";
  trivial_dag_out<<"NODES:1\n";
  trivial_dag_out<<"GRAPH:\n";
  trivial_dag_out<<"CLAUSES:\n";
  trivial_dag_out<<"0:0-"<<(cnf.num_clauses - 1)<<"\n";
  trivial_dag_out<<"REPORTING:\n";
  trivial_dag_out<<"1-"<<cnf.num_vars<<"\n";
  trivial_dag_out.close();
  
  std::cout<<"Successfully generated "<<prefix<<".cnf, "<<prefix<<".dag and the trivial DAG _"<<prefix<<".dag\n";
}

int parse_positive_int(const char* text, const char* name) {
  errno = 0;
  char* end = nullptr;
  const long value = std::strtol(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0' || value < 1 || value > INT_MAX) {
    std::cerr<<"Invalid "<<name<<": "<<(text ? text : "(null)")
             <<"; expected a positive integer.\n";
    std::exit(EXIT_FAILURE);
  }
  return static_cast<int>(value);
}

int main(int argc, char** argv){
  if (argc > 4) {
    std::cerr<<"Usage: "<<argv[0]
             <<" [input.cnf [split-width [output-prefix]]]\n";
    return 1;
  }

  const char* input_cnf_filename = (argc > 1) ? argv[1] : nullptr;
  const int split_width = (argc > 2)
    ? parse_positive_int(argv[2], "split width")
    : 32;
  const std::string output_prefix = (argc > 3) ? argv[3] : "x";

  FILE* file_pointer = input_cnf_filename ? std::fopen(input_cnf_filename, "rb") : stdin; // assume stdin if no input file given
  if (!file_pointer) { std::perror("Unable to open intput file."); return 1; }
  
  try {
    CNF _cnf;
    CNF cnf;
    parse_file(file_pointer, _cnf);
    if (input_cnf_filename) std::fclose(file_pointer);    
    up::Result r = up::simplify(_cnf, cnf, true);
    assert(r.sat != false);
    
    // FOR DEBUGGING ONLY -- Print CNF parsed
    // std::printf("vars=%d clauses=%d lits=%zu\n",
    // 		cnf.num_vars, cnf.num_clauses, cnf.lits.size());
    // for (int c = 0
    // 	   ; c < cnf.num_clauses
    // 	   ; c++) {
    //   size_t b = cnf.clause_offsets[c], e = cnf.clause_offsets[c+1];
    //   for (size_t i = b; i < e; i++) std::fprintf(stderr,"%d ", cnf.lits[i]);
    //   std::puts("0");
    // }

    // FOR DEBUGGING ONLY -- Print variable ranks
    auto ranks = rank_vars__optimised(cnf);
    int rank = 1;
    for (const auto& [var_id, degree] : ranks) {
      std::cerr<<rank<<"\t"<<var_id<<"\t"<<degree<<"\n";
      rank++;
    }


    // Print DAG and CNF files for Dagster
    std::cerr<<"Generating c_pro DAG with split width "<<split_width
             <<" and output prefix '"<<output_prefix<<"'.\n";
    generate_dagster_input_files(cnf, split_width, output_prefix);
    
  } catch (const std::exception& e) {
    if (input_cnf_filename) std::fclose(file_pointer);
    std::fprintf(stderr, "Parse error: %s\n", e.what());
    return 2;
  }
  return 0;
}
