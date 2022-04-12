/*************************
Copyright 2020 Eamon Barett

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



#include "message.h"
#include "BDDSolutions.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <utility>
#include <glog/logging.h>
#include "utilities.h"
#include "exceptions.h"
#include <cassert>
#include "mpi_global.h"

void BDDSolutions::register_message_completion(Message* m) {
  VLOG(5) << "adding completed message " << *m;
  DdNode *dis, *var, *tmp, *tmp2;
  dis = Cudd_ReadOne(ddmgr);
  Cudd_Ref(dis);
  VLOG(3) << "register completion message size " << m->assignments.size();
  for (int i=0; i<m->assignments.size(); i++) {
    if (m->assignments[i]<0) {
      var = Cudd_Not(Cudd_bddIthVar(ddmgr, -m->assignments[i]));
      Cudd_Ref(var);
    }
    else {
      var = Cudd_bddIthVar(ddmgr, m->assignments[i]);
      Cudd_Ref(var);
    }
    tmp = Cudd_bddAnd(ddmgr, dis, var);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(ddmgr, dis);
    Cudd_RecursiveDeref(ddmgr, var);
    dis = tmp;
  }
  // now OR this new dd to the completed m->from diagram
  tmp2 = Cudd_bddOr(ddmgr, completed[m->from], dis);
  Cudd_Ref(tmp2);
  Cudd_RecursiveDeref(ddmgr, dis);
  Cudd_RecursiveDeref(ddmgr, completed[m->from]);
  completed[m->from] = tmp2;
}

int** BDDSolutions::get_additional_clauses(int node) {
  int** neg_rep;
  if(cubes == bdd_compilation_scheme){
    neg_rep = compile_negated_cnf(node);
  } else if ( minisat == bdd_compilation_scheme ) {
    neg_rep = encode_negated_cnf(node);
  } else if ( paths == bdd_compilation_scheme ){
    neg_rep = path_based_encode_negated_cnf(node);
  } else {
    assert(0);
    exit(-1);
  }

  if (neg_rep) {
    int* var_map = neg_rep[1];
    int var_map_size = 1;
    for (int i=1; var_map[i]; i++) {
      var_map_size++;
    }
    int** additional_clauses;
    TEST_NOT_NULL(additional_clauses = (int**)calloc(neg_rep[0][1]+2, sizeof(int*)))
    int i;
    for (i=2; neg_rep[i]; i++) {
      int count = 0;
      while (neg_rep[i][count]) {
        count++;
      }
      TEST_NOT_NULL(additional_clauses[i-2] = (int*)calloc(count+2, sizeof(int)))
      for (int j=0; j<count; j++) {
        if (abs(neg_rep[i][j]) < var_map_size) {
          // if variable, then map it to the master cnf variable using var_map
          additional_clauses[i-2][j] = neg_rep[i][j] > 0 ? var_map[neg_rep[i][j]] : -var_map[-neg_rep[i][j]];
        } else {
          additional_clauses[i-2][j] = neg_rep[i][j];  // if auxvar then just leave the number unchanged
        }
      }
      free(neg_rep[i]);
    }
    additional_clauses[i-2] = NULL;
    free(neg_rep[0]);
    free(neg_rep[1]);
    free(neg_rep);
    return additional_clauses;    
  } else {
    return NULL;
  }
}


void BDDSolutions::set__BDD_compilation_scheme(const std::string& scheme)
{
  if ( "cubes" == scheme) {
    bdd_compilation_scheme = cubes;
  } else if ( "minisat"  == scheme ){
    bdd_compilation_scheme = minisat;
  } else if ( "paths" == scheme ){
    bdd_compilation_scheme = paths;
  } else {
    assert(0);
    exit(-1);
  }
}

BDDSolutions::BDD_compilation_scheme BDDSolutions::get__BDD_compilation_scheme() const
{
  return bdd_compilation_scheme;
}


// does not consider provenance
BDDSolutions::BDDSolutions(Dag* new_dag, int vc)
  : WrappedSolutionsInterface(new_dag),
    bdd_compilation_scheme(cubes)
{
  if (!new_dag) {
    throw BadParameterException("cannot initialise master with NULL dag)");
  }
  this->vc = vc;
  initial_messages.clear();
  dag = new_dag;
  max_depth =  &(dag->max_depth);
  // initialise BDDs
  ddmgr = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS,0);
  bdds.resize(dag->no_nodes);
  communicated.resize(dag->no_nodes);
  completed.resize(dag->no_nodes);
  for (int i=0; i<dag->no_nodes; i++) {
    bdds[i].resize(dag->no_nodes);
    communicated[i] = Cudd_ReadLogicZero(ddmgr);
    Cudd_Ref(communicated[i]);
    completed[i] = Cudd_ReadLogicZero(ddmgr);
    Cudd_Ref(completed[i]);
    for (int j=0; j<dag->no_nodes; j++) {
      bdds[i][j] = Cudd_ReadLogicZero(ddmgr);
      Cudd_Ref(bdds[i][j]);
    }
  }
  // place an initial empty message from and to itself for all base_nodes
  /*for (int i=0; i<dag->no_nodes; i++) {
    if (dag->node_status[i] == 1) {
      Message* m = new Message(i, i);
      initial_messages.push_back(m);
    }
  }*/
}

BDDSolutions::~BDDSolutions() {
  initial_messages.clear();
  for (int i=0; i<dag->no_nodes; i++) {
    Cudd_RecursiveDeref(ddmgr, communicated[i]);
    Cudd_RecursiveDeref(ddmgr, completed[i]);
    for (int j=0; j<dag->no_nodes; j++) {
      Cudd_RecursiveDeref(ddmgr, bdds[i][j]);
    }
    bdds[i].clear();
  }
  communicated.clear();
  completed.clear();
  bdds.clear();
  Cudd_Quit(ddmgr);
}

