#include <stdio.h> 
#include <stdlib.h>
#include <time.h>

#define KB 1024
#define MB 1024 * 1024

int main() {
    unsigned int steps = KB * MB;
    static int arr[4 * MB];
    int lengthMod;
    unsigned int i;
    double timeTaken;
    clock_t start;
    int sizes[] = { 
        1 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB,
        512 * KB, 1 * MB, 1.5 * MB, 2 * MB, 2.5 * MB, 3 * MB, 3.5 * MB, 4 * MB
    };
    int results[sizeof(sizes) / sizeof(int)];
    int s;

    // for each size
    for (s = 0; s < sizeof(sizes)/sizeof(int); s++) {
	    lengthMod = sizes[s] - 1;
	    start = clock();
	    for (i = 0; i < steps; i++) {
	        arr[(i * 16) & lengthMod] *= 10;
            arr[(i * 16) & lengthMod] /= 10;
	    }

	    timeTaken = (double)(clock() - start)/CLOCKS_PER_SEC;
        printf("Step: %d, time: %.8f \n", sizes[s] / 1024, timeTaken);
    }

    return 0;
}
