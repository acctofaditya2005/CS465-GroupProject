#define _POSIX_C_SOURCE 199309L
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef N
#define N 1024
#endif
// each thread handles different output rows, so there are no write conflicts
static void matmul_openmp(const double *A, const double *B, double *C, int n, int num_threads) {
    #pragma omp parallel for num_threads(num_threads) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}
// a simple single-threaded reference for optional correctness checking
static void matmul_reference(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}
static double elapsed_seconds(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}
int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <thread_count> [--verify]\n", argv[0]);
        return 1;
    }
    char *endptr;
    long requested_threads = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || requested_threads < 1 || requested_threads > 256) {
        fprintf(stderr, "Thread count must be an integer between 1 and 256.\n");
        return 1;
    }
    int verify = 0;
    if (argc == 3) {
        if (strcmp(argv[2], "--verify") != 0) {
            fprintf(stderr, "Unknown option: %s\n", argv[2]);
            return 1;
        }
        verify = 1;
    }
    size_t elements = (size_t)N * N;
    double *A = malloc(elements * sizeof(double));
    double *B = malloc(elements * sizeof(double));
    double *C = malloc(elements * sizeof(double));
    double *reference = verify ? malloc(elements * sizeof(double)) : NULL;
    if (!A || !B || !C || (verify && !reference)) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(A); free(B); free(C); free(reference);
        return 1;
    }
    srand(10);
    for (size_t i = 0; i < elements; i++) {
        A[i] = rand();
        B[i] = rand();
        C[i] = 0.0;
        if (verify) {
            reference[i] = 0.0;
        }
    }
    // respect the number of threads requested on the command line
    omp_set_dynamic(0);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    matmul_openmp(A, B, C, N, (int)requested_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long double checksum = 0.0L;
    for (size_t i = 0; i < elements; i++) {
        checksum += C[i];
    }
    printf("Matrix size: %d x %d\n", N, N);
    printf("Threads: %ld\n", requested_threads);
    printf("Execution time: %.6f seconds\n", elapsed_seconds(start, end));
    printf("Checksum: %.9Le\n", checksum);
    if (verify) {
        matmul_reference(A, B, reference, N);
        size_t mismatches = 0;
        for (size_t i = 0; i < elements; i++) {
            if (C[i] != reference[i]) mismatches++;
        }
        printf("Reference validation: %s (%zu mismatches)\n",
               mismatches == 0 ? "PASS" : "FAIL", mismatches);
        if (mismatches != 0) {
            free(A); free(B); free(C); free(reference);
            return 2;
        }
    }
    free(A); free(B); free(C); free(reference);
    return 0;
}