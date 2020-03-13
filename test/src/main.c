#include <stdio.h>
#include <strings.h>
#include "/home/khadimou/Images/Allocateur_Memoire/protos.h"
#include <pthread.h>
#include <sys/mman.h>
#include <math.h>
#include "/home/khadimou/Images/Allocateur_Memoire/rdtsc.h"
#include "test.h"

#define nthreads 3
#define  ALIGN 16

#define CPU_FREQ 1.8

struct mes_args_t
{
	char *str;
};

static void *run_test_thread(void *ret) {
    ret = malloc(80);
    struct mes_args_t *args = (struct mes_args_t*)ret;
    args->str = "Hello world !\n";
    printf("%s \n", args->str);
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

    for(i = 0; i < nthreads; i++) {
        before = rdtsc();

        pthread_t *thread = malloc(sizeof(pthread_t) * nthreads);
        void *ptr = NULL;

        if (pthread_create(thread + i, NULL, run_test_thread, ptr)) {
            printf("Error pthread");
            return EXIT_FAILURE;
             	}
        if (pthread_join(thread[i], NULL)) {
            printf("error pthread join");
            return EXIT_FAILURE;
             }

        run_test_malloc();
        run_test_realloc();
        run_test_mixed();
        // alignement sur 16 octects
        thread = aligned_malloc(ALIGN,16);

        //run_test_rlimit(); // Always at the end
        aligned_free(thread);
        free(ptr);
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