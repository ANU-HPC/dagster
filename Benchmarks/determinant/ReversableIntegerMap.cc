
#include "ReversableIntegerMap.h"
#include <assert.h>
#include "stdio.h"
#include "stdlib.h"


void ReversableIntegerMap_increase_size(ReversableIntegerMap* this, unsigned int additional_size) {
  if (additional_size<=0) return;
  this->map = (unsigned int*)realloc(this->map, sizeof(unsigned int)*(this->size+additional_size));
  this->reverse_map = (unsigned int*)realloc(this->reverse_map, sizeof(unsigned int)*(this->size+additional_size));
  for (int i=this->size; i<this->size+additional_size; i++) {
    this->reverse_map[i] = -1;
  }
  this->size = this->size+additional_size;
}

void ReversableIntegerMap_append(ReversableIntegerMap* this, unsigned int c) {
  //if (c+1>this->size)
  //  ReversableIntegerMap_increase_size(this,c+1-this->size);
  if (this->reverse_map[c] == -1) {
    this->map[this->map_size] = c;
    this->reverse_map[c] = this->map_size;
    this->map_size++;
  }
}

void ReversableIntegerMap_append_inplace(ReversableIntegerMap* this, unsigned int c, unsigned int place) {
  //if (c+1>this->size)
  //  ReversableIntegerMap_increase_size(this,c+1-this->size);
  assert(this->reverse_map[c] == -1);
  if (place==this->map_size) {
    ReversableIntegerMap_append(this, c);
    return;
  }
  assert(place<this->map_size);
  // swap variable in <place> to the end
  this->map[this->map_size] = this->map[place];
  this->reverse_map[this->map[place]] = this->map_size;
  // write new variable c into <place>
  this->map[place] = c;
  this->reverse_map[c] = place;
  
  this->map_size++;
}

void ReversableIntegerMap_reset(ReversableIntegerMap* this) {
  for (int i=0; i<this->size; i++)
    this->reverse_map[i] = -1;
  this->map_size = 0;
}


void ReversableIntegerMap_init(ReversableIntegerMap* this, unsigned int size) {
  this->size = size;
  this->map = (unsigned int*)malloc(sizeof(unsigned int)*size);
  this->reverse_map = (unsigned int*)malloc(sizeof(unsigned int)*size);
  ReversableIntegerMap_reset(this);
}


void ReversableIntegerMap_del(ReversableIntegerMap* this) {
  free(this->map);
  free(this->reverse_map);
}



void ReversableIntegerMap_remove(ReversableIntegerMap* this, unsigned int c) {
  assert(c<this->size);
  if (this->reverse_map[c] != -1) {
    this->map_size--;
    this->map[this->reverse_map[c]] = this->map[this->map_size];
    this->reverse_map[this->map[this->map_size]] = this->reverse_map[c];
    this->reverse_map[c] = -1;
  }
}



void ReversableIntegerMap_resize(ReversableIntegerMap* this, unsigned int new_size) {
  if (new_size>this->size)
    ReversableIntegerMap_increase_size(this,new_size-this->size);
}


void ReversableIntegerMap_print(ReversableIntegerMap* this) {
  for (int i=0; i<this->map_size; i++)
    printf("%i ",this->map[i]);
  printf("\n");
  for (int i=0; i<this->size; i++)
    printf("%i ",this->reverse_map[i]);
  printf("\n");
}

int ReversableIntegerMap_get_forward_map(ReversableIntegerMap* this, int v) {
  if ((v<0)||(v>=this->map_size))
    return -1;
  return this->map[v];
}

int ReversableIntegerMap_get_reverse_map(ReversableIntegerMap* this, int v) {
  if ((v<0)||(v>this->size))
    return -1;
  return this->reverse_map[v];
}





