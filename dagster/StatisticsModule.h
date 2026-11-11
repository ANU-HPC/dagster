/*************************
Copyright 2020 Mark Burgess

This file is part of Dagster.

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


#ifndef STATISTICS_MODULE_H_
#define STATISTICS_MODULE_H_

#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <glog/logging.h>

// MeanModule:
//  keeps track of a mean of a series of input numbers
class MeanModule {
  public:
  long counter;
  double existing_mean;
  void add(double time) {
    existing_mean = (counter*existing_mean+time)/(counter+1);
    counter++;
  }
  MeanModule() {
    counter = 0;
    existing_mean = 0;
  }
};

// NodeStatisticsModule:
//  A class that holds and keeps statistical information about the information provided-to and generated-by a particular node in the dag
//  Most notably, for each node a statisticsmodule is created, and timing input is provided to it,
//  various statistical information is updated and kept, and output later
class NodeStatisticsModule {
  public:
  MeanModule message_time,solution_time,unsat_time;
  MeanModule message_time2,solution_time2,unsat_time2;
  
  void add_message_count(double time, double unsat_time) {
    this->message_time.add(time);
    this->message_time2.add(time*time);
    this->unsat_time.add(unsat_time);
    this->unsat_time2.add(unsat_time*unsat_time);
  }
  
  void add_solution_count(double time) {
    this->solution_time.add(time);
    this->solution_time2.add(time*time);
  }
};

// StatisticsModule:
//   a class that keeps track of uptodate timing information of the relationship between workers and their messages wrt the dag nodes.
//   Master calls this class with minimal information, and this class handles the storing and processing of timing information.
class StatisticsModule {
  public:
  std::vector<NodeStatisticsModule> node_statistics;
  std::vector<clock_t> worker_start_time;
  std::vector<clock_t> worker_last_solution_time;
  std::vector<int> worker_message_node;
  std::vector<int> worker_status; // 0 if not working on a message, 1 if working on a message
  
  int no_nodes,no_workers;
  
  StatisticsModule(int no_workers, int no_nodes) {
    this->no_nodes = no_nodes;
    this->no_workers = no_workers;
    node_statistics.resize(no_nodes);
    worker_start_time.resize(no_workers);
    worker_last_solution_time.resize(no_workers);
    worker_message_node.resize(no_workers);
    worker_status.resize(no_workers);
    for (int i=0; i<no_workers; i++) {
      worker_message_node[i] = 0;
      worker_status[i] = 0;
    }
  }
  
  void start_message(int worker_index, int node) {
    if ((worker_index<0) || (worker_index>=no_workers) || (node<0) || (node>=no_nodes)) {
      VLOG(3) << "MASTER: inappropriate use of statisticsmodule, start_message with worker " << worker_index << " and node " << node << ", whereas " << no_workers << " workers, and " << no_nodes << " nodes.";
    }
    //if (worker_status[worker_index] != 0) {
    //  VLOG(3) << "MASTER: inappropriate use of statisticsmodule, bad start_message";
    //}
    clock_t t = clock();
    worker_message_node[worker_index] = node;
    worker_start_time[worker_index] = t;
    worker_last_solution_time[worker_index] = t;
    worker_status[worker_index] = 1;
  }
  void register_solution(int worker_index) {
    if ((worker_index<0) || (worker_index>=no_workers)) {
      VLOG(3) << "MASTER: inappropriate use of statisticsmodule, register_solution with worker " << worker_index << ", whereas " << no_workers << " workers.";
    }
    if (worker_status[worker_index] != 1) {
      VLOG(3) << "MASTER: inappropriate use of statisticsmodule, bad register_solution";
    }
    clock_t t = clock();
    clock_t duration = t - worker_last_solution_time[worker_index];
    worker_last_solution_time[worker_index] = t;
    node_statistics[worker_message_node[worker_index]].add_solution_count(((double)duration)/CLOCKS_PER_SEC);
  }
  void register_finish(int worker_index) {
    if ((worker_index<0) || (worker_index>=no_workers)) {
      VLOG(3) << "MASTER: inappropriate use of statisticsmodule, register_solution with worker " << worker_index << ", whereas " << no_workers << " workers.";
    }
    if (worker_status[worker_index] != 1) {
      VLOG(3) << "MASTER: inappropriate use of statisticsmodule, bad register_finish";
    }
    clock_t t = clock();
    node_statistics[worker_message_node[worker_index]].add_message_count(((double)(t-worker_start_time[worker_index]))/CLOCKS_PER_SEC,
      ((double)(t-worker_last_solution_time[worker_index]))/CLOCKS_PER_SEC);
    worker_status[worker_index] = 0;
    worker_message_node[worker_index] = 0;
  }

  void print_stats() {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    for (int i=0; i<no_nodes; i++) {
      ss << i << ":" << node_statistics[i].message_time.counter << "," << node_statistics[i].solution_time.counter << ":";
      if (node_statistics[i].message_time.counter>0) {
        double mean_message_time = node_statistics[i].message_time.existing_mean;
        double standard_deviation_message_time = sqrt(node_statistics[i].message_time2.existing_mean - node_statistics[i].message_time.existing_mean*node_statistics[i].message_time.existing_mean);
        ss << mean_message_time << "spm" << ((100.0*standard_deviation_message_time)/mean_message_time) << "%,";
      } else {
        ss << "###" << "spm" << "###" << "%,";
      }
      if (node_statistics[i].solution_time.counter>0) {
        double mean_solution_time = node_statistics[i].solution_time.existing_mean;
        double standard_deviation_solution_time = sqrt(node_statistics[i].solution_time2.existing_mean - node_statistics[i].solution_time.existing_mean*node_statistics[i].solution_time.existing_mean);
        ss << "S"<< mean_solution_time << "spm" << ((100.0*standard_deviation_solution_time)/mean_solution_time) << "%,";
      } else {
        ss << "S###" << "spm" << "###" << "%,";
      }
      if (node_statistics[i].unsat_time.counter>0) {
        double mean_unsat_time = node_statistics[i].unsat_time.existing_mean;
        double standard_deviation_unsat_time = sqrt(node_statistics[i].unsat_time2.existing_mean - node_statistics[i].unsat_time.existing_mean*node_statistics[i].unsat_time.existing_mean);
        ss << "U"<< mean_unsat_time << "spm" << ((100.0*standard_deviation_unsat_time)/mean_unsat_time) << "%, ";
      } else {
        ss << "U###" << "spm" << "###" << "%,";
      }
          }
    VLOG(3) << "MASTER: Timings -- " << ss.str();
  }  
};

#endif
