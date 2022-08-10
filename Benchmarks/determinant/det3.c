
#include "stdio.h"
#define N 3

#define DOT(VARNAME, ARRAY1, ARRAY2, COL1, COL2) \
  double VARNAME=0; \
  for (int k=0; k<N; k++) \
    VARNAME += ARRAY1[k*N+COL1]*ARRAY2[k*N+COL2]; \
\

double A[N*N];
double dotA[N];

int main() {

	A[0*3+0]=1;
	A[0*3+1]=0;
	A[0*3+2]=0;
	
	A[1*3+0]=0;
	A[1*3+1]=1;
	A[1*3+2]=-1;
	
	A[2*3+0]=3;
	A[2*3+1]=0;
	A[2*3+2]=-4;
	
	printf("Matrix is:\n");
	for (int y=0; y<N; y++) {
		for (int x=0; x<N; x++)
			printf("%f ",A[y*N+x]);
		printf("\n");
	}
	
	for (int i=0; i<N; i++) {
		for (int j=0; j<i; j++) {
			if (dotA[j]==0.0)
				continue;
			DOT(dotba,A,A,i,j)
			double dd = dotba/dotA[j];
			for (int k=0; k<N; k++)
				A[k*N+i] = A[k*N+i] - dd*A[k*N+j];
		}
		DOT(dotaa,A,A,i,i)
		dotA[i]=dotaa;
	}
	double m = 1.0;
	for (int k=0; k<N; k++)
		m *= dotA[k];
	
	printf("Square of Determinant is: %f\n",m);
}