// add the assignments in the message to the appropriate BDD
void BDDSolutions::add_message(Message* m) {
  VLOG(3) << "adding message size " << m->assignments.size();
  VLOG(5) << "adding message " << *m;
  // check the indices
  if ((m->from >=0) && (m->to >= 0) && (m->from <dag->no_nodes) && (m->to <dag->no_nodes)) {
    if (dag->node_status[m->to]==1) { // if base node
      initial_messages.push_back(new Message(m));
      return;
    }
    // add assignment to appropriate bdd:
    // assignment is and of variables
    DdNode *dis, *var, *tmp, *tmp2;
    dis = Cudd_ReadOne(ddmgr);
    Cudd_Ref(dis);
    // deleting the variables that are not on this arc is already done in loops.cpp
    // so no need to check anything here
    for (int i=0; i<m->assignments.size(); i++) {
      if (m->assignments[i]<0) {
        var = Cudd_Not(Cudd_bddIthVar(ddmgr, -m->assignments[i]));
        Cudd_Ref(var);
      }
      else {
        var = Cudd_bddIthVar(ddmgr, m->assignments[i]);
        Cudd_Ref(var);
      }
      tmp = Cudd_bddAnd(ddmgr, dis, var);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(ddmgr, dis);
      Cudd_RecursiveDeref(ddmgr, var);
      dis = tmp;
    }
    // now OR this new dd to the m->to, m->from bdd
    tmp2 = Cudd_bddOr(ddmgr, bdds[m->to][m->from], dis);
    Cudd_Ref(tmp2);
    Cudd_RecursiveDeref(ddmgr, dis);
    Cudd_RecursiveDeref(ddmgr, bdds[m->to][m->from]);
    bdds[m->to][m->from] = tmp2;
  } else {
    throw BadParameterException("master adding message that doesn't make sense");
  }
}

void BDDSolutions::readd_message(Message* m) {
  // add a message back to master's memory, for example if a worker needs to stop
  // work on a given problem but has not found all solutions

  // if the messages is empty, readd to initial_messages
  if (m->assignments.size() == 0) {
    initial_messages.push_back(m);
    return;
  } else {
    // need to remove assignment from communicated bdd
    // first AND all assignments in message 
    // - assume all variables in message are in interface, assume checking already done (like in add_message)
    DdNode *var, *dis, *tmp, *tmp2;
    dis = Cudd_ReadOne(ddmgr);
    Cudd_Ref(dis);
    for (auto it = m->assignments.begin(); it != m->assignments.end(); it++) {
      if (*it < 0) {
        var = Cudd_Not(Cudd_bddIthVar(ddmgr, -*it));
        Cudd_Ref(var);
      } else {
        var = Cudd_bddIthVar(ddmgr, *it);
        Cudd_Ref(var);
      }
      tmp = Cudd_bddAnd(ddmgr, dis, var);
      Cudd_RecursiveDeref(ddmgr, dis);
      Cudd_RecursiveDeref(ddmgr, var);
      dis = tmp;
    }
    // now AND NOT this new assignment diagram to communicated m->from bdd
    tmp2 = Cudd_bddAnd(ddmgr, communicated[m->to], Cudd_Not(dis));
    Cudd_Ref(tmp2);
    Cudd_RecursiveDeref(ddmgr, dis);
    Cudd_RecursiveDeref(ddmgr, communicated[m->to]);
    communicated[m->to] = tmp2;
  }
}

