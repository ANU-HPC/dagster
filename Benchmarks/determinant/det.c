
#include <stdint.h>

#include "ReversableIntegerMap.h"
#include "ReversableIntegerMap.cc"

#define N 3

_Bool M[N*N];

//char M[N*N];
ReversableIntegerMap x_indexes;
ReversableIntegerMap x_not_indexes;
ReversableIntegerMap y_indexes;
ReversableIntegerMap y_not_indexes;

/*char get(int x,int y) {
	int max = x>y? x:y;
	int min = x>y? y:x;
	return M[max*N+min];
}*/

int get(int x, int y) {
	return M[x*N+y];
}

int depth = -1;
int a() {
	depth += 1;
	if (x_not_indexes.map_size == 1) {
		depth -= 1;
		return get(ReversableIntegerMap_get_forward_map(&x_not_indexes,0), ReversableIntegerMap_get_forward_map(&y_not_indexes,0));
	}
	int val = 0;
	int sign = 1;
	int x_index = ReversableIntegerMap_get_forward_map(&x_not_indexes,0);
	ReversableIntegerMap_append(&x_indexes,x_index);
	ReversableIntegerMap_remove(&x_not_indexes,x_index);
	for (int i=0; i<y_not_indexes.map_size; i++) {
		int y_index = ReversableIntegerMap_get_forward_map(&y_not_indexes,i);
		ReversableIntegerMap_append(&y_indexes,y_index);
		ReversableIntegerMap_remove(&y_not_indexes,y_index);
		val = val + sign*get(x_index,y_index)*a();
		ReversableIntegerMap_append_inplace(&y_not_indexes,y_index,i);
		ReversableIntegerMap_remove(&y_indexes,y_index);
		sign = sign * -1;
	}
	ReversableIntegerMap_append_inplace(&x_not_indexes,x_index,0);
	ReversableIntegerMap_remove(&x_indexes,x_index);
	depth -= 1;
	return val;
}

int main(){

	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			__CPROVER_assume((M[i*N+j]==1)||(M[i*N+j]==0));
			//M[i*N+j] = i+j+1;
		}
	}

	ReversableIntegerMap_init(&x_indexes,N);
	ReversableIntegerMap_init(&x_not_indexes,N);
	ReversableIntegerMap_init(&y_indexes,N);
	ReversableIntegerMap_init(&y_not_indexes,N);
	for (int i=0; i<N; i++) {
		ReversableIntegerMap_append(&x_not_indexes,i);
		ReversableIntegerMap_append(&y_not_indexes,i);
	}
	
	int aa = a();
	__CPROVER_assert(aa>20, "i_rdev mismatch");
	//printf("%i\n",a());
    // GO FOR ANOTHER ROUND - SUGGEST UNROLL MANUALLY.
    /*_Bool aa;
    int aaa = 3*(aa + 1);
    __CPROVER_assert(aaa <-2, "postcondition1");*/

    //__CPROVER_assert(a() > 5, "postcondition1");

    // return 0;
}
