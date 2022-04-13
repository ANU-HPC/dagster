#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int n=13;
int data[n];
int deltas[n*n];
clock_t tStart;

void print() {
	printf("Numbers: ");
	for (int i=0; i<n; i++)
		printf("%i ",data[i]);
	printf("\nPlot:\n");
	for (int y=0; y<n; y++) {
		for (int x=0; x<n; x++) {
			if (data[x] == y) {
				printf("X");
			} else {
				printf("_");
			}
		}
		printf("\n");
	}
	printf("\nDifference Pyramid:\n");
	for (int d=1; d<n; d++) {
		for (int i=0; i+d<n; i++) {
			printf("%i ",data[i+d]-data[i]);
		}
		printf("\n");
	}
}

void run(int upto) {
	for (int upto1 = upto; upto1 < n; upto1++) {
		continue_label:
		int dataupto1 = data[upto1];
		//if (upto < 5) {
		//	printf("updating level %i to value %i\n",upto,dataupto1);
		//}
		for (int i=0; i<upto; i++) {
			int displacement = dataupto1 - data[i];
			deltas[i*n+(upto-i)] = displacement;
			for (int j=0; j<i; j++) {
				if (deltas[j*n+(upto-i)] == displacement) {
					upto1++;
					if (upto1 >= n)
						return;
					goto continue_label;
				}
			}
		}
		if (upto == n-1) {
			//printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
			print();
			//exit(0);
		} else {
			int temp = dataupto1;
			data[upto1] = data[upto];
			data[upto] = temp;
			run(upto+1);
			temp = data[upto1];
			data[upto1] = data[upto];
			data[upto] = temp;
		}
	}
}

int main(int argc, char **argv) {
	printf("running costas for %i\n",n);
	tStart = clock();
	for (int i = 0; i<n; i++)
		data[i] = i;
	run(0);
	printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
}