Message *BDDSolutions::get_new_message_combination(int depth) {
  VLOG(5) << "MASTER: get_new_message_combination: getting new message combination at depth " << depth << std::endl;
  if ( depth < 0 || depth > dag->max_depth) {
    throw BadParameterException("master called get_new_message combination with bad depth");
  }
  
  // check if there are initial messages to send
  if (!initial_messages.empty()) {
    for (int j=0; j<initial_messages.size(); j++) {
      if (dag->depth_for_each_node[initial_messages[j]->to] == depth) {
        Message* mess = initial_messages[j];
        initial_messages.erase(initial_messages.begin()+j);
        VLOG(5) << "MASTER: get_new_message_combination: returning initial message " << *mess;
        return mess;
      }
    }
  }
  
  for (int i=0; i<dag->no_nodes; i++) { // for all nodes (index) $i$ in the graph
    
    // check that the node is at the right \argument{depth}
    if (dag->depth_for_each_node[i] != depth) {
      continue;
    }

    // node cannot be a leave/base node - these are dealt with by initial_messages
    if (dag->node_status[i]/*is_leaf*/ == 1) continue; 

    VLOG(5) << "MASTER: get_new_message_combination: inspecting node " << i;

    // try to find an assignment that has not been communicated yet
    
    // AND all the bdds to the node - look at reverse connections in dag
    DdNode* tmp;
    DdNode* full_bdd = Cudd_ReadOne(ddmgr);
    Cudd_Ref(full_bdd);

    for(auto child = dag->reverse_connections[i].begin()
	  ; child != dag->reverse_connections[i].end()
	  ; child++) {

      // a node is always a reverse connection of itself, don't want to AND this too
      if (i == *child)  continue;
      if (bdds[i][*child] == Cudd_ReadLogicZero(ddmgr)) {
        VLOG(5) << "MASTER: get_new_message_combination: bdd to " << i << " from " << *child << " is the zero bdd";
      }
      tmp = Cudd_bddAnd(ddmgr, full_bdd, bdds[i][*child]);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(ddmgr, full_bdd);
      full_bdd = tmp;
    }
    
    if (full_bdd == Cudd_ReadLogicZero(ddmgr)) {
      VLOG(5) << "MASTER: get_new_message_combination: full bdd is zero";
    }
    
    DdNode* full_not_communicated  = Cudd_bddAnd(ddmgr, full_bdd, Cudd_Not(communicated[i]));
    Cudd_Ref(full_not_communicated);
    Cudd_RecursiveDeref(ddmgr, full_bdd);

    int* cube;
    int numvars = Cudd_ReadSize(ddmgr);
    CUDD_VALUE_TYPE value;
    DdGen* gen = Cudd_FirstCube(ddmgr, full_not_communicated, &cube, &value);
    // cube now points to the cube array in gen (don't free cube, only free gen
    // after using cube)
    if (Cudd_IsGenEmpty(gen)) {
      VLOG(5) << "MASTER: get_new_message_combination: master continuing since gen is empty at node " << i;
      Cudd_GenFree(gen);
      continue;
    }
    
    // create a new message to return
    VLOG(5) << "MASTER: get_new_message_combination: found assignment for node " << i << ", adding it to communicated BDD and creating message";
    Message* m = new Message(i, i);
    // add message to communicated and put the literals from 
    // the cube into the message's assignments
    // only the appropriate variables are in the bdds (checking done in loops.cpp)
    // so communicated will also only have the right variables, no need to check here
    DdNode* tmp2;
    DdNode* var;
    DdNode* fresh_comm = Cudd_ReadOne(ddmgr);
    Cudd_Ref(fresh_comm);
    for (int j=1; j<numvars; j++) {

      if (cube[j]==2) {
        continue;
      }
      else if (cube[j]==1) {
        m->assignments.push_back(j);
        var = Cudd_bddIthVar(ddmgr, j);
      } else if (cube[j]==0) {
        m->assignments.push_back(-j);
        var = Cudd_Not(Cudd_bddIthVar(ddmgr, j));
      } else {
        continue;
      }
      tmp = Cudd_bddAnd(ddmgr, fresh_comm, var);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(ddmgr, fresh_comm);
      fresh_comm = tmp;
    }
    tmp2 = Cudd_bddOr(ddmgr, communicated[i], fresh_comm);
    Cudd_Ref(tmp2);
    Cudd_RecursiveDeref(ddmgr, fresh_comm);
    Cudd_RecursiveDeref(ddmgr, communicated[i]);
    communicated[i] = tmp2;
    Cudd_GenFree(gen);
    VLOG(5) << "MASTER: get_new_message_combination: returning message " << *m;
    VLOG(3) << "MASTER: get_new_message_combination: new message with " << m->assignments.size() << " assignments";
    CHECK_LE(m->assignments.size(), vc + dag->max_depth + 5); // check that the assignment message is smaller than max_message_length
    return m;    
  }
  VLOG(5) << "MASTER: get_new_message_combination: exhausted all nodes, returning NULL";
  return NULL;
}


// memory use and node statistcs - total for all decision diagrams
void BDDSolutions::print_stats(bool print_all_bdds, bool full) {
  VLOG(1) << "MASTER: number of nodes in each bdd:";
  // get number of nodes in each bdd and completed
  std::vector<std::vector<int>> bdd_nodes;
  std::vector<int> communicated_nodes;
  std::vector<int> completed_nodes;
  bdd_nodes.resize(dag->no_nodes);
  completed_nodes.clear();
  for (int i=0; i<dag->no_nodes; i++) {
    communicated_nodes.push_back(Cudd_DagSize(communicated[i]));
    completed_nodes.push_back(Cudd_DagSize(completed[i]));
    for (int j=0; j<dag->no_nodes; j++) {
      bdd_nodes[i].push_back(Cudd_DagSize(bdds[i][j]));
    }
  }
  // print the number of nodes in each bdd
  for (int i=0; i<dag->no_nodes; i++) {
    for (int j=0; j<dag->no_nodes; j++) {
      if (std::find(dag->forward_connections[j].begin(),
                    dag->forward_connections[j].end(),
                    i) != dag->forward_connections[j].end()) {
        VLOG(1) << j << "->" << i << ": " << bdd_nodes[i][j] << " nodes ";
      }
    }
  }
  // and for the communicated diagrams
   for (int i=0; i<dag->no_nodes; i++) {
    VLOG(1) << communicated_nodes[i] << " nodes in communicated bdd " << i;
  } 
  // now for the completed diagrams
  for (int i=0; i<dag->no_nodes; i++) {
    VLOG(1) << completed_nodes[i] << " nodes in completed bdd " << i;
  }
  if (print_all_bdds) {
    for (int i=0; i<dag->no_nodes; i++) {
      // print the bdd and completed diagram for each connection
      std::string name = "bdds";
      name += std::to_string(i);
      name += ".dot";
      FILE* fp;
      TEST_NOT_NULL(fp = fopen(name.c_str(), "w"))
      print_bdds(fp, i);
      fclose(fp);
      std::string name1 = "completed";
      name1 += std::to_string(i);
      name1 += ".dot";
      FILE* fp1;
      TEST_NOT_NULL(fp1 = fopen(name1.c_str(), "w"))
      print_completed(fp1, i);
      fclose(fp1);
      std::string name2 = "communicated";
      name2 += std::to_string(i);
      name2 += ".dot";
      FILE* fp2;
      TEST_NOT_NULL(fp2 = fopen(name2.c_str(), "w"))
      print_communicated(fp2, i);
      fclose(fp2);
    }
  }

  if (full) {
    // number of nodes stats
    long peak_nodes = Cudd_ReadPeakNodeCount(ddmgr);
    int peak_live_nodes = Cudd_ReadPeakLiveNodeCount(ddmgr);
    long current_nodes = Cudd_ReadNodeCount(ddmgr);
    int num_vars =  Cudd_ReadSize(ddmgr);
    // memory usage stats
    size_t current_memory = Cudd_ReadMemoryInUse(ddmgr);
    size_t max_memory = Cudd_ReadMaxMemory(ddmgr);
    
    // cache usage stats
    unsigned int max_cache = Cudd_ReadMaxCache(ddmgr);
    unsigned int cache_slots = Cudd_ReadCacheSlots(ddmgr);
    double cache_hits = Cudd_ReadCacheHits(ddmgr);
    double cache_lookups = Cudd_ReadCacheLookUps(ddmgr);
    
    VLOG(1) << "------------------------------------" << std::endl;
    VLOG(1) << "\t Cudd memory use" << std::endl;
    VLOG(1) << "Current memory used by Cudd: " << current_memory << std::endl;
    VLOG(1) << "Max possible memory for Cudd: " << max_memory << std::endl;
    VLOG(1) << "\t -------------" << std::endl;
    VLOG(1) << "\t Number of Cudd decision diagram nodes" << std::endl;
    VLOG(1) << "Total number of variables in all diagrams: " << num_vars << std::endl;
    VLOG(1) << "Number of current nodes: " << current_nodes << std::endl;
    VLOG(1) << "Peak number of nodes: " << peak_nodes << std::endl;
    VLOG(1) << "Peak number of live nodes: " << peak_live_nodes << std::endl;
    VLOG(1) << "\t -------------" << std::endl;
    VLOG(1) << "\t Cudd cache usage stats" << std::endl;
    VLOG(1) << "Cudd max cache size: " << max_cache << std::endl;
    VLOG(1) << "Cudd number of cache slots: "<< cache_slots << std::endl;
    VLOG(1) << "Cudd number of cache hits: " << cache_hits 
              << " out of " << cache_lookups << " lookups" << std::endl;
    VLOG(1) << "------------------------------------" << std::endl;
  }
}

