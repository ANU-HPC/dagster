/* Author: Charles Gretton 
 * Created: Sat 7 Sep 12:23:01 AEST 2019
 *
 *
 * Background: Problem described by David Jamieson on preceding
 * Thursday (5th Sep). He gave a demo where he was using Z3 to
 * generate instances of The Problems stth only have one solution.
 * 
 * NOTE: I have seen similar problems posed recently in sensing
 * byzantine-fault "tolerant" consensus networks.
 *
 * NOTE: I believe a tractable algorithm exists for the exact problem
 * below, however the network synthesis problem with a solution
 * counting constraint, especially where we allow higher degrees of
 * connectivity (M-dimensional scenario), is intractable. It would not
 * be a waste of time to do a lit.review and characterise these.
 *
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * Setting: Two dimensional lattice induces planar square grid of tiles. Some tiles are labelled as being members of Class A or Class B. 
 *
 * Problem: is to find a path/boarder between tiles of class A and tiles of class B, where that path extends from the bottom-left vertex to the top-right vertex.
 *
 * Question: D. Jamieson asked me if we could do this in SAT without a counter. 
 *
 * Answer: Yes, as follows.
 *
 */

/*************************
Copyright 2020 Charles Gretton, Josh Milthorpe

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


#include "global.hh"

class Problem{
public:
  /* Regions, including tiles, are coloured in two colours, here white and black.*/
  const static int WHITE = 0;
  const static int BLACK = 1;

  /* Special index for boundary spaces around the matrix of tiles. No need to consider the diagonals.*/
  const static int _TOP;
  const static int _BOTTOM;
  const static int _LEFT;
  const static int _RIGHT;
  const  tuple<int,int> TOP=make_tuple(_TOP,0);//=tuple<int,int>(_TOP, 0);
  const  tuple<int,int> BOTTOM=make_tuple(_BOTTOM,0);//=tuple<int,int>(_BOTTOM,0);
  const  tuple<int,int> LEFT=make_tuple(_LEFT,0);//=tuple<int,int>(_LEFT,0);
  const  tuple<int,int> RIGHT=make_tuple(_RIGHT,0);//=tuple<int,int>(_RIGHT,0);
  
  // Schematic of problem statement that is to be compiled to CNF. Source of information about tiles whose colours are constrained.
  vector<vector<int> > matrix;

  // Parse problem statement, populating $matrix$, $N$ and $regions$. The latter is a vertex in the tile+boundary connectivity graph.
  Problem(const char* input){
    N=sqrt(strlen(input));
    if (N*N != strlen(input)){
      cerr<<"UNRECOVERABLE ERROR: Input must be square.\n";
      exit(-1);
    }
    cerr<<"DIMENSION: "<<N<<endl;
    matrix=vector<vector<int> >(N, vector<int>(N,-1)); // If a tile is not coloured, it has value -1 in $matrix$
    auto count=0;
    for ( auto i = 0; i < matrix.size(); i++){
      for (auto j = 0 ; j < matrix.size(); j++){
	if (input[count] == 'w'){
	  matrix[i][j]=WHITE;
	} else if (input[count] == 'b'){
	  matrix[i][j]=BLACK;
	}
	count++;
      }
    }

    // for(auto x = 0; x < N ; x++){
    //   for(auto y = 0 ; y < N ; y++){
    // 	cout<<matrix[x][y]<<" ";
    //   }
    //   cout<<"\n";
    // }
    for(auto x = 0; x < N ; x++){
      for(auto y = x ; y < N ; y++){
	auto region1 = tuple<int,int>(x,y);
	auto region2 = tuple<int,int>(y,x);
	regions.insert(region1);
	if (region1 != region2) regions.insert(region2);
      }
    }
    
    regions.insert(TOP);
    regions.insert(BOTTOM);
    regions.insert(LEFT);
    regions.insert(RIGHT);
  }
  
  // PROPOSITION -- FLOWS(x, z) and FLOWS(z, y)
  int cnfvar(const tuple<int,int>& region_x, const tuple<int,int>& region_z, const tuple<int,int>& region_y){
    assert(region_x != region_y);
    assert(region_x != region_z);
    assert(region_z != region_y);
    // assert(adjacent(region_x, region_z));
    // assert(adjacent(region_z, region_y));
    // assert(!adjacent(region_x, region_y));
    tuple<tuple<int,int>, tuple<int,int>, tuple<int,int> > query(region_x,region_z,region_y);
    auto _cnfvar = threegram__to__cnfvar.find(query);
    if(threegram__to__cnfvar.end() != _cnfvar){
      return _cnfvar->second;
    }
  
    threegram__to__cnfvar[query] = nextcnfvar++;
    return nextcnfvar - 1;
  }
  
  // PROPOSITION -- FLOWS(x, y)
  int cnfvar(const tuple<int,int>& region_x, const tuple<int,int>& region_y){
    assert(region_x != region_y);
    //assert(adjacent(region_x, region_y));
    tuple<tuple<int,int>, tuple<int,int> > query(region_x,region_y);
    auto _cnfvar = flows__to__cnfvar.find(query);
    if(flows__to__cnfvar.end() != _cnfvar){
      return _cnfvar->second;
    }
  
    flows__to__cnfvar[query] = nextcnfvar++;
    cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> flows"
	<<get<0>(region_x)<<"x"<<get<1>(region_x)
	<<"_"
	<<get<0>(region_y)<<"x"<<get<1>(region_y)<<endl;
    return nextcnfvar - 1;
  }

  // PROPOSITION -- colour(x)=colour
  int cnfvar(const tuple<int,int>& region_x, int colour/*0 (i.e. white) or 1 (i.e. black)*/){
    assert(0<=colour && colour<=1);

    tuple<tuple<int,int>, int > query(region_x, colour);
    
    auto _cnfvar = colour__to__cnfvar.find(query);
    if(colour__to__cnfvar.end() != _cnfvar){
      return _cnfvar->second;
    }
  
    colour__to__cnfvar[query] = nextcnfvar++;
    cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> colour"<<get<0>(region_x)<<"x"<<get<1>(region_x)<<"="<<colour<<endl;
    return nextcnfvar - 1;
  }

  // Region adjacency: Are regions x and y adjacent?
  bool adjacent(const tuple<int,int>& /*region_*/x, const tuple<int,int>& /*region_*/y) const{
    if( y == x ){ // A region is not adjacent itself
      return false;
    } else if (get<0>(x) < 0 && get<0>(y) < 0) { // By definition two boundary regions cannot be adjacent
      return false;
    }

    else if (x == TOP && get<0>(y) == 0){ // Boundary regions test appear in the next blocks
      return true;
    } else if (x == BOTTOM && get<0>(y) == N-1){
      return true; 
    } else if (x == LEFT && get<1>(y) == 0){ 
      return true;
    } else if (x == RIGHT && get<1>(y) == N-1){ 
      return true;
    } else if (y == TOP && get<0>(x) == 0){
      return true;
    } else if (y == BOTTOM && get<0>(x) == N-1){
      return true; 
    } else if (y == LEFT && get<1>(x) == 0){ 
      return true;
    } else if (y == RIGHT && get<1>(x) == N-1){ 
      return true;
    }

    // Tile adjacency is not treated.
    else if (get<0>(x) == get<0>(y) && 1 == abs(get<1>(x) - get<1>(y))){ // Location is equal in one dimension,  and different by one discrete unit in the other[1]
      return true;
    }  else if (get<1>(x) == get<1>(y) && 1 == abs(get<0>(x) - get<0>(y))){ // Location is equal in one dimension,  and different by one discrete unit in the other[0]
      return true;
    } 

    return false;
  }

  // Any region that is _not_ a tile in the space induced by the lattice
  // is a boundary region. These are indicated by the first of the 2D
  // coordinates being negative.
  bool boundary_region(const tuple<int, int>&x) const{
    return get<0>(x) < 0;
  }
  
  // The dimensions of the tiled space induced by the lattice
  int N;
  
  // Includes tiles induced by the lattice, as well as regions to the top, bottom, right, and left of that tiled space.
  set<tuple<int,int> > regions;

  // CNF RELATED. RHS of maps are variable names. LHS of maps are descriptions of variables. 
  map<tuple<tuple<int,int>, tuple<int,int> >, int> flows__to__cnfvar;// Variable:- colour flows from LHS region to RHS region
  map<tuple<tuple<int,int>, int>, int> colour__to__cnfvar;           // Variable:- Colour of LHS region is RHS colour
  map<tuple<tuple<int,int>, tuple<int,int>, tuple<int,int> >, int> threegram__to__cnfvar; // Variable:- Colour flows from LHS region to RHS region via MIDDLE region. 
  
  // CNF RELATED. Global counter, incremented each time we create a new proposition.
  int nextcnfvar = 1;
  
  /* CNF representation of problem.*/
  ostringstream cnf;

  
  // Pre-assigned region colours are made here, and "exactly one" colouring constraint is imposed. 
  void print__regions_are_one_colour(){
    for (auto p = regions.begin()
	   ; p != regions.end()
	   ; p++ ){
      auto colour_p_WHITE = cnfvar(*p,WHITE);
      auto colour_p_BLACK = cnfvar(*p,BLACK);
      //cerr<<"CHECKING REGION: "<<get<0>(*p)<<"x"<<get<1>(*p)<<"\n";
      if (!boundary_region(*p) && WHITE == matrix[get<0>(*p)][get<1>(*p)]){
	cnf<<colour_p_WHITE<<" 0\n";
      } else if (!boundary_region(*p) && BLACK == matrix[get<0>(*p)][get<1>(*p)]){
	cnf<<colour_p_BLACK<<" 0\n";
      }
      cnf<<"-"<<colour_p_WHITE<<" -"<<colour_p_BLACK<<" 0\n";
      cnf<<colour_p_WHITE<<" "<<colour_p_BLACK<<" 0\n";
    }
  }

  void print__flows_are_one_way(){
    for (auto p = regions.begin()
	   ; p != regions.end()
	   ; p++ ){
      auto q = p;
      q++;
      for (
	   ; q != regions.end()
	     ; q++ ){
	if (!boundary_region(*p) && !boundary_region(*q)){
	  auto flows_pq = cnfvar(*p,*q);
	  auto flows_qp = cnfvar(*q,*p);
	  cnf<<"-"<<flows_pq<<" -"<<flows_qp<<" 0\n";
	} else if (boundary_region(*p)){
	  //cerr<<"DEBUG: "<<get<0>(*p)<<"x"<<get<1>(*p)<<"\n"
	  //    <<get<0>(*q)<<"x"<<get<1>(*q)<<"\n";
	  auto flows_pq = cnfvar(*p,*q);
	  cnf<<"-"<<flows_pq<<" 0\n";
	} else if (boundary_region(*q)){
	  auto flows_qp = cnfvar(*q,*p);
	  cnf<<"-"<<flows_qp<<" 0\n";
	}
      }
    }
  }

  // \forall tiles x, \exists boundary s.t. flows(x,boundary)
  void print__goal(){
    auto top_colour_white = cnfvar(TOP, WHITE);
    auto left_colour_white = cnfvar(LEFT, WHITE);
    auto bottom_colour_white = cnfvar(BOTTOM, WHITE);
    auto right_colour_white = cnfvar(RIGHT, WHITE);
    
    auto top_colour_black = cnfvar(TOP, BLACK);
    auto left_colour_black = cnfvar(LEFT, BLACK);
    auto bottom_colour_black = cnfvar(BOTTOM, BLACK);
    auto right_colour_black = cnfvar(RIGHT, BLACK);

    cnf<<"-"<<top_colour_white<<" "<<left_colour_white<<" 0\n";
    cnf<<"-"<<top_colour_white<<" "<<right_colour_black<<" 0\n";
    cnf<<""<<top_colour_white<<" -"<<right_colour_black<<" 0\n";
    cnf<<""<<top_colour_white<<" -"<<left_colour_white<<" 0\n";
    cnf<<"-"<<top_colour_black<<" "<<left_colour_black<<" 0\n";
    cnf<<""<<top_colour_black<<" -"<<left_colour_black<<" 0\n";

    
    cnf<<"-"<<bottom_colour_white<<" "<<right_colour_white<<" 0\n";
    cnf<<"-"<<bottom_colour_white<<" "<<left_colour_black<<" 0\n";
    cnf<<""<<bottom_colour_white<<" -"<<left_colour_black<<" 0\n";
    cnf<<""<<bottom_colour_white<<" -"<<right_colour_white<<" 0\n";
    cnf<<"-"<<bottom_colour_black<<" "<<right_colour_black<<" 0\n";
    cnf<<""<<bottom_colour_black<<" -"<<right_colour_black<<" 0\n";
    
    
    for (auto p = regions.begin()
	   ; p != regions.end()
	   ; p++ ){
      if (!boundary_region(*p) ){ // non-boundary tile
	auto top = cnfvar(*p,TOP);
	auto bottom = cnfvar(*p,BOTTOM);
	auto left = cnfvar(*p,LEFT);
	auto right = cnfvar(*p,RIGHT);

	cnf<<top<<" "<<bottom<<" "<<left<<" "<<right<<" 0\n"; 
      }
    }
  }
  
  //  flows(x,y) AND colour(x)=z ->  colour(y)=z
  // adjacent(x,y) AND colour(x) = colour(y) -> flows(x,y) OR flows(y,x)
  void print__flows_are_colour_matched(){

    for (auto p = regions.begin()
	   ; p != regions.end()
	   ; p++ ){
      auto q = p;
      q++;
      for (
	   ; q != regions.end()
	     ; q++ ){
	auto flows_pq = cnfvar(*p,*q);
	auto flows_qp = cnfvar(*q,*p);
	auto colour_p_WHITE = cnfvar(*p,WHITE);
	auto colour_q_WHITE = cnfvar(*q,WHITE);
	auto colour_p_BLACK = cnfvar(*p,BLACK);
	auto colour_q_BLACK = cnfvar(*q,BLACK);
      
	cnf<<"-"<<flows_pq<<" -"<<colour_p_WHITE<<" "<<colour_q_WHITE<<" 0\n";
	cnf<<"-"<<flows_pq<<" -"<<colour_p_BLACK<<" "<<colour_q_BLACK<<" 0\n";
	cnf<<"-"<<flows_qp<<" -"<<colour_p_WHITE<<" "<<colour_q_WHITE<<" 0\n";
	cnf<<"-"<<flows_qp<<" -"<<colour_p_BLACK<<" "<<colour_q_BLACK<<" 0\n";
	if (adjacent(*p,*q)){ // Forcing flows for monochrome adjacent regions
	  cnf<<" -"<<colour_p_WHITE<<" -"<<colour_q_WHITE<<" "<<flows_pq<<" "<<flows_qp<<" 0\n";
	  cnf<<" -"<<colour_p_BLACK<<" -"<<colour_q_BLACK<<" "<<flows_pq<<" "<<flows_qp<<" 0\n";
	}
      }
    }
  }

  
  
  // threegram(p, r, q) iff flows(p,r) AND flows(r,q)
  void print__threegrams(){
    for (auto p = regions.begin()
	   ; p != regions.end()
	   ; p++ ){
      if (boundary_region(*p))continue;
      for (auto q = regions.begin()
	     ; q != regions.end()
	     ; q++ ){
	for (auto r = regions.begin()
	       ; r != regions.end()
	       ; r++ ){
	  if (boundary_region(*r))continue; // Cannot transit via boundary region
	  if ( *p != *q && *p != *r && *r != *q ){ // all_different(p,q,r)
	    auto flows_pr = cnfvar(*p,*r);
	    auto flows_rq = cnfvar(*r,*q);
	    auto threegram = cnfvar(*p,*r,*q);
	  
	    cnf<<" -"<<flows_pr<<" -"<<flows_rq<<" "<<threegram<<" 0\n";
	    cnf<<flows_pr<<" -"<<threegram<<" 0\n";
	    cnf<<flows_rq<<" -"<<threegram<<" 0\n";
	  }
	}
      }
    }
  }

  //  flows(p,q) <-  flows(p,r) AND flows(r,q) ; AND
  //  flows(p,q) ->  [[ adjacent(p,q) or ]] \exists z s.t. flows(p,r) AND flows(r,q) -- i.e. \exists threegram(p, r, q)
  void print__flows_are_transitive(){
    for (auto p = regions.begin()
	   ; p != regions.end()
	   ; p++ ){
      auto q = p;
      q++;
      for (
	   ; q != regions.end()
	     ; q++ ){
	if (!adjacent(*p,*q)){
	  auto flows_pq = cnfvar(*p,*q);
	  auto flows_qp = cnfvar(*q,*p);
	  ostringstream threegrams_pq;
	  ostringstream threegrams_qp;
	  for (auto r = regions.begin()
		 ; r != regions.end()
		 ; r++ ){
	    if (boundary_region(*r))continue; // Cannot transition through a boundary
	    if (*r != *p && *r != *q){
	      assert(*q != *p);
	      if (!boundary_region(*p)){
		auto flows_pr = cnfvar(*p,*r);
		auto flows_rq = cnfvar(*r,*q);
		cnf<<"-"<<flows_pr<<" -"<<flows_rq<<" "<<flows_pq<<" 0\n";

		
		auto threegram_pq = cnfvar(*p,*r,*q);
		threegrams_pq<<" "<<threegram_pq;
	      }
	      
	      if (!boundary_region(*q)){
		auto flows_qr = cnfvar(*q,*r);
		auto flows_rp = cnfvar(*r,*p);
		cnf<<"-"<<flows_qr<<" -"<<flows_rp<<" "<<flows_qp<<" 0\n";
		
		auto threegram_qp = cnfvar(*q,*r,*p);
		threegrams_qp<<" "<<threegram_qp;
	      }

	    }
	  }// for all possible transit tiles
	  
	  if (!boundary_region(*p)) cnf<<"-"<<flows_pq<<" "<<threegrams_pq.str()<<" 0\n";
	  if (!boundary_region(*q)) cnf<<"-"<<flows_qp<<" "<<threegrams_qp.str()<<" 0\n";
	}
      }
    }
  }
};

const int Problem::_TOP = -1;
const int Problem::_BOTTOM = -2;
const int Problem::_LEFT = -3;
const int Problem::_RIGHT = -4;

int main(int argc, char** argv){

  /* string representing square matrix of cells contain elements in {w(hite),b(lack),0}*/
  char* matrix = argv[1];
  Problem problem(matrix);

  problem.print__regions_are_one_colour();
  problem.print__flows_are_one_way();
  problem.print__flows_are_colour_matched();
  problem.print__threegrams();
  problem.print__flows_are_transitive();
  problem.print__goal();
  
  auto num_vars = problem.threegram__to__cnfvar.size() + problem.flows__to__cnfvar.size() + problem.colour__to__cnfvar.size();
  const auto& formula = problem.cnf.str();
  cout<<"p cnf "<<num_vars<<" "<<count(formula.begin(), formula.end(), '\n')<<endl;
  cout<<problem.cnf.str();
  return 0;
}
