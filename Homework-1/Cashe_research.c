#include <stdio.h>
#include <math.h>
#include <intrin.h>
#include <windows.h>

/* Array size */
int step(int m) {
    if (m < 1024) m = m * 2;
    else if (m < 4 * 1024) m += 1024;
    else {
        int s;
        for (s = 32 * 1024; s <= m; s *= 2);
        m += s / 16;
    }
    return m;
}

#define twice(x) x x
#define mb (1024*1024)

int main() {
    /* Finding clock frequency */
    LARGE_INTEGER perfcnt1, perfcnt2;
    __int64 tsc1, tsc2;
    QueryPerformanceCounter(&perfcnt1);
    tsc1 = __rdtsc();

    /* Load */
    Sleep(250);

    /* Measuring */
    QueryPerformanceCounter(&perfcnt2);
    tsc2 = __rdtsc();
    perfcnt2.QuadPart -= perfcnt1.QuadPart;
    QueryPerformanceFrequency(&perfcnt1);

    /* Check result */
    const double MHz = (double) (tsc2 - tsc1) * (long) perfcnt1.QuadPart / (long) perfcnt2.QuadPart / 1e6;
    printf("Clock rate: %.0f MHz\n", MHz);

    /* Time to access memory */
    typedef struct segment_t segment;
    struct segment_t {
        int size_l, size_r;         // size of left and right edges
        double level, total;        // time to access in loops
        int width;                  // width in marked points
        segment *next;
    };

    /* With a constant step size */
    typedef struct {
        int step_size_bytes;
        segment data;
    }
            segments;

    /* Setting five step values */
    segments allsegs[] = {{256},{512},{1024},{2048},{4096},{0}};

    int step_size;
    for (segments * cursegs = allsegs;
        int step_size = cursegs->step_size_bytes / sizeof(void *);
        cursegs++) {
        printf("\rTesting stride: %d \n", cursegs->step_size_bytes);

        int iters = 1 << 28;                        // Call to array in every pass
        int state = 0;                              // Initial state
        segment *curseg = &(cursegs->data);         // Current segment

        /* Before two array sizes and the results on them */
        int a_size_bytes, b_size_bytes;
        double a, b;

        /* At each iteration given more of the memory */
        for (int arr_size_bytes = 1 << 12; arr_size_bytes <= 1 << 29;
             arr_size_bytes = step(arr_size_bytes)) {
            const int arr_size = arr_size_bytes / sizeof(void *);
            void **x = (void **) malloc(arr_size_bytes);              // Allocate the memory

            // Filling the memory with pointers in step_size */
            int m;
            for (m = 0; m < arr_size; m += step_size) {
                x[m] = x + m + step_size;
            }
            x[m - step_size] = x;
            const int arr_iters = m / step_size;                    // Number of the filled array elements

            /* At least four complete passes through the array */
            if (iters < 4 * arr_iters) iters = 4 * arr_iters;

            /* Pointer to access the array */
            void **p = x;

            // Counter before executing commands
            const __int64 ticks_start = __rdtsc();

            // Main cycle, measure it's execution time
            for (int j = 0; j < iters; j += 256) {
                twice(twice(twice(twice(twice(twice(twice(twice(p = (void **) *p;))))))))
            }

            /* Counter after executing commands */
            const __int64 ticks_end = __rdtsc();

            /* Number of processor cycles spent (on average per request)
             * multiply by !!p (one) so that the optimizer doesn't throw it out as unused */
            const double cycles = (double) !!p * (ticks_end - ticks_start) / iters;

            /* Showing results */
            printf("\r%f mb - %.2f cycles ", (double) arr_size_bytes / mb, cycles);

            free(x);                      // Free memory

            /* Correct number of iterations so that each pass take less than a second */
            while (cycles / MHz * iters > 1e6) iters >>= 1;

            /* Left side step */
            if (!state && (curseg->width > 2) && (fabs(a - curseg->level) < (a * .1)) &&
                (b > (curseg->level * 1.1)) && (cycles > (curseg->level * 1.1))) {
                curseg->size_r = a_size_bytes;
                curseg = curseg->next = (segment *) calloc(1, sizeof(segment));
                state = 1;
                b = 0;                    // Right side need to be "to the right" of the left side
            }
            /* Right side step */
            if (state && (fabs(cycles - a) < (a * .1)) && (fabs(cycles - b) < (b * .1))) {
                curseg->size_l = a_size_bytes;
                state = 0;
            }
            /* Always take into account the first two points */
            if (!state && ((curseg->width <= 2) || (fabs(cycles - curseg->level) < cycles * .1))) {
                curseg->total += cycles;
                curseg->width++;
                curseg->level = curseg->total / curseg->width;
            }
            /* Shifting */
            a_size_bytes = b_size_bytes;
            b_size_bytes = arr_size_bytes;
            a = b;
            b = cycles;
        }
    }
    return 0;
}