void BDDSolutions::print_bdds(FILE* fp, int to) {
  DdNode** add = new DdNode*[dag->no_nodes];
  for (int i=0; i<dag->no_nodes; i++) {
    add[i] = Cudd_BddToAdd(ddmgr, bdds[to][i]); 
  }
  Cudd_DumpDot(ddmgr, dag->no_nodes, add, NULL, NULL, fp);
  delete[] add;
}

void BDDSolutions::print_completed(FILE* fp, int node) {
  DdNode** add = new DdNode*[1];
  add[0] = Cudd_BddToAdd(ddmgr, completed[node]);
  Cudd_DumpDot(ddmgr, 1, add, NULL, NULL, fp);
  delete[] add;
}

void BDDSolutions::print_completed_bdd(FILE* fp, int node) {
  DdNode** bdd = new DdNode*[1];
  bdd[0] = Cudd_BddToAdd(ddmgr, Cudd_Not(completed[node]));
  Cudd_DumpDot(ddmgr, 1, bdd, NULL, NULL, fp);
  delete[] bdd;
}

void BDDSolutions::print_communicated(FILE* fp, int node) {
  DdNode** add = new DdNode*[1];
  add[0] = Cudd_BddToAdd(ddmgr, communicated[node]);
  Cudd_DumpDot(ddmgr, 1, add, NULL, NULL, fp);
  delete[] add;
}

//dump_checkpoint: (DRAFT)
// dump all BDD information to open writable file pointer
// suitable for files subsequently loaded by load_checkpoint() method
void BDDSolutions::dump_checkpoint(FILE* fp) {
	#if __has_include("dddmp.h")
	fprintf(fp,"124349 ");
	Dddmp_VarInfoType varoutinfo = DDDMP_VARIDS;
	fprintf(fp,"%i %i %i ", initial_messages.size(),this->dag->no_nodes,this->vc);
	for (auto it = initial_messages.begin(); it!=initial_messages.end(); it++) {
		(*it)->dump_to_file(fp);
	}
	fprintf(fp,"\n");
	for (int i=0; i< this->dag->no_nodes; i++) {
		for (int j=0; j< this->dag->no_nodes; j++) {
			int status = Dddmp_cuddBddStore(ddmgr, NULL, this->bdds[i][j], NULL, NULL, DDDMP_MODE_TEXT, varoutinfo, NULL, fp);
			fprintf(fp,"\n");
		}
	}
	for (int i=0; i< this->dag->no_nodes; i++) {
		int status = Dddmp_cuddBddStore(ddmgr, NULL, this->communicated[i], NULL, NULL, DDDMP_MODE_TEXT, varoutinfo, NULL, fp);
		fprintf(fp,"\n");
	}
	for (int i=0; i< this->dag->no_nodes; i++) {
		int status = Dddmp_cuddBddStore(ddmgr, NULL, this->completed[i], NULL, NULL, DDDMP_MODE_TEXT, varoutinfo, NULL, fp);
		fprintf(fp,"\n");
	}
	#else
    LOG(ERROR) << "MASTER: Cannot load/save BDD checkpoint without dddmp, cudd sublibrary installed";
    exit(1);
	#endif
}

