/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */

/*
*  Name: Zehao Wang StudentID:518021910976
*/
#include <stdio.h>

#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, ii, jj;
  int a, b, c, d, e, f, g, h;
  switch (M) {
    case 32:
      for (ii = 0; ii < N; ii += 8) {
        for (jj = 0; jj < M; jj += 8) {
          for (i = ii; i < ii + 8 && i < N; i++) {
            for (j = jj; j < jj + 8 && j < M; j++)
              if (i != j)
                B[j][i] = A[i][j];
              else {
                a = A[i][j];
                b = i;
              }
            if (ii == jj) B[b][b] = a;
          }
        }
      }
      break;
    case 61:
      for (ii = 0; ii < N; ii += 16) {
        for (jj = 0; jj < M; jj += 16) {
          for (i = ii; i < ii + 16 && i < N; i++) {
            for (j = jj; j < jj + 16 && j < M; j++)
              if (i != j)
                B[j][i] = A[i][j];
              else {
                a = A[i][j];
                b = i;
              }
            if (ii == jj) B[b][b] = a;
          }
        }
      }
      break;
    case 64:
      for (ii = 0; ii < M; ii += 8) {
        for (jj = 0; jj < M; jj += 8) {
          for(i=ii;i<ii+4;i++){
            a=A[i][jj];b=A[i][jj+1];c=A[i][jj+2];d=A[i][jj+3];
            e=A[i][jj+4];f=A[i][jj+5];g=A[i][jj+6];h=A[i][jj+7];
            B[jj][i]=a;B[jj][i+4]=e;B[jj+1][i]=b;B[jj+1][i+4]=f;
            B[jj+2][i]=c;B[jj+2][i+4]=g;B[jj+3][i]=d;B[jj+3][i+4]=h;
          }
          for(i=jj;i<jj+4;i++){
            a=B[i][ii+4];b=B[i][ii+5];c=B[i][ii+6];d=B[i][ii+7];
            e=A[ii+4][i];f=A[ii+5][i];g=A[ii+6][i];h=A[ii+7][i];
            B[i][ii+4]=e;B[i][ii+5]=f;B[i][ii+6]=g;B[i][ii+7]=h;
            B[i+4][ii]=a;B[i+4][ii+1]=b;B[i+4][ii+2]=c;B[i+4][ii+3]=d;
          }
          for(j=jj+4;j<jj+8;j++){
            a=A[ii+4][j];b=A[ii+5][j];c=A[ii+6][j];d=A[ii+7][j];
            B[j][ii+4]=a;B[j][ii+5]=b;B[j][ii+6]=c;B[j][ii+7]=d;
          }
        }
      }
      break;
    default:
      trans(M, N, A, B);
  }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, tmp;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      tmp = A[i][j];
      B[j][i] = tmp;
    }
  }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}
