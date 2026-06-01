#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cg.h"

// Solve Ax = b for x, using the Conjugate Gradient method.
// Terminates once the maximum number of steps or tolerance has been reached
double *conjugate_gradient_serial(double *A, double *b, int N, int max_steps, double tol) {
  /* PUT OR MODIFY YOUR SERIAL CODE IN THIS FUNCTION*/
  
  double *x, *r, *p, *omega;
  double rho, rho_new, alpha, beta, temp;

  x     = calloc(N, sizeof(double));
  r     = malloc(N * sizeof(double));
  p     = malloc(N * sizeof(double));
  omega = malloc(N * sizeof(double));

  malloc_test(x);
  malloc_test(r);
  malloc_test(p);
  malloc_test(omega);

  for (int i = 0; i < N; i++) {
    r[i] = b[i];
    p[i] = r[i];
  }
  rho = 0.0;
  for (int i = 0; i < N; i++)
    rho += r[i] * r[i];

  for (int k = 0; k < max_steps; k++) {

    if (sqrt(rho) < tol) break;

    for (int i = 0; i < N; i++) {
      omega[i] = 0.0;
      for (int j = 0; j < N; j++)
        omega[i] += A[i * N + j] * p[j];
    }

    temp = 0.0;
    for (int i = 0; i < N; i++)
      temp += p[i] * omega[i];
    alpha = rho / temp;

    for (int i = 0; i < N; i++)
      x[i] += alpha * p[i];

    for (int i = 0; i < N; i++)
      r[i] -= alpha * omega[i];

    rho_new = 0.0;
    for (int i = 0; i < N; i++)
      rho_new += r[i] * r[i];

    if (sqrt(rho_new) < tol) break;

    beta = rho_new / rho;

    for (int i = 0; i < N; i++)
      p[i] = r[i] + beta * p[i];

    rho = rho_new;
  }

  free(r);
  free(p);
  free(omega);

  return x;
  /* PUT OR MODIFY YOUR SERIAL CODE IN THIS FUNCTION*/
}


void conjugate_gradient_parallel(process_data row, equation_data equation, int N, int max_steps, double tol) {
  /* PUT OR MODIFY YOUR PARALLEL CODE IN THIS FUNCTION*/
  
  double *x, *r, *p, *p_send, *p_recv, *omega;
  double alpha, alpha_tmp, beta, rho, rho_new, rho_tmp;
  int rank_up, rank_down, rank_block, displ_send, count_send;
  MPI_Request send_req;

  x = &(equation.x_star[0]);

  r      = malloc(row.count     * sizeof(double));
  p      = malloc(row.count     * sizeof(double));
  p_send = malloc(row.count_max * sizeof(double));
  p_recv = malloc(row.count_max * sizeof(double));
  omega  = malloc(row.count     * sizeof(double));

  malloc_test(r);
  malloc_test(p);
  malloc_test(p_send);
  malloc_test(p_recv);
  malloc_test(omega);

  rank_up   = row.ranks[(row.Np + row.coord - 1) % row.Np];
  rank_down = row.ranks[(row.coord + 1)           % row.Np];

  rho_tmp = 0.0;
  for (int i = 0; i < row.count; i++) {
    x[i]     = 0.0;
    r[i]     = equation.b[i];
    p[i]     = r[i];
    rho_tmp += r[i] * r[i];
  }
  MPI_Allreduce(&rho_tmp, &rho, 1, MPI_DOUBLE, MPI_SUM, row.comm);

  for (int k = 0; k < max_steps; k++) {

    if (sqrt(rho) < tol) break;

    for (int i = 0; i < row.count; i++)
      omega[i] = 0.0;

    memcpy(p_send, p, row.count * sizeof(double));

    for (int m = 0; m < row.Np; m++) {

      if (m < row.Np - 1)
        MPI_Isend(p_send, row.count_max, MPI_DOUBLE,
                  rank_up, 222, row.comm, &send_req);

      rank_block = row.ranks[(row.coord + m) % row.Np];
      displ_send = row.displs[rank_block];
      count_send = row.counts[rank_block];

      for (int i = 0; i < row.count; i++) {
        double *a_row = equation.A + (i * N + displ_send);
        for (int j = 0; j < count_send; j++)
          omega[i] += p_send[j] * a_row[j];
      }

      if (m < row.Np - 1) {
        MPI_Recv(p_recv, row.count_max, MPI_DOUBLE,
                 rank_down, 222, row.comm, MPI_STATUS_IGNORE);
        MPI_Wait(&send_req, MPI_STATUS_IGNORE);

        double *tmp = p_send;
        p_send = p_recv;
        p_recv = tmp;
      }
    }

    alpha_tmp = 0.0;
    for (int i = 0; i < row.count; i++)
      alpha_tmp += p[i] * omega[i];
    MPI_Allreduce(&alpha_tmp, &alpha, 1, MPI_DOUBLE, MPI_SUM, row.comm);
    alpha = rho / alpha;

    for (int i = 0; i < row.count; i++)
      x[i] += alpha * p[i];

    for (int i = 0; i < row.count; i++)
      r[i] -= alpha * omega[i];

    rho_tmp = 0.0;
    for (int i = 0; i < row.count; i++)
      rho_tmp += r[i] * r[i];
    MPI_Allreduce(&rho_tmp, &rho_new, 1, MPI_DOUBLE, MPI_SUM, row.comm);

    if (sqrt(rho_new) < tol) break;

    beta = rho_new / rho;
    for (int i = 0; i < row.count; i++)
      p[i] = r[i] + beta * p[i];
    rho = rho_new;
  }

  free(r);
  free(p);
  free(p_send);
  free(p_recv);
  free(omega);

  /* PUT OR MODIFY YOUR PARALLEL CODE IN THIS FUNCTION*/
}
