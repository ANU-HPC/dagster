
#include "stdio.h"
#define N 6

#define DOT(VARNAME, ARRAY1, ARRAY2, COL1, COL2) \
  float VARNAME=0; \
  for (int k=0; k<N; k++) \
    VARNAME += ARRAY1[k*N+COL1]*ARRAY2[k*N+COL2]; \
\

float nondet_float();
int main() {
	float A[N*N];
	float dotA[N];
	
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			A[i*N+j] = nondet_float();
			if ((A[i*N+j]!=0.0)&&(A[i*N+j]!=1.0))
				return 0;
			if (A[i*N+j] != A[j*N+i])
				return 0;
		}
		if (A[i*N+i]!=0)
			return 0;
	}
	
	for (int i=0; i<N; i++) {
		for (int j=0; j<i; j++) {
			if (dotA[j]==0.0)
				continue;
			DOT(dotba,A,A,i,j)
			float dd = dotba/dotA[j];
			for (int k=0; k<N; k++)
				A[k*N+i] = A[k*N+i] - dd*A[k*N+j];
		}
		DOT(dotaa,A,A,i,i)
		dotA[i]=dotaa;
	}
	float m = 1.0;
	for (int k=0; k<N; k++)
		m *= dotA[k];
	//__CPROVER_assert(m>=0.0, "Determinant Assert");
	__CPROVER_assert(m<=6.9, "Determinant Assert");
}
