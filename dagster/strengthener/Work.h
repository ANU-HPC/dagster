#ifndef REDUCER_WORK_H
#define REDUCER_WORK_H
//
//     MiniRed/GlucoRed
//
//     Siert Wieringa 
//     siert.wieringa@aalto.fi
// (c) Aalto University 2012/2013
//
//
#include <cassert>
#include <cstddef>
#include <tuple>

struct WorkWithPosArrayWithLength {
  int* clauseArrayWithPos;
  int length;
};

// Implements the 'Work' set.
class Work {
 public:
  Work  ();
  ~Work ();
  
  WorkWithPosArrayWithLength* insert (WorkWithPosArrayWithLength* w, int smetric); // Inserts work 'w'. In case the workset is full returns the element that is removed to make space for 'w'.
  std::tuple<WorkWithPosArrayWithLength*, int>  get    ();                         // Get the 'best' element from the workset.
  
  inline bool available () const { return best != NULL; }
 protected:
  
  // 'E' is an element of two linked lists, one sorted by time of insertion,
  // and one sorted by 'smetric' (smaller=better)
  class E {
  public:
	void insert(WorkWithPosArrayWithLength *_work, int _smetric, E* best, E* newest) {
      work    = _work;
      smetric = _smetric;
      older   = newest;
      newer   = NULL;
      
      // Make this the 'best' element, then 'scroll it backwards' to the
      // right position
      worse  = best;
      better = NULL;
      while( worse && worse->smetric < smetric ) {
		better= worse;
		worse = worse->worse;
      }
      
      // Update pointers in other instances of this class to deal with insertion
      if ( worse  ) { assert(worse->better == better); worse->better = this; }
      if ( better ) { assert(better->worse == worse);  better->worse = this; }
      if ( older )  { assert(older->newer == NULL); older->newer = this; }
	}
    
	void remove() {
      // Update pointers in other instances of this class to deal with removal
      if ( newer  ) { assert(newer->older == this); newer->older = older; }
      if ( older  ) { assert(older->newer == this); older->newer = newer; }
      if ( better ) { assert(better->worse == this); better->worse = worse; }
      if ( worse  ) { assert(worse->better == this); worse->better = better; }
	}
    
	WorkWithPosArrayWithLength* work;
	int       smetric;
	E*        newer;
	E*        older;
	E*        better;
	E*        worse;
  };
  
  int  free;
  E*   workset;
  E**  space;
  E*   newest;
  E*   oldest;
  E*   best;
};
#endif // REDUCER_WORK_H_