//load_checkpoint: (DRAFT)
// load all BDD information from a readable file pointer
// suitable for files written by dump_checkpoint() method
void BDDSolutions::load_checkpoint(FILE* fp) {
	#if __has_include("dddmp.h")
	int identifier;
	int reads;
	reads = fscanf(fp, "%i ", &identifier);
	CHECK_EQ(reads,1);
	if (identifier != 124349) {
		throw BadParameterException("Cannot load checkpoint not created using BDDSolutions interface into BDDSolutions interface class");
	}
	
	Dddmp_VarMatchType varmatchtype = DDDMP_VAR_MATCHIDS;
	initial_messages.clear();
	for (int i=0; i<dag->no_nodes; i++) {
		Cudd_RecursiveDeref(ddmgr, communicated[i]);
		Cudd_RecursiveDeref(ddmgr, completed[i]);
		for (int j=0; j<dag->no_nodes; j++) {
			Cudd_RecursiveDeref(ddmgr, bdds[i][j]);
		}
	}
	int initial_message_size;
	int dag_no_nodes;
	int vvc;
	reads = fscanf(fp, "%i %i %i ", &initial_message_size, &dag_no_nodes, &vvc);
	CHECK_EQ(reads,3);
	CHECK_EQ(dag_no_nodes, this->dag->no_nodes);
	CHECK_EQ(vvc,this->vc);
	for (int i=0; i<initial_message_size; i++) {
		Message *m = new Message;
		m->read_from_file(fp);
		initial_messages.push_back(m);
	}
	for (int i=0; i< this->dag->no_nodes; i++) {
		for (int j=0; j< this->dag->no_nodes; j++) {
			this->bdds[i][j] = Dddmp_cuddBddLoad(ddmgr, varmatchtype, NULL, NULL, NULL, DDDMP_MODE_TEXT, NULL, fp);
		}
	}
	for (int i=0; i< this->dag->no_nodes; i++) {
		this->communicated[i] = Dddmp_cuddBddLoad(ddmgr, varmatchtype, NULL, NULL, NULL, DDDMP_MODE_TEXT, NULL, fp);
	}
	for (int i=0; i< this->dag->no_nodes; i++) {
		this->completed[i] = Dddmp_cuddBddLoad(ddmgr, varmatchtype, NULL, NULL, NULL, DDDMP_MODE_TEXT, NULL, fp);
	}
	#else
    LOG(ERROR) << "MASTER: Cannot load/save BDD checkpoint without dddmp, cudd sublibrary installed";
	exit(1);
	#endif
}


// return a cnf representation of the negation of index bdd
int** BDDSolutions::compile_negated_cnf(int node) {
  int** negated_rep = NULL;
  // want to get a cnf of the negated bdd
  // we do this by extracting  all the paths to 0 (in the negated bdd), and then not each 
  // variable in the path. not of a bdd only interchanges the leaves, so finding a path to 
  // zero of the not bdd is the  same as finding a path to one in the original bdd,
  // which is called a cube in cudd
  int numvars = Cudd_ReadSize(ddmgr);
  int* cube;
  int num_cubes = 2;  // reserve first element for var count and clauses count,
                      // and the second for the variable map
  int vars = 1;
  std::vector<int> var_map; // map from var number in cnf to var number in master cnf
  DdNode* bdd = completed[node];
  // int vc = Cudd_SupportSize(ddmgr, bdd);
  CUDD_VALUE_TYPE value;
  DdGen* gen;
  Cudd_ForeachCube(ddmgr, bdd, gen, cube, value) {
    TEST_NOT_NULL(negated_rep = (int**)realloc(negated_rep, sizeof(int*)*(num_cubes+2)))
    // convert the cube into cnf - negate each element where each element in cube is 0 (false) 1 (true)
    // or 2 (don't care)
    TEST_NOT_NULL(negated_rep[num_cubes] = (int*)calloc(numvars+2,sizeof(int)))
    int index=0;
    for (int i=1; i<numvars; i++) {
      if (cube[i]==2) continue;
      auto it = std::find(var_map.begin(), var_map.end(), i);
      if (cube[i]==0) {
        if (it != var_map.end()) {
          negated_rep[num_cubes][index] = std::distance(var_map.begin(), it) + 1;
        } else {
          negated_rep[num_cubes][index] = vars;
          var_map.push_back(i);
          vars++;
        }
        index++;
      }
      else if (cube[i]==1) {
        if (it != var_map.end()) {
          negated_rep[num_cubes][index] = -std::distance(var_map.begin(), it) - 1;
        } else {
          negated_rep[num_cubes][index] = -vars;
          var_map.push_back(i);
          vars++;
        }
        index++;
      }
    }
    num_cubes++;
  }
  if (negated_rep) {
    negated_rep[num_cubes] = NULL;
    // add var count and clause count
    TEST_NOT_NULL(negated_rep[0] = (int*)calloc(3, sizeof(int)))
    negated_rep[0][0] = vars - 1;
    negated_rep[0][1] = num_cubes - 1;
    // quick hack to this to work with new interface
    TEST_NOT_NULL(negated_rep[1] = (int*)calloc(var_map.size() + 2, sizeof(int)))
    for (int i=0; i<var_map.size(); i++) {
      negated_rep[1][i+1] = var_map[i];
    }
  }
  
  return negated_rep;
}

void BDDSolutions::reorder_bdds() {
  Cudd_ReduceHeap(ddmgr, CUDD_REORDER_SIFT, 3000);
}

