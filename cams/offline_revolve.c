#include <stdio.h>

typedef struct _revolve_ctx *rev;
struct _revolve_ctx {
  int *checkpoint;
  int max_size;
  int num_checkpoints_stored;
};

/*
  This function predicts where to store the next checkpoint.
  The decision is made when
  1. current step is stored;
  2. current step is restored;

  By definition
  beta(s,t) := C(s+t,t)

  The following relations are used for fast calculation, they can be easily proved with the definition.

  beta(s,t-1)   = beta(s,t)*t/(s+t)
  beta(s-1,t-1) = beta(s,t-1)*s/(s+t-1)
  beta(s,t-2)   = beta(s-1,t-1)*(t-1)/s
  beta(s-3,t)   = beta(s-2,t-1)*(s-2)/t
  beta(s+1,t+1) = beta(s,t)*t/(s+1)

  For example, the first relation can be obtained by noticing
    beta(s,t) = beta(s,t-1) + beta(s-1,t) = beta(s,t-1)*(1+s/t)

*/
int revolve(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep)
{
  int b_s_tm1,b_sm1_tm1,b_sm2_tm1,b_s_tm2,b_sm3_t,b_sp1_tm1;
  int num_steps  = endstep - lastcheckpointstep;
  int rep_num    = 0;
  int upperbound = 1;

  if (num_checkpoints_avail <= 1 || lastcheckpointstep < 0 || endstep <= 0 || num_steps < 2) {
    *nextcheckpointstep = -1;
    return 0;
  }

  /* calculate repetition number and beta(s,t) (upperbound) */
  while (upperbound < num_steps) {
    rep_num++;
    upperbound = upperbound*(num_checkpoints_avail+rep_num)/rep_num; /* Do not write as a *= b, which is different */
  }

  /*
    b_s_tm1:   beta(s,t-1)
    b_sm1_tm1: beta(s-1,t-1)
    b_sm2_tm1: beta(s-2,t-1)
    b_s_tm2:   beta(s,t-2)
    b_sm3_t:   beta(s-3,t)
    b_sp1_tm1: beta(s+1,t-1)
  */
  b_s_tm1   = upperbound*rep_num/(num_checkpoints_avail+rep_num);
  b_sm1_tm1 = b_s_tm1*num_checkpoints_avail/(num_checkpoints_avail+rep_num-1);
  b_sm2_tm1 = b_sm1_tm1*(rep_num-1)/(num_checkpoints_avail+rep_num-2);
  b_s_tm2   = b_sm1_tm1*(rep_num-1)/num_checkpoints_avail;
  b_sm3_t   = b_sm2_tm1*(num_checkpoints_avail-2)/rep_num;
  b_sp1_tm1 = upperbound*rep_num/(num_checkpoints_avail+1);

  if (num_steps <= b_s_tm1+b_sm2_tm1) {
    *nextcheckpointstep = lastcheckpointstep + b_s_tm2;
  } else if (num_steps >= upperbound-b_sm3_t) {
    *nextcheckpointstep = lastcheckpointstep + b_s_tm1;
  } else {
    *nextcheckpointstep = endstep - b_sm1_tm1 - b_sm2_tm1;
  }
  return 0;
}

int numfwdstep(int m, int s)
{
  int num;
  int rep_num    = 0;
  int upperbound = 1;

  if (m < 1) {
    printf("Error in first: m < 1 ");
    return -1;
  }
  if (s < 1) {
    printf("Error in second arg: s < 1 ");
    return -1;
  }
  while (upperbound < m)
  {
    rep_num += 1;
    upperbound = upperbound*(rep_num + s)/rep_num;
  }
  num = rep_num*m - upperbound*rep_num/(s+1);
  return num;
}
