

#define SWAP(A,B) { \
	auto temp = B; \
	B = A; \
	A = temp; \
}
#define ABS(A) A<0 ? -A : A
#define N 4

#include "stdio.h"
int A[N*N];
int B[N*N];


int gjinv() {
	int i, j, k, p;
	int f, g;
	for (i = 0; i < N; i++)  /* Set b to identity matrix. */
		for (j = 0; j < N; j++)
			B[j+i*N] = (i == j) ? 1 : 0;
	for (k = 0; k < N; k++) {  /* Main loop */
		f = ABS(A[k+k*N]);  /* Find pivot. */
		p = k;
		for (i = k+1; i < N; i++) {
			g = ABS(A[k+i*N]);
			if (g > f) {
				f = g;
				p = i;
			}
		}
		if (f == 0) return 1;  /* Matrix is singular. */
		if (p != k) {  /* Swap rows. */
			for (j = k; j < N; j++) SWAP(A[j+k*N],A[j+p*N])
			for (j = 0; j < N; j++) SWAP(B[j+k*N],B[j+p*N])
		}
		if (A[k+k*N]<0) {
			for (j = k; j < N; j++) A[j+k*N] *= -1;
			for (j = 0; j < N; j++) B[j+k*N] *= -1;
		}
		for (i = 0; i < N; i++) {  /* Subtract to get zeros. */
			//if (i<k) continue;
			if (i == k) continue;
			f = A[k+i*N];
			g = A[k+k*N];
			if (f!=0) {
				for (j = 0; j < N; j++) A[j+i*N] = A[j+i*N]*g - A[j+k*N]*f;
				for (j = 0; j < N; j++) B[j+i*N] = B[j+i*N]*g - B[j+k*N]*f;
			}
		}
	}
	return 0;
}





// returns 0 if rejected by lex-leader or no-block constraint
// returns 1 if success
// returns -1 if excess bits
int fill(long l, int check_lex) {
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
	if (check_lex==1) {
		// check if lex minimal
		for (int i=0; i<N; i++) { // row i
			for (int j=0; j<i; j++) { // row j
				int ii = 0;
				int jj = 0;
				for (int k=N-1; k>=0; k--) { // across columns
					if ((k!=i) && (k!=j)) {
						ii = ii * 2 + A[k*N+i];
						jj = jj * 2 + A[k*N+j];
					}
				}
				if (jj>ii) {
					return 0;
				}
			}
		}
		for (int k=0; k<N-1; k++) {
			int is_block_k = 1;
			for (int col = k+1; col<N; col++) {
				for (int row = 0; row < k+1; row++) {
					if (A[row*N+col]==1) {
						is_block_k = 0;
					}
				}
			}
			if (is_block_k==1)
				return 0;
		}
	}
	
	// if excess bits, then return -1;
	if (l!=0)
		return -1;
	return 1;
}

// returns -1 if excess bits
// returns -2 if invalid by lex-leader or no-block constraints
// returns 1 if singular or if not local maxima
// returns 2 if local maxima, success
int run(long l) {
	int f = fill(l,1);
	for (int x=0; x<N; x++) {
		for (int y=0; y<N; y++) {
			printf("%i ",A[y*N+x]);
		}
		printf("\n");
	}
	if (f==-1) return -1;
	if (f==0) return -2;
	// do gauss jordan elimination to create the identity matrix
	int singular = gjinv();
	if (singular==0) {
		fill(l,0);
		for (int x=0; x<N; x++)
			for (int y=0; y<N; y++)
				if ( ((A[y*N+x]==0) && (B[y*N+x] > 0))   // if we can only increasing the bit and such an increase would increase the determinant, then fail
				  || ((A[y*N+x]==1) && (B[y*N+x] < 0)) ) // if we can only decrease the bit, and such a decrease would cause an increase in determinant, then fail
					return 1;
		return 2;
	}
	return 1;
}

long nondet_long();
int main() {
	long l = 51;//nondet_long();
	int r = run(l);
	//__CPROVER_assert(r==2, "Determinant Assert");
}