int** BDDSolutions::encode_negated_cnf(int node) {
  DdNode* tmp1 = Cudd_Not(completed[node]);
  DdNode* tmp = Cudd_BddToAdd(ddmgr, tmp1);
  Cudd_Ref(tmp);
  int** clauses = NULL;
  int cc = 2; // clause count+2, use the first 2 slots to store info other than clauses
    
  // for each non terminal node v=(x,t,f) generate a Boolean variable v which represents
  // the truth value of the BDD rooted at v
  
  int* support = NULL;
  int support_size = Cudd_SupportIndices(ddmgr, tmp, &support);
  // support is an array with length equal to support size that has
  // the indices of the variables in the support of bdd
  if (support_size == 0) { // there are no nodes in the bdd
    return NULL;
  }
  CHECK_NE(support_size, -1) << "CUDD out of memory from support size";

  int vars[vc+1];                 // maps each variable to its index in the cnf
  std::unordered_map<DdNode*, int> nodes; // maps each node to its index in the cnf
  int numvars = 0;

  TEST_NOT_NULL(clauses = (int**)realloc(clauses, (cc+2)*sizeof(int*)))

  // map these variables in cnf variables starting at 1
  // also use clauses[1] to map cnf varnumber to master var number
  TEST_NOT_NULL(clauses[1] = (int*)calloc(support_size+2, sizeof(int)))
  for (int i=0; i<support_size; i++) {
    // only interested if the variable is in the support of the bdd
    vars[support[i]] = ++numvars;
    clauses[1][numvars] = support[i];
  }

  // start the auxillary variables at the variable count of the master cnf
  int numnodes = vc + 1;
  VLOG(3) << "MASTER: encode negated cnf: there are " << numvars << " total variables in the support of bdd";

  // iterate through all nodes (including the 1 terminal node) to add variables
  DdGen* gen;
  DdNode* dd;
  Cudd_ForeachNode(ddmgr, tmp, gen, dd) {
    // add the node
    VLOG(5) << "MASTER: encode negated cnf: adding node for variable " << (Cudd_IsConstant(dd) ? "logical " : "") 
    << (Cudd_IsConstant(dd) ? Cudd_V(dd) : Cudd_NodeReadIndex(dd));
    nodes[dd] = ++numnodes;
  }
  VLOG(3) << "MASTER: encode negated cnf: there are " << numnodes - vc - 1 << " nodes in the bdd";

  // store the variable and clause count at the start
  TEST_NOT_NULL(clauses[0] = (int*)calloc(3, sizeof(int)))
  clauses[0][0] = numnodes;

  // add constraint that the root (which is the last node expanded) is true
  TEST_NOT_NULL(clauses[cc] = (int*)calloc(2, sizeof(int)))
  clauses[cc][0] = numnodes;
  cc++;

  // now add the other constraints
  VLOG(5) << "MASTER: encode negated cnf: adding constraints for each node";
  DdGen* gen1;
  Cudd_ForeachNode(ddmgr, tmp, gen1, dd) {
    // continue if dd is constant (must be 1 or not 1)
    if (Cudd_IsConstant(dd)) {
      // and add clause that 1 is true 
      // also add clause that 0 is false - using cudd adds
      TEST_NOT_NULL(clauses = (int**)realloc(clauses, (cc+2)*sizeof(int*)))
      TEST_NOT_NULL(clauses[cc] = (int*)calloc(2, sizeof(int)))
      if (Cudd_V(dd) == 1) {
        clauses[cc][0] = nodes[dd];
        cc++;
      } else {
        clauses[cc][0] = -nodes[dd];
        cc++;
      }
    } else {
      // first allocate the memory
      TEST_NOT_NULL(clauses = (int**)realloc(clauses, (cc+7)*sizeof(int*)))
      for (int i=0; i<6; i++) {
        TEST_NOT_NULL(clauses[cc+i] = (int*)calloc(4, sizeof(int)))
      }

      // B1: t and x implies v. ie not t or not x or v
      clauses[cc][0] = -nodes[Cudd_T(dd)];
      clauses[cc][1] = -vars[Cudd_NodeReadIndex(dd)];
      clauses[cc][2] = nodes[dd];
      cc++;
      
      // B2: not t and x implies not v. ie t or not x or not v
      clauses[cc][0] = nodes[Cudd_T(dd)];
      clauses[cc][1] = -vars[Cudd_NodeReadIndex(dd)];
      clauses[cc][2] = -nodes[dd];
      cc++;

      // B3: f and not x implies v. ie not f or x or v
      clauses[cc][0] = -nodes[Cudd_Regular(Cudd_E(dd))];
      clauses[cc][1] = vars[Cudd_NodeReadIndex(dd)];
      clauses[cc][2] = nodes[dd];
      cc++;

      // B4: not f and not x implies not v. ie f or x or not v
      clauses[cc][0] = nodes[Cudd_Regular(Cudd_E(dd))];
      clauses[cc][1] = vars[Cudd_NodeReadIndex(dd)];
      clauses[cc][2] = -nodes[dd];
      cc++;

      // B5: t and f implies v. ie not t or not f or v
      clauses[cc][0] = -nodes[Cudd_T(dd)];
      clauses[cc][1] = -nodes[Cudd_Regular(Cudd_E(dd))];
      clauses[cc][2] = nodes[dd];
      cc++;

      // B6: not t and not f implies not v. ie t or f or not v
      clauses[cc][0] = nodes[Cudd_T(dd)];
      clauses[cc][1] = nodes[Cudd_Regular(Cudd_E(dd))];
      clauses[cc][2] = -nodes[dd];
      cc++;
    }
  }
  VLOG(5) << "MASTER: encode negated cnf: all constraints added, now clean up and return clauses";
  clauses[cc] = NULL;
  // store the variable and clause count at the start
  clauses[0][1] = cc - 1;

  // clean up
  Cudd_RecursiveDeref(ddmgr, tmp);
  free(support);
  nodes.clear();
  return clauses;
}

using namespace std;

// first create variables for each node and edge
// - need to record more info than directly available in Cudd
// we need to record the incoming and outgoing edges for a node
// and for an edge the from and to nodes and what variable is true/false
struct Node;
struct Edge {
  Node* from;        // contains the variable
  Node* to; 
  bool var_value;    // whether var is true or false

  Edge(Node* node_from, Node* node_to, bool value)
    : from(node_from)
    , to(node_to)
    , var_value(value) 
  { }

  ~Edge(){
    from = NULL;
    to = NULL;
  }
};

struct Node {
  DdNode* ddnode;
  Edge* T;
  Edge* E;
  set<Edge*> incoming_edges;

