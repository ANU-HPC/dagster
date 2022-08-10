
// https://rosettacode.org/wiki/Gauss-Jordan_matrix_inversion#C
/*----------------------------------------------------------------------
gjinv - Invert a matrix, Gauss-Jordan algorithm
A is destroyed.  Returns 1 for a singular matrix.
 
___Name_____Type______In/Out____Description_____________________________
   a[n*n]   double*   In        An N by N matrix
   n        int       In        Order of matrix
   b[n*n]   double*   Out       Inverse of A
----------------------------------------------------------------------*/
#include <math.h>
int gjinv (double *a, int n, double *b)
{
	int i, j, k, p;
	double f, g, tol;
	if (n < 1) return -1;  /* Function Body */
	f = 0.;  /* Frobenius norm of a */
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			g = a[j+i*n];
			f += g * g;
		}
	}
	f = sqrt(f);
	tol = f * 2.2204460492503131e-016;
	for (i = 0; i < n; ++i) {  /* Set b to identity matrix. */
		for (j = 0; j < n; ++j) {
			b[j+i*n] = (i == j) ? 1. : 0.;
		}
	}
	for (k = 0; k < n; ++k) {  /* Main loop */
		f = fabs(a[k+k*n]);  /* Find pivot. */
		p = k;
		for (i = k+1; i < n; ++i) {
			g = fabs(a[k+i*n]);
			if (g > f) {
				f = g;
				p = i;
			}
		}
		if (f <= tol) return 1;  /* Matrix is singular. */
		if (p != k) {  /* Swap rows. */
			for (j = k; j < n; ++j) {
				f = a[j+k*n];
				a[j+k*n] = a[j+p*n];
				a[j+p*n] = f;
			}
			for (j = 0; j < n; ++j) {
				f = b[j+k*n];
				b[j+k*n] = b[j+p*n];
				b[j+p*n] = f;
			}
		}
		f = 1. / a[k+k*n];  /* Scale row so pivot is 1. */
		for (j = k; j < n; ++j) a[j+k*n] *= f;
		for (j = 0; j < n; ++j) b[j+k*n] *= f;
		for (i = 0; i < n; ++i) {  /* Subtract to get zeros. */
			if (i == k) continue;
			f = a[k+i*n];
			if (f!=0) {
				for (j = k; j < n; ++j) a[j+i*n] -= a[j+k*n] * f;
				for (j = 0; j < n; ++j) b[j+i*n] -= b[j+k*n] * f;
			}
		}
	}
	return 0;
} /* end of gjinv */



#include "stdio.h"
#define N 8





double A[N*N];
double B[N*N];
long C[N];



void special(int i, int* b, int n) {
	int upto=0;
	
	for int
}




//  0100100111
//  1001001001
//  0101001011


int fill(long l, bool check_lex) {
	// fill matrix from bits from l
	for (int i=N-1; i>=0; i--) {
		

		
		if (i+1<N) {
			long upper = (C[i+1] >> (i+2)) << (i+2);
			long lower = C[i+1]- (C[i+1] >> i) << i;
			long c_upper = (C[i] >> (i+2)) << (i+2);
			
			long new_lower = l % 
			long new_cc = C[i+1] - (C[i+1] & (1<<i)) - (C[i+1] & (1<<(i+1)));
			if (new_cc>cc)
				cc = new_cc;
			new_cc += ((C[i+1] & (1<<i))<<1);
		}
		for (int j=i, j<N; j++)
			if ((cc==-1) || (cc < C[j]))
				cc = C[j];
		if (cc&(1<<i) !=0) // set the i'th bit of cc to be zero if it isnt already
			cc -= (1<<i);
		
		
		for (int j=0; j<i; j++) {
			int ll = l&1;
			l = l >> 1;
			A[i*N+j] = ll;
			A[j*N+i] = ll;
		}
		A[i*N+i] = 0;
	}
	if (check_lex) {
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
	}
	for (int k=0; k<N-1; k++) {
		bool is_block_k = true;
		for (int col = k+1; col<N; col++) {
			for (int row = 0; row < k+1; row++) {
				if (A[row*N+col]==1.) {
					is_block_k = false;
				}
			}
		}
		if (is_block_k)
			return 0;
	}
	
	// if excess bits, then return -1;
	if (l!=0)
		return -1;
	return 1;
}

int draw(long l) {
	if (fill(l,true)==-1) return -1;
	printf("Matrix is:\n");
	for (int y=0; y<N; y++) {
		for (int x=0; x<N; x++)
			printf("%i ",A[y*N+x]==1.0 ? 1 : 0);
		printf("\n");
	}
	return 0;
}

int count=0;

int run(long l) {
	int f = fill(l,true);
	if (f==-1) return -1;
	if (f==0) return -2;
	// do gauss jordan elimination to create the identity matrix
	int singular = gjinv (A, N, B);
	if (singular==0) {
		fill(l,false);
		for (int x=0; x<N; x++) {
			for (int y=0; y<N; y++) {
				if ( ((A[y*N+x]==0) && (B[y*N+x] > 0)) // if we can only increasing the bit and such an increase would increase the determinant, then fail
				  || ((A[y*N+x]==1) && (B[y*N+x] < 0)) ) { // if we can only decrease the bit, and such a decrease would cause an increase in determinant, then fail
					return 1;
				}
			}
		}
		printf("matrix %li:\n",l);
		for (int y=0; y<N; y++) {
			for (int x=0; x<N; x++)
				printf("%i ",(int)(A[y*N+x]));
			printf("\n");
		}
		/*printf("inverse matrix:\n");
		for (int y=0; y<N; y++) {
			for (int x=0; x<N; x++)
				printf("%f ",B[y*N+x]);
			printf("\n");
		}*/
		count += 1;
		return 2;
	}
	return 1;
}


int main() {
	long l = 0;
	double ret=0;
	while (ret!=-1) {
		ret = run(l);
		l++;
	}
	printf("Number of matricies: %i\n",count);
}
