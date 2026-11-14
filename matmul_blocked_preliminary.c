#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define BLOCK 64

// given code
void matmul_blocked(double *A, double *B, double *C, int n) {
         for (int i = 0; i < n; i += BLOCK) {
                 for (int j = 0; j < n; j += BLOCK) {
                         for (int k = 0; k < n; k += BLOCK) {
                                 for (int ii = i; ii < i + BLOCK && ii < n; ii++) {
                                         for (int jj = j; jj < j + BLOCK && jj < n; jj++) {
                                                 double sum = C[ii*n + jj];
                                                 for (int kk = k; kk < k + BLOCK && kk < n; kk++) {
                                                         sum += A[ii * n + kk] * B[kk * n + jj];
                                                 }
                                                 C[ii * n + jj] = sum;
                                         }
                                 }
                         }
                 }
         }
}

int main() {

         // allocate memory for matrix multi
         double *A = malloc(N * N * sizeof(double));
         double *B = malloc(N * N * sizeof(double));
         double *C = malloc(N * N * sizeof(double));

         // print if failure to get memory
         if (A == NULL || B == NULL || C == NULL) {
                 printf("mem allocation failed\n");

                 free(A);
                 free(B);
                 free(C);

                 return 1;
         }

         // set the rand to 10
         srand(10);

         // allocate the rand numbers
         for (int i = 0; i < N * N; i++) {
                 A[i] = rand();
                 B[i] = rand();
                 C[i] = 0.0;
         }

         // create the struct to record the start and end time for the matrix multiplication
         struct timespec start, end;

         // get the start time
         clock_gettime(CLOCK_MONOTONIC, &start);

         matmul_blocked(A, B, C, N);

         // get the end time
         clock_gettime(CLOCK_MONOTONIC, &end);

         // calculate the time it takes for the matrix multi to finish
         // .tv_sec gets the whole seconds and .tv_nsec gets the nanoseconds then we divide by
         // 10^9 to convert them into seconds
         //
         // tldr - we get the total execution time
         double timeOfClock = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

         // print the results
         printf("Block size: %d\n", BLOCK);
         printf("Exec Time: %.6f seconds\n", timeOfClock);

         // free the allocated memory
         free(A);
         free(B);
         free(C);

         return 0;
}