  Node(DdNode* n, Edge* then_edge, Edge* else_edge)
  : ddnode(n)
  , T(then_edge)
  , E(else_edge)
  { }

  ~Node() {
    ddnode = NULL;
    delete T;
    T = NULL;
    delete E;
    E = NULL;
    incoming_edges.clear();
  }
};

// now implement a path-based encoding of the bdd that is propagation complete
int** BDDSolutions::path_based_encode_negated_cnf(int node) {
  DdNode* tmp1 = Cudd_Not(completed[node]); // the bdd we are interested in
  DdNode* tmp = Cudd_BddToAdd(ddmgr, tmp1);
  Cudd_Ref(tmp);

  int** clauses = NULL;
  int cc = 2; // clause count+2, store other info in first two int*

  // only interested in the variables that are in the support of tmp
  VLOG(5) << "MASTER: path encode negated cnf adding variables in support";
  int* support = NULL;
  int support_size = Cudd_SupportIndices(ddmgr, tmp, &support);
  // support is an array with length equal to support size that has
  // the indices of the variables in the support of bdd
  if (support_size == 0) { // there are no nodes in the bdd
    return NULL;
  }
  int vars[vc+1];            // maps each variable to its index in the cnf
  unordered_map<Node*, int> nodes;   // maps each node to its index in the cnf
  unordered_map<Edge*, int> edges;   // maps each edge to its index in the cnf
  int numvars = 0;
  
  TEST_NOT_NULL(clauses = (int**)realloc(clauses, 3 * sizeof(int*)))


  // map these variables in cnf variables starting at 1
  // also create var map to return (maps var number in cnf to var number in master cnf)
  TEST_NOT_NULL(clauses[1] = (int*)calloc(support_size + 2, sizeof(int)))
  for (int i=0; i<support_size; i++) {
    // only interested if the variable is in the support of the bdd
    vars[support[i]] = ++numvars;
    clauses[1][numvars] = support[i];
  }

  // start the auxilary variables from greater than the variable count of the master cnf
  int num_auxvars = vc + 1;

  // go through the diagram and make the edges and nodes and make incoming_edges
  VLOG(5) << "MASTER: path encode negated cnf adding nodes and edges";
  DdGen* gen;
  DdNode* dd;
  // Note: the iteration in cudd is done so that we have already seen a nodes children
  // Note: each dd is already regular
  Cudd_ForeachNode(ddmgr, tmp, gen, dd) {
    if (Cudd_IsConstant(dd)) {
      Node* node = new Node(dd, NULL, NULL);
      nodes[node] = ++num_auxvars;
      // and add clause that 1 is true and that 0 is false
      TEST_NOT_NULL(clauses = (int**)realloc(clauses, (5)*sizeof(int*)))
      TEST_NOT_NULL(clauses[cc] = (int*)calloc(2, sizeof(int)))
      if (Cudd_V(dd) == 1) {
        clauses[cc][0] = nodes[node];
        cc++;
      } else {
        clauses[cc][0] = -nodes[node];
        cc++;
      }
    } else {
      Node* node = new Node(dd, NULL, NULL);
      // want to find child nodes - guarenteed by CUDD to already be in nodes map     
      Node* node_T;
      Node* node_E;
      bool found_T = false;
      bool found_E = false;
      auto tmp_node = nodes.begin();
      do {
        if (tmp_node->first->ddnode == Cudd_T(dd)) {
          node_T = tmp_node->first;
          found_T = true;
        } else if (tmp_node->first->ddnode == Cudd_Regular(Cudd_E(dd))) {
          node_E = tmp_node->first;
          found_E = true;
        }
        tmp_node++;
      } while (!found_T || !found_E);
      
      Edge* edge_T = new Edge(node, node_T, true);
      edges[edge_T] = ++num_auxvars;
      node->T = edge_T;
      Edge* edge_E = new Edge(node, node_E, false);
      edges[edge_E] = ++num_auxvars;
      node->E = edge_E;
      nodes[node] = ++num_auxvars;
      // add incoming edges to the nodes children
      node->T->to->incoming_edges.insert(edge_T);
      node->E->to->incoming_edges.insert(edge_E);
    }
  }
  VLOG(5) << "MASTER: path encode negated cnf: there are " << num_auxvars - vc - 1 << " auxillary variables (" 
          << nodes.size() << " nodes and " << edges.size() << " edges) in the bdd";

  // now add the clauses
  
  // store the variable and clause count at the start
  TEST_NOT_NULL(clauses[0] = (int*)calloc(3, sizeof(int)))
  clauses[0][0] = num_auxvars;

  // we add constraints for each node
  VLOG(5) << "MASTER: path encode negated cnf adding constraints for each node";
  for (pair<Node*, int> elem : nodes) {
    if (Cudd_IsConstant(elem.first->ddnode)) {
      continue; // already dealt with clause for 1
    }
    TEST_NOT_NULL(clauses = (int**)realloc(clauses, (cc+6) * sizeof(int*)))
    

    // T1: node implies then edge or else edge. ie not node or then edge or else edge
    TEST_NOT_NULL(clauses[cc] = (int*)calloc(4, sizeof(int)))
    clauses[cc][0] = -elem.second;
    clauses[cc][1] = edges[elem.first->T];
    clauses[cc][2] = edges[elem.first->E];
    cc++;
    
    // P1: node and value of var at node implies edge from node with that value
    // ie not node or not node value or edge with value
    // do var is true case first - then edge
    TEST_NOT_NULL(clauses[cc] = (int*)calloc(4,sizeof(int)))
    clauses[cc][0] = -elem.second;
    clauses[cc][1] = -vars[Cudd_NodeReadIndex(elem.first->ddnode)];
    clauses[cc][2] = edges[elem.first->T];
    cc++;
    // var is false - else edge
    TEST_NOT_NULL(clauses[cc] = (int*)calloc(4, sizeof(int)))
    clauses[cc][0] = -elem.second;
    clauses[cc][1] = vars[Cudd_NodeReadIndex(elem.first->ddnode)];
    clauses[cc][2] = edges[elem.first->E];
    cc++;
   
    // the last clause is different if the node is root
    int num_incoming_edges = elem.first->incoming_edges.size();
    if (num_incoming_edges == 0) {
      // add constraint that the root is true
      TEST_NOT_NULL(clauses[cc] = (int*)calloc(2, sizeof(int)))
      clauses[cc][0] = elem.second;
      cc++;
    } else {  // only if node is not root
      // P2: node implies one of the incoming edges. ie not node or an incoming edge
      TEST_NOT_NULL(clauses[cc] = (int*)calloc(2 + num_incoming_edges, sizeof(int)))
      clauses[cc][0] = -elem.second;
      int index = 1;
      for (auto edge : elem.first->incoming_edges) {
        clauses[cc][index] = edges[edge];
        index++;
      }
      cc++;
    }
  }

  // add constraints for each edge
  VLOG(5) << "MASTER: path encode negated cnf adding constraints for each edge";
  for (pair<Edge*, int> elem : edges) {
    TEST_NOT_NULL(clauses = (int**)realloc(clauses, (cc+4) * sizeof(int*)))

    // T2: edge implies edge->node from. ie not edge or node from
    TEST_NOT_NULL(clauses[cc] = (int*)calloc(3, sizeof(int)))
    clauses[cc][0] = -elem.second;
    clauses[cc][1] = nodes[elem.first->from];
    cc++;

    // T3: edge implies edge->node to. ie not edge or node to
    TEST_NOT_NULL(clauses[cc] = (int*)calloc(3, sizeof(int)))
    clauses[cc][0] = -elem.second;
    clauses[cc][1] = nodes[elem.first->to];
    cc++;

    // T4: edge implies var is the value on the edge. 
    // ie not edge or var is the value described by edge
    TEST_NOT_NULL(clauses[cc] = (int*)calloc(3, sizeof(int)))
    clauses[cc][0] = -elem.second;
    clauses[cc][1] = elem.first->var_value ? vars[Cudd_NodeReadIndex(elem.first->from->ddnode)]
                                           : -vars[Cudd_NodeReadIndex(elem.first->from->ddnode)];
    cc++;
  }

  // Note: there are other constraints in the paper, but these don't make sense with CUDD
  // (for example constraints like exactly one node on each level, but paths can skip a level in CUDD)

  // add clause count
  clauses[0][1] = cc - 1;
  // clean up data:
  VLOG(5) << "MASTER: path encode negated cnf cleaning up data";
  Cudd_RecursiveDeref(ddmgr, tmp);
  free(support);
  nodes.clear();
  edges.clear();
  clauses[cc] = NULL;
  return clauses;
}

