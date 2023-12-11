/*
Copyright 2023 Charles Gretton

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

*/


#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <cassert>
#include <sstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

#include "global.hh"
#include "problem.hh"
#include "constraints.hh"

using namespace std;


// Representation of The Problem as CNF (see \member{problem.cnf}), and related helper functions.
// see -- problem.hh
Problem problem;

class T_cgraph{
public:
  std::set<int> colors;
  std::set<int> vertices;
  std::map<std::tuple<int, int>, int> edge__to__color;
};

class T_rcprob{
public:
  Triangles nonmonochromatic_triangles;
};

/* Populate the set of nonmonochromatic in $rcprob$ */
void build_all_triangles(int M, T_rcprob& rcprob){
  assert(M);
  for(auto x = 0 ; x < M; x++){
    for(auto y = x ; y < M; y++){
      for(auto z = y ; z < M; z++){
	auto triangle = Triangle(x,y,z);
	if (x == z && x == y){
	  cerr<<"MONOCHROMATIC TRIANGLE :: "<<triangle<<endl;
	} else{
	  rcprob.nonmonochromatic_triangles.insert(triangle);
	}
      }
    }
  }
}

/* Returns true if the first argument is a Ramsey coloring. */
bool is_Ramsey(T_cgraph& cgraph, const T_rcprob& rcprob, const set<int>& skips){  
  for (auto i : cgraph.vertices){
    if (skips.end() != skips.find(i))continue; // Do not process vertices we are skipping...
    for (auto j : cgraph.vertices){
      if (skips.end() != skips.find(j))continue; // Do not process vertices we are skipping...
      if (j<=i)continue;
      for (auto k : cgraph.vertices){
	if (k<=j)continue;
	if (skips.end() != skips.find(k))continue; // Do not process vertices we are skipping...

	
	auto out = std::tuple<int, int>(i,j);
	auto across = std::tuple<int, int>(j,k);
	auto back = std::tuple<int, int>(k,i);

	auto p_out = cgraph.edge__to__color.find(out);
	auto p_across = cgraph.edge__to__color.find(across);
	auto p_back = cgraph.edge__to__color.find(back);

	// SANITY - Graph should be a clique, thus all above edges should exist. Aborts if that is not the case. 
	if(cgraph.edge__to__color.end() == p_out){cerr<<"Cannot find edge: "<<get<0>(out)<<","<<get<1>(out)<<std::endl;assert(0);}
	if(cgraph.edge__to__color.end() == p_across){cerr<<"Cannot find edge: "<<get<0>(across)<<","<<get<1>(across)<<std::endl;assert(0);}
	if(cgraph.edge__to__color.end() == p_back){cerr<<"Cannot find edge: "<<get<0>(back)<<","<<get<1>(back)<<std::endl;assert(0);}
	
	auto color1 = p_out->second;
	auto color2 = p_across->second;
	auto color3 = p_back->second;

	// Do we have a monochromatic triangle. If yes, return false. 
	if (color1 == color2 && color2 == color3 ){
	  std::cerr<<"FAILING - monocromatic "<<i<<" "<<j<<" "<<k<<std::endl;
	  return false;
	}
	
      }
    }
  }
  cerr<<"Completed test for monocromatic triangles.\n";
  
  for (auto i : cgraph.vertices){
    if (skips.end() != skips.find(i))continue; // Do not process vertices we are skipping...
    
    Triangles my_triangles;
    cerr<<"Processing vertex: "<<i<<endl;
    for(auto j : cgraph.vertices){
      if ( i == j ) continue;
      if (skips.end() != skips.find(j))continue; // Do not process vertices we are skipping...

      /* Once we have satisfied the Ramsey condition at a vertex, stop checking for it. */
      if(my_triangles.size() == rcprob.nonmonochromatic_triangles.size())continue;
      
      for(auto k : cgraph.vertices){
	if (j == k || k == i ) continue;
	if(my_triangles.size() == rcprob.nonmonochromatic_triangles.size())continue;
	if (skips.end() != skips.find(k))continue; // Do not process vertices we are skipping...
	
	auto out = std::tuple<int, int>(i,j);
	auto across = std::tuple<int, int>(j,k);
	auto back = std::tuple<int, int>(k,i);

	auto p_out = cgraph.edge__to__color.find(out);
	auto p_across = cgraph.edge__to__color.find(across);
	auto p_back = cgraph.edge__to__color.find(back);
	
	// SANITY - Graph should be a clique, thus all above edges should exist. Aborts if that is not the case. 
	if(cgraph.edge__to__color.end() == p_out){cerr<<"Cannot find edge: "<<get<0>(out)<<","<<get<1>(out)<<std::endl;assert(0);}
	if(cgraph.edge__to__color.end() == p_across){cerr<<"Cannot find edge: "<<get<0>(across)<<","<<get<1>(across)<<std::endl;assert(0);}
	if(cgraph.edge__to__color.end() == p_back){cerr<<"Cannot find edge: "<<get<0>(back)<<","<<get<1>(back)<<std::endl;assert(0);}
	
	auto color1 = p_out->second;
	auto color2 = p_across->second;
	auto color3 = p_back->second;

	vector<int> local_colors;
	local_colors.push_back(color1);
	local_colors.push_back(color2);
	local_colors.push_back(color3);
	sort(local_colors.begin(), local_colors.end());
       
	assert(my_triangles.size() <= rcprob.nonmonochromatic_triangles.size());
	auto triangle = Triangle(local_colors[0], local_colors[1], local_colors[2]);
	my_triangles.insert(triangle);
	assert(my_triangles.size() <= rcprob.nonmonochromatic_triangles.size());
      }
    }
    assert(my_triangles.size() <= rcprob.nonmonochromatic_triangles.size());
    if(my_triangles.size() != rcprob.nonmonochromatic_triangles.size()){
      std::cerr<<"FAILING - all required triangles represented "<<i<<std::endl;
      return false;
    }
  }

  cerr<<"Is Ramsey is True.\n";
  return true;
}

