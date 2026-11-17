/*
 -  Name: Jeffrey Gonzalez
 -  Date Created: 11/15/2026
 -  Last Edited: 11/16/2026
 -  Class: CS 465
 -  Code: POSIX without blocking
 * 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ROWS 1024
#define COLS 1024

int **A, **B, **C;

// Structure to pass row boundaries to each thread
typedef struct {
    int start_row;
    int end_row;
} ThreadArg;

// The POSIX thread worker mimicking the standard naive kernel
void *matmul_worker(void *arg) {
    ThreadArg *t_arg = (ThreadArg *)arg;
    
    for (int i = t_arg->start_row; i < t_arg->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = 0;
            for (int k = 0; k < COLS; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // 1. Read thread count from command line arguments
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }

    // Dynamically allocate 1024x1024 matrices
    A = (int **)malloc(ROWS * sizeof(int *));
    B = (int **)malloc(ROWS * sizeof(int *));
    C = (int **)malloc(ROWS * sizeof(int *));
    for (int i = 0; i < ROWS; i++) {
        A[i] = (int *)malloc(COLS * sizeof(int));
        B[i] = (int *)malloc(COLS * sizeof(int));
        C[i] = (int *)malloc(COLS * sizeof(int));
    }

    // Initialize matrix elements with rand() and seed 10
    srand(10);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = rand();
            B[i][j] = rand();
        }
    }

    // 2. Use variable-length arrays based on the command-line input
    pthread_t threads[num_threads];
    ThreadArg t_args[num_threads];
    struct timespec start, end;
    
    // Divide the 1024 rows among the threads
    int chunk_size = ROWS / num_threads;

    // Start timer just before the kernel execution begins
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create and launch threads
    for (int i = 0; i < num_threads; i++) {
        t_args[i].start_row = i * chunk_size;
        // Ensure the final thread processes any remaining rows if ROWS isn't perfectly divisible
        t_args[i].end_row = (i == num_threads - 1) ? ROWS : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, matmul_worker, (void *)&t_args[i]);
    }

    // Wait for all threads to finish computing
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // End timer immediately after threads return
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time in seconds
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    // 3. Format output simply as "Threads, Time" so the bash script can redirect it cleanly to a CSV
    printf("%d, %f\n", num_threads, time_taken);

    // Free memory
    for(int i = 0; i < ROWS; i++) {
        free(A[i]); free(B[i]); free(C[i]);
    }
    free(A); free(B); free(C);

    return 0;
}
