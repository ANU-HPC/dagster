
#include "stdio.h"
#define N 7


long long A_n[N*N];
long long A_d[N*N];
long long dotA_n[N];
long long dotA_d[N];

int fill(long l) {
	// fill matrix from bits from l
	for (int i=0; i<N; i++) {
		for (int j=0; j<i; j++) {
			int ll = l&1;
			l = l >> 1;
			A_n[i*N+j] = ll;
			A_n[j*N+i] = ll;
			A_d[i*N+j] = 1;
			A_d[j*N+i] = 1;
		}
		A_n[i*N+i] = 0;
		A_d[i*N+i] = 1;
	}
	// check if lex minimal
	for (int i=0; i<N; i++) { // row i
		for (int j=0; j<i; j++) { // row j
			int ii = 0;
			int jj = 0;
			for (int k=N-1; k>=0; k--) { // across columns
				if ((k!=i) && (k!=j)) {
					ii = ii * 2 + A_n[k*N+i];
					jj = jj * 2 + A_n[k*N+j];
				}
			}
			if (jj>ii) {
				return 0;
			}
		}
	}
	
	// if excess bits, then return -1;
	if (l!=0)
		return -1;
	return 1;
}

int draw(long l) {
	if (fill(l)==-1) return -1;
	printf("Matrix is:\n");
	for (int y=0; y<N; y++) {
		for (int x=0; x<N; x++)
			printf("%i ",A_n[y*N+x]);
		printf("\n");
	}
	return 0;
}

void draw_matrix() {
	printf("Matrix is:\n");
	for (int y=0; y<N; y++) {
		for (int x=0; x<N; x++)
			printf("%i/%i ",A_n[y*N+x],A_d[y*N+x]);
		printf("\n");
	}

}

#define INPLACE_ADD(AN,AD,BN,BD) \
  AN = AN*BD + BN*AD; \
  AD = AD*BD; \
  if (AN==0) \
    AD=1; \
  if (AN==AD) { \
    AN = 1; \
    AD = 1; \
  } \
  if (AD<0) { \
    AN *= -1; \
    AD *= -1; \
  } \
  while (((AD&1)==0) && ((AN&1)==0)) { \
    AD = AD / 2; \
    AN = AN / 2; \
  } \

#define MULTIPLY(A_n, A_d, B_n, B_d, C_n, C_d) \
  long long A_n = B_n*C_n; \
  long long A_d = B_d*C_d; \
  if (A_d<0) { \
    A_n *= -1; \
    A_d *= -1; \
  }\

#define DOT(VARNAMEN,VARNAMED, ARRAY1N, ARRAY1D, ARRAY2N, ARRAY2D, COL1, COL2) \
  long long VARNAMEN=0; \
  long long VARNAMED=1; \
  for (int k=0; k<N; k++) { \
    INPLACE_ADD(VARNAMEN,VARNAMED,ARRAY1N[k*N+COL1]*ARRAY2N[k*N+COL2], ARRAY1D[k*N+COL1]*ARRAY2D[k*N+COL2]) \
  } \
\


long long ret_n;
long long ret_d;

int run(long l) {
	int f = fill(l);
	if (f==-1) return -1;
	if (f==0) return -2;
	
	draw_matrix();
	
	// calculate the square of the determinant
	for (int i=0; i<N; i++) {
		for (int j=0; j<i; j++) {
			if (dotA_n[j]==0.0)
				continue;
			DOT(dotba_n,dotba_d,A_n,A_d,A_n,A_d,i,j)
			if (dotba_d<0) printf("here2\n");
			if (dotba_n!=0) {
				MULTIPLY(dd_n, dd_d, dotba_n, dotba_d, dotA_d[j], dotA_n[j])
				if (dd_d<0) printf("here3\n");
				for (int k=0; k<N; k++) {
					INPLACE_ADD(A_n[k*N+i],A_d[k*N+i], -dd_n*A_n[k*N+j], dd_d*A_d[k*N+j])
					if (A_d[k*N+i]<0) printf("here4\n");
				}
			}
		}
		draw_matrix();
		DOT(dotaa_n,dotaa_d,A_n,A_d,A_n,A_d,i,i)
		dotA_n[i]=dotaa_n;
		dotA_d[i]=dotaa_d;
		if (dotaa_d<0) printf("here1\n");
	}
	ret_n = 1;
	ret_d = 1;
	for (int k=0; k<N; k++) {
		ret_n *= dotA_n[k];
		ret_d *= dotA_d[k];
	}
	return 0;
}


int main() {
	int max_det2 = 0;
	long l = 1587324;
	int ret=0;
	printf("starting\n");
	while (ret!=-1) {
		ret = run(l);
		return 0;
		if (ret!=-2)
			printf("returning code %i: %i %i\n",ret,ret_n,ret_d);
		double ret_val = ret_n*1.0/ret_d;
		if ( (ret_val>=max_det2) && (ret_val>0)) {
			max_det2 = ret_val;
			printf("returning Det^2 of %i\n",max_det2);
			draw(l);
		}
		l++;
	}
}
