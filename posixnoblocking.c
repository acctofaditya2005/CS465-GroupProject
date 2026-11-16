/*
 * Name: Jeffrey Gonzalez
 * Date Created: 11/15/2026
 * Last Edited: 11/15/2026 @ 1:13 AM
 * Class: CS 465
 * Code: POSIX without blocking
 * 
 *
 */



#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define ROWS 1024
#define COLS 1024

//globals for matrix setups , [A] * [B] = [C]
int **A, **B, **C;

//number of threads , this will change
#define NUM_THREADS 4

void *matmul(void *args){

	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			C[i][j] = 0;
			for (int k = 0; k < COLS; k++) {
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
}



int main() {
	

	//CREATING THREE EMPTY MATRICES 1024x1024
	A = (int **)malloc(ROWS * sizeof(int *));
	B = (int **)malloc(ROWS * sizeof(int *));
	C = (int **)malloc(ROWS * sizeof(int *));
	
	for(int i = 0; i<ROWS; i++) {
		A[i] = (int *)malloc(COLS * sizeof(int));
		B[i] = (int *)malloc(COLS * sizeof(int));
		C[i] = (int *)malloc(COLS * sizeof(int));
	}

	//INITIALIZING TWO MATRICES WITH RANDOM VALUES
	srand(10)
	for(i = 0; i<ROWS; i++) {
		for(j = 0; j < COLS; j++) {
			A[i][j] = rand()
			B[i][j] = rand()
		}
	}

	//INITIALIZING THREADS
	pthread_t threads[NUM_THREADS];
	int threadsarg[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; i++) {
		threadsarg[i] = i; //giving each thread it's own slot in mem
		
		int result = pthread_create(&threads[i], NULL, matmul, &threadarg[i]);
		
		if (result != 0) {
			perror("Failure to create thread");
			return 1;
		}

		for (int i = 0; i < NUM_THREADS; i++) {
			pthread_join(threads[i],NULL);
		}	
	}


	//logic for dividing the rows into chunks for threading

	return 0;
			


}
