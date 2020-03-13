#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "rdtsc.h"

#include "./test/src/test.h"

#define nthreads 3

#define ALIGN 128

#define CPU_FREQ 1.8


static void *run_test_thread() {
    void *ret = malloc(80);
    pthread_exit(NULL);
    return ret;
}

int main(int argc, char *argv[])
{
    double before,after;
    int i, count = 100;
    int n = 10;
    unsigned long long s = sizeof(double)* n*n;
    double min,max,med,avg,bpc,gbps;
    double cycle[count];

    for(i = 0; i < count; i++) {
        before = rdtsc();

        pthread_t *thread = malloc(sizeof(pthread_t) * count);
        void *ptr = NULL;

        if (pthread_create(thread + i, NULL, run_test_thread, ptr)) {
            printf("Error pthread");
            return EXIT_FAILURE;
        }
        if (pthread_join(thread[i], NULL)) {
            printf("error pthread join");
            return EXIT_FAILURE;
        }

        void* new = realloc(ptr,100);
        ptr = aligned_alloc(ALIGN,16);
        printf("new %p\n",new);

        run_test_malloc();
        run_test_realloc();
        run_test_mixed();
        // alignement sur 16 octects
        thread = aligned_alloc(ALIGN,16);
        printf("thread %p\n",thread);

        free(thread);
        free(new);
        after = rdtsc();
        cycle[i] = (after - before)/count;
    }
    min = cycle[0];
    max = cycle[count-1];
    avg = (min + max) / 2.;
    med = (cycle[count>>1] + cycle[count>>1] + 1)/2.;

    s *= 3;
    bpc = s / med;
    gbps = s / (med/CPU_FREQ);

    //(B, KiB, MiB, GiB), min, max, avg, med, bpc, gbps
    printf("\t(%10llu, %10llu, %10llu, %llu);\t%20.10lf;\t%20.10lf;\t%20.10lf;\t%20.10lf;\t%20.10lf;\t%20.0lf GiB/s;\n\n",
           s, s >> 10, s >> 20, s >> 30,
           min, max, avg, med, bpc, roundf(gbps));

    return 0;
}