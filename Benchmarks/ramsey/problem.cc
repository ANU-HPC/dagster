/*************************
Copyright 2020 Charles Gretton

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

#include "problem.hh"
#include <fstream>
#include <ostream>


int Triangle::ID = 0 ;

void Problem::build_all_triangles(){
  assert(M);
  for(auto x = 0 ; x < M; x++){
    for(auto y = x ; y < M; y++){
      for(auto z = y ; z < M; z++){
	auto triangle = Triangle(x,y,z);
	if (x == z && x == y){
	  cerr<<"MONOCHROMATIC TRIANGLE :: "<<triangle<<endl;
	  monochromatic_triangles.insert(triangle);
	} else{
	  nonmonochromatic_triangles.insert(triangle);
	  cerr<<"NONMONOCHROMATIC TRIANGLE :: "<<triangle<<endl;
	}
	triangles.insert(triangle);
      }
    }
  }
}

Triangle::Triangle(int a, int b, int c)
    :id(ID++)
{
  push_back(a);
  push_back(b);
  push_back(c);
  
  vector<int> tmp = *this;
  do {
    permutations.insert(tmp);
  } while (std::next_permutation(tmp.begin(), tmp.end()));
}

const set<vector<int> >& Triangle::get__permutations() const{return permutations;}
int Triangle::get__id() const{return id;}


ostream& operator<<(ostream& o, const Triangle& t){
  o<<"["<<t.id<<"] ";
  for ( auto c : t ){o<<c<<" ";}
  
  return o<<endl;
}


int Problem::cnfauxvar(){
  aux.insert(nextcnfvar++);
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> auxilliary variable"<<endl;
  return nextcnfvar - 1;
}

int Problem::cnfauxvar_lex(){
  aux.insert(nextcnfvar++);
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> lexical auxilliary variable"<<endl;
  return nextcnfvar - 1;
}



int Problem::cnfvar(int triangle, int vertex){
  auto index = pair<int,int>(triangle, vertex);
  
  auto it = triangle_at_vertex__to__cnfvar.find(index);
  
  if(triangle_at_vertex__to__cnfvar.end() == it){
    auto answer = nextcnfvar++;
    triangle_at_vertex__to__cnfvar[index] = answer;
    cerr<<"MAPPING: "<<answer<<" -> T"<<triangle<<" v"<<vertex<<endl;
    return answer;
  }

  return it->second;
}
  
void Problem::report__witnesses(int permutation, int witness){
  auto it = triad__to__witnesses.find(permutation);
  if( triad__to__witnesses.end() == it ){
    triad__to__witnesses[permutation] = set<int>();
    it = triad__to__witnesses.find(permutation);
  }
  it->second.insert(witness);
}

int Problem::cnfvar(vector<int> permutation, int vertex){
  assert(3 == permutation.size());
  
  auto index = pair<vector<int>,int>(permutation, vertex);
  
  auto it = triad__to__cnfvar.find(index);
  
  if(triad__to__cnfvar.end() == it){
    auto answer = nextcnfvar++;
    triad__to__cnfvar[index] = answer;
    cerr<<"MAPPING: "<<answer<<" -> PERM{c"<<permutation[0]<<",c"<<permutation[1]<<",c"<<permutation[2]<<"} v"<<vertex<<endl;
    return answer;
  }

  return it->second;
}

int Problem::cnfvar(EC _ec){
  EC ec = _ec;

  // Not a directed graph...
  if (ec.first.first > ec.first.second){
    auto tmp = ec.first.first;
    ec.first.first = ec.first.second;
    ec.first.second = tmp;
  } else if (ec.first.first == ec.first.second) {
    assert(0);
  }
  
  auto it = edge_colour__to__cnfvar.find(ec);

  if(edge_colour__to__cnfvar.end() == it){
    auto answer = nextcnfvar++;
    edge_colour__to__cnfvar[ec] = answer;
    cerr<<"MAPPING: "<<answer<<" -> v"<<ec.first.first<<" x v"<<ec.first.second<<" = c"<<ec.second<<endl;
    return answer;
  }

  return it->second;
}



/** 
 * Print a DAG for this problem
 */
void Problem::print__dag_file(string filename) {
  ofstream dag_file;
  dag_file.open(filename);
  dag_file << "DAG-FILE" << endl;
  dag_file << "NODES:" << 3 << endl;
  dag_file << "GRAPH:" << endl;
  dag_file << "0->1:";

  for (int i=0;i<N-1; i++)
    for (int c=0; c<M; c++) {
      dag_file << cnfvar(EC(E(i,N-1),c));
      if ((i!=N-2)||(c!=M-1))
        dag_file << ",";
    }
  dag_file << endl;
  dag_file << "1->2:";

  for (int i=0;i<N-1; i++)
    for (int c=0; c<M; c++) {
      dag_file << cnfvar(EC(E(i,N-1),c));
      if ((i!=N-2)||(c!=M-1))
        dag_file << ",";
    }
  if (Z!=0)
    dag_file << ",";
  for (int i=0;i<min(Z,N-2); i++) { // 3 is sufficient divergence
    for (int c=0; c<M; c++) {
      dag_file << cnfvar(EC(E(i,N-2),c));
      if ((i!=min(Z,N-2)-1)||(c!=M-1))
        dag_file << ",";
    }
  }
  dag_file << endl;

  dag_file << "CLAUSES:" << endl;
  dag_file << "0:" << levNa_begin << "-" << (levNa_end-1) << "," << levNb_begin << "-" << (levNb_end-1) << endl;
  dag_file << "1:0-" << levNc_end << endl;
  dag_file << "2:" << 0 << "-" << (num_clauses-1) << endl;
  dag_file << "REPORTING:" << endl;
  for (int i=0; i<N; i++)
    for (int j=i+1; j<N; j++)
      for (int c=0; c<M; c++) {
        dag_file << cnfvar(EC(E(i,j),c));
        if ((i!=N-2)||(j!=N-1)||(c!=M-1))
          dag_file << ",";
      }
  dag_file << endl;
}




