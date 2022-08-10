
#include "stdio.h"
#define N 5

#define DOT(VARNAME, ARRAY1, ARRAY2, COL1, COL2) \
  double VARNAME=0; \
  for (int k=0; k<N; k++) \
    VARNAME += ARRAY1[k*N+COL1]*ARRAY2[k*N+COL2]; \
\

double A[N*N];
double dotA[N];

int fill(long l) {
	// fill matrix from bits from l
	for (int i=0; i<N; i++) {
		for (int j=0; j<i; j++) {
			int ll = l&1;
			l = l >> 1;
			A[i*N+j] = ll;
			A[j*N+i] = ll;
		}
		A[i*N+i] = 0;
	}
	// if excess bits, then return -1;
	return l==0;
}

double draw(long l) {
	if (!fill(l)) return -1.0;
	printf("Matrix is:\n");
	for (int y=0; y<N; y++) {
		for (int x=0; x<N; x++)
			printf("%i ",A[y*N+x]==1.0 ? 1 : 0);
		printf("\n");
	}
}

double run(long l) {
	if (!fill(l)) return -1.0;
	
	// calculate the square of the determinant
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
	return m;
}


int main() {
	int max_det2 = 0;
	long l = 0;
	double ret=0;
	while (ret!=-1.0) {
		ret = run(l);
		if ((int)(ret+0.000001)>=max_det2) {
			max_det2 = (int)(ret+0.000001);
			printf("returning Det^2 of %i\n",max_det2);
			draw(l);
		}
		l++;
	}
}