// print the clauses, assumed to be null terminated
// and that the first int* has the variable count and clause count
void BDDSolutions::print_cnf(int** additional_clauses, std::string fname) {
  std::ofstream file;
  int** clauses = additional_clauses;
  int* var_map  = additional_clauses[1];
  file.open(fname);
  if (file.is_open()) {
    // make the header
    int vc = clauses[0][0];
    int cc = clauses[0][1];
    // print the header
    VLOG(5) << "MASTER: print cnf printing header";
    file << "p cnf " << vc << " " << cc << std::endl;
    // print the clauses
    for (int i=2; clauses[i]; i++) {
      VLOG(5) << "MASTER: print cnf printing clause " << i;
      for (int j=0; clauses[i][j]; j++) {
        file << clauses[i][j] << " ";
      }
      file << "0\n";
    }
    // print the mapping line at the end (also using the dag variable_map)
    if (var_map) {
      for (int i=0; var_map[i]; i++) {
        file << var_map[i] << " ";
      }
      file << std::endl;
    }
    file.close();
  } else {
    LOG(ERROR) << "MASTER: print cnf could not open file";
  }
}

void BDDSolutions::print_three_negated_cnfs(int node, int num_models) {
  int** neg_cnf_cubes = compile_negated_cnf(node);
  int** neg_cnf_minisat = encode_negated_cnf(node);
  int** neg_cnf_completepath = path_based_encode_negated_cnf(node);

  // print the cnf files
  string fname = "bdd_cnf_files/cubes_";
  fname += to_string(node);
  fname += "_";
  fname += to_string(num_models);
  fname += ".cnf";
  print_cnf(neg_cnf_cubes, fname);

  fname = "bdd_cnf_files/minisat_";
  fname += to_string(node);
  fname += "_";
  fname += to_string(num_models);
  fname += ".cnf";
  print_cnf(neg_cnf_minisat, fname);

  fname = "bdd_cnf_files/completepath_";
  fname += to_string(node);
  fname += "_";
  fname += to_string(num_models);
  fname += ".cnf";
  print_cnf(neg_cnf_completepath, fname);

  // clean up
  fname.erase();
  if (neg_cnf_cubes) {
    for (int i=0; neg_cnf_cubes[i]; i++) {
      free(neg_cnf_cubes[i]);
    }
    free(neg_cnf_cubes);
  }
  if (neg_cnf_minisat) {
    for (int i=0; neg_cnf_minisat[i]; i++) {
      free(neg_cnf_minisat[i]);
    }
    free(neg_cnf_minisat);
  }
  if (neg_cnf_completepath) {
    for (int i=0; neg_cnf_completepath[i]; i++) {
      free(neg_cnf_completepath[i]);
    }
    free(neg_cnf_completepath);
  }
}
