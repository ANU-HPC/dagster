

#define SWAP(A,B) { \
	auto temp = B; \
	B = A; \
	A = temp; \
}
#define ABS(A) A<0 ? -A : A
#define N 7


#include "stdio.h"
#include <math.h>
int gjinv (int *a, int *b)
{
	int i, j, k, p;
	int f, g;
	for (i = 0; i < N; i++)  /* Set b to identity matrix. */
		for (j = 0; j < N; j++)
			b[j+i*N] = (i == j) ? 1 : 0;
	for (k = 0; k < N; k++) {  /* Main loop */
		f = ABS(a[k+k*N]);  /* Find pivot. */
		p = k;
		for (i = k+1; i < N; i++) {
			g = ABS(a[k+i*N]);
			if (g > f) {
				f = g;
				p = i;
			}
		}
		if (f == 0) return 1;  /* Matrix is singular. */
		if (p != k) {  /* Swap rows. */
			for (j = k; j < N; j++) SWAP(a[j+k*N],a[j+p*N])
			for (j = 0; j < N; j++) SWAP(b[j+k*N],b[j+p*N])
			printf("swapping row %i with %i\n",k,p);
			for (int y=0; y<N; y++) {
				for (int x=0; x<N; x++)
					printf("%i ",(int)(a[y*N+x]));
				printf("\t\t");
				for (int x=0; x<N; x++)
					printf("%i ",(int)(b[y*N+x]));
				printf("\n");
			}
		}
		if (a[k+k*N]<0) {
			for (j = k; j < N; j++) a[j+k*N] *= -1;
			for (j = 0; j < N; j++) b[j+k*N] *= -1;
		}
		for (i = 0; i < N; i++) {  /* Subtract to get zeros. */
			//if (i<k) continue;
			if (i == k) continue;
			f = a[k+i*N];
			g = a[k+k*N];
			if (f!=0) {
				for (j = 0; j < N; j++) a[j+i*N] = a[j+i*N]*g - a[j+k*N]*f;
				for (j = 0; j < N; j++) b[j+i*N] = b[j+i*N]*g - b[j+k*N]*f;
				printf("eliminating rows %i and %i\n",i,k);
				for (int y=0; y<N; y++) {
					for (int x=0; x<N; x++)
						printf("%i ",(int)(a[y*N+x]));
					printf("\t\t");
					for (int x=0; x<N; x++)
						printf("%i ",(int)(b[y*N+x]));
					printf("\n");
				}
			}
		}
	}
	return 0;
}







int A[N*N];
int B[N*N];

int fill(long l, bool check_lex) {
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
				if (A[row*N+col]==1) {
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
			printf("%i ",A[y*N+x]);
		printf("\n");
	}
	return 0;
}

int count=0;

int run(long l) {
	int f = fill(l,true);
	if (f==-1) return -1;
	if (f==0) return -2;
		printf("matrix %li:\n",l);
		for (int y=0; y<N; y++) {
			for (int x=0; x<N; x++)
				printf("%i ",(int)(A[y*N+x]));
			printf("\n");
		}
	// do gauss jordan elimination to create the identity matrix
	int singular = gjinv (A, B);
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
	long l = 2095954;
	double ret=0;
	while (ret!=-1) {
		ret = run(l);
		l++;
		break;
	}
	printf("Number of matricies: %i\n",count);
}