/*Parse 'PAJEK' formatted graph with colored edges. Result stored in second argument. */
int parse(const std::string& filename, T_cgraph& cgraph) {
  std::ifstream file(filename);
  std::string line;
  
  while (std::getline(file, line)) {
    if ( '*' == line[0]) continue; // skip *ed lines
    if ( string::npos != line.find("ellipse")) continue;
    std::istringstream iss(line);

    int from, to, color;
    iss>>from;
    if(iss.fail()){std::cerr<<line<<std::endl;assert(0);}
    iss>>to;
    if(iss.fail()){std::cerr<<line<<std::endl;assert(0);}
    iss>>color;
    if(iss.fail()){std::cerr<<line<<std::endl;assert(0);}

    cgraph.vertices.insert(to);
    cgraph.vertices.insert(from);
    
    cgraph.colors.insert(color);
    auto edge1 = std::tuple<int, int>(from, to);
    auto edge2 = std::tuple<int, int>(to, from);
    cgraph.edge__to__color[edge1] = color;
    cgraph.edge__to__color[edge2] = color;
    
    std::cerr << "Parsing :: "<<line << '\n';
  }
  
  return 0;
}

int main(int argc, char** argv){
  std::string arg1;
  int vertices_to_remove_per_iteration = 20;
  int color_to_remove = -1;
  
  if (argc > 1) {
    arg1 = argv[1];
  }
  if (argc > 2 ) {
      vertices_to_remove_per_iteration= atoi(argv[2]);
  }
  if (argc > 3 ) {
      color_to_remove = atoi(argv[3]);
      cout<<"c removing c"<<color_to_remove<<endl;
  }

  T_cgraph cgraph;
  T_rcprob rcprob;
  parse(arg1, cgraph);
  cerr<<"Colors :"<<cgraph.colors.size()<<endl;
  cerr<<"Vertices: "<<cgraph.vertices.size()<<endl;
  build_all_triangles(cgraph.colors.size(), rcprob);
  
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 random_generator(seed);
  set<int> skips;//vertices that have been removed
  set<int> local_skips;
  while( is_Ramsey(cgraph, rcprob, skips)){
    local_skips = set<int>();
  
    vector<int> diff;
    set_difference(cgraph.vertices.begin(), cgraph.vertices.end(), skips.begin(), skips.end(), back_inserter(diff));
    std::uniform_int_distribution<int> distribution(0, diff.size() - 1);
    for ( auto ii = 0; ii < vertices_to_remove_per_iteration; ii++){
      auto random_index = distribution(random_generator);
      assert(random_index < diff.size());
      auto i = diff[random_index];
      //assert(skips.end()==skips.find(i));
      if (skips.find(i) != skips.end())continue;

      local_skips.insert(i);
      skips.insert(i);
      
    }
    
    cerr<<"Ramsey still with Skipping: ";
    for (auto x : skips){
      cerr<<x<<" ";
    }
    cerr<<std::endl;
  }

  //Recover what was still a Ramsey coloring
  for(auto x : local_skips){skips.erase(x);}


  cerr<<"Colors :"<<cgraph.colors.size()<<endl;
  cerr<<"Vertices: "<<cgraph.vertices.size()<<endl;
  cerr<<"Skipping ("<<skips.size()<<"): ";
  for (auto x : skips){
    cerr<<x<<" ";
  }
  cerr<<std::endl;

  // Setup problem parameters M (colors) and N (number of vertices)
  assert(skips.size() < vertices.size());
  problem.N = cgraph.vertices.size() - skips.size();
  problem.M = (color_to_remove >= 0)
    ?(cgraph.colors.size()-1)// We will try to use the parsed graph produce a Ramsey coloring with fewer colors
    :cgraph.colors.size();
  
  problem.build_all_triangles();
  
  post__clique_edges_have_one_colour_each(problem.M, problem.N); // DONE
  problem.levNc_end = problem.num_clauses;
  
  post__triad_at_vertex_implies_corresponding_triangle_at_vertex(problem.N, problem.triangles); // DONE

  // Clique vertices
  auto vertices = vector<int>(problem.N);
  iota (std::begin(vertices), std::end(vertices), 0);
  post__coloured_3_tour_from_vertex_implies_corresponding_triad(vertices, problem.triangles); // DONE  

  // Should do nothing because $nonmonochromatic_triangles$ should be empty...
  post__every_nonmonochromatic_triangle_appears_everywhere_it_can(problem.N,problem.nonmonochromatic_triangles); // DONE
  post__there_are_no_monochromatic_triangles(problem.N,problem.monochromatic_triangles); // DONE

  //Color to skip
  assert(cgraph.colors.size());
  assert(color_to_remove < 0 || cgraph.colors.find( color_to_remove) != cgraph.colors.end());
  int color_to_skip = color_to_remove;//*cgraph.colors.begin();
  
  // Remapping vertices given we intend to skip some. 
  int counter = 0;
  map<int,int> map_vertex__to__sequence;
  for (auto i : cgraph.vertices){
    if (skips.find(i) != skips.end()) continue;
    map_vertex__to__sequence[i] = counter++;
  }

  counter = 0;
  map<int,int> map_color__to__sequence;
  for(auto i : cgraph.colors){
    if (i == color_to_skip) continue;
    map_color__to__sequence[i] = counter++;
  }

  // Add unit clauses to problem that imply decisions we are keeping for the parsed graph. 
  for (auto i : cgraph.vertices){
    if (skips.find(i) != skips.end()) continue;
    for (auto j : cgraph.vertices){
      if (i >= j) continue;
      if (skips.find(j) != skips.end()) continue;

      cout<<"c query x"<<i<<" y"<<j<<endl;
      auto edge = std::tuple<int, int>(i,j);
      assert(cgraph.edge__to__color.find(edge) != cgraph.edge__to__color.end());
      auto p_out = cgraph.edge__to__color.find(edge)->second;
      assert(p_out>=0);
      cout<<"c query c"<<p_out<<" sc"<<color_to_skip<<endl;
      if (p_out == color_to_skip ) continue;

      // Make sure we have parsed the edge color information we will force with a unit clause
      assert(map_color__to__sequence.find(p_out) != map_color__to__sequence.end());
      assert( map_vertex__to__sequence.find(i) !=  map_vertex__to__sequence.end());
      assert( map_vertex__to__sequence.find(j) !=  map_vertex__to__sequence.end());
      
      auto mapped_color = map_color__to__sequence[p_out];
      auto mapped_i = map_vertex__to__sequence[i];
      auto mapped_j = map_vertex__to__sequence[j];
      cout<<"c "<<"x"<<mapped_i<<" y"<<mapped_j<<" c"<<mapped_color<<endl;
      print__color_edge(mapped_i, mapped_j, mapped_color);
    }
  }

  auto num_vars = problem.edge_colour__to__cnfvar.size() +
    problem.triangle_at_vertex__to__cnfvar.size() +
    problem.triad__to__cnfvar.size() +
    problem.aux.size();
  
  const auto& formula = problem.cnf.str();
  cout<<"p cnf "<<num_vars<<" "<<problem.num_clauses<<endl;
  cout<<formula;

  
  return 0;
}
