
#ifndef RIM_HH
#define RIM_HH

// an easy lookup list of integers. map stores sequential list of positive integers upto a specified number
// and reverse_map stores the position on the list that any specific integer occurs on.
struct ReversableIntegerMap {
  int size;
  unsigned int* map;
  unsigned int* reverse_map;
  unsigned int map_size;
};

typedef struct ReversableIntegerMap ReversableIntegerMap;

#endif
