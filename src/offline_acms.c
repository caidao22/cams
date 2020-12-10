#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define P(x, y) (P[(x-1)*m+y-1])
#define Path(x, y) (Path[(x-1)*m+y-1])
#define S(x, y) (S[(x-1)*m+y-1])

typedef struct {
  int initialize;
  int *P;
  int *Path;
  int *S;
} _ctx;

_ctx acms_ctx;
_ctx ac_ctx;

/*
  P(s,m) - number of extra forward steps to adjoin m steps given s checkpoints
  Path(s,m) - the last checkpoint position

  Because cases where m=0 or s=0 can be excluded, these tables can be stored in arrays of size m*s.
*/
void dp(int m, int s, int **ptr_P, int **ptr_Path)
{
  int i,j,k;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *Path = (int *)malloc(m*s*sizeof(int));

  /* Initialize P(1,*) */
  for (j=1; j<=m; j++) {
    P(1,j) = (j-1)*j/2;
    Path(1,j) = 0;
  }
  for (i=2; i<=s; i++) 
    for (j=1; j<=m; j++) {
      if (i >= j-1) {
        P(i,j) = j-1;
        Path(i,j) = 0;
      } else {
        P(i,j) = INT_MAX;
        for (k=1; k<j; k++) {
          //printf("i=%d j=%d k=%d pmin=%d pik=%d\n",i,j,k,P(i-1,j-k),P(i,k));
          if (k+P(i,k)+P(i-1,j-k) < P(i,j)) {
            P(i,j) = k+P(i,k)+P(i-1,j-k);
            Path(i,j) = k;
          }
        }
      }
    }
  *ptr_P = P;
  *ptr_Path = Path;
}

void dp2(int m, int s, int l, int **ptr_P, int **ptr_Path, int **ptr_S)
{
  int i,j,k;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *Path = (int *)malloc(m*s*sizeof(int));
  int *S = (int *)malloc(m*s*sizeof(int));
  int pmin,store_stage;

  /* Initialize P(1,*) */
  for (j=1; j<=m; j++) {
    P(1,j) = (j-1)*j/2;
    Path(1,j) = 0;
  }
  for (i=2; i<=s; i++) 
    for (j=1; j<=m; j++) {
      if (i-1 >= (j-1)*(l+1)) {
        P(i,j) = 0;
        Path(i,j) = 0;
        S(i,j) = 1;
      } else {
        P(i,j) = INT_MAX;
        for (k=0; k<j; k++) {
          if (i>l+1 && k != j-1 && P(i-1-l,j-k-1) < P(i-1,j-k)) {
            pmin = P(i-1-l,j-k-1);
            store_stage = 1;
          } else {
            pmin = P(i-1,j-k);
            store_stage = 0;
          }
          //printf("i=%d j=%d k=%d pmin=%d store=%d\n",i,j,k,pmin,store_stage);
          if ((k==0 && pmin < P(i,j)) || (k!=0 && k+P(i,k)+pmin < P(i,j))) {
            P(i,j) = k+P(i,k)+pmin;
            Path(i,j) = k;
            S(i,j) = store_stage;
          }
        }
      }
    }
  *ptr_P = P;
  *ptr_Path = Path;
  *ptr_S = S;
}

void printstates_acms(int m, int s, int l)
{
  int i,j;
  int *P = acms_ctx.P,*Path = acms_ctx.Path,*S = acms_ctx.S;

  printf("Path:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%4d %4d) %4d  ",i,j,Path(i,j));
    }
    printf("\n");
  }
  printf("\nP:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%4d %4d) %4d  ",i,j,P(i,j));
    }
    printf("\n");
  }
  printf("\nS:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%4d %4d) %d  ",i,j,S(i,j));
    }
    printf("\n");
  }
}

void printpath_acms(int m, int s, int l)
{
  int k,current_step = 0,store_stage,num_checkpoints_avail = s;
  int *P = acms_ctx.P,*Path = acms_ctx.Path,*S = acms_ctx.S;

  if (m>1 && s>l+1 && P(s-l,m-1) > P(s,m)) {
    printf("Store solution at 0\n");
    k = Path(s,m);
    store_stage = S(s,m);
    num_checkpoints_avail--;
  } else {
    printf("Store solution and stage values at 1\n");
    k = Path(s-l,m-1);
    store_stage = S(s-l,m-1);
    num_checkpoints_avail-= l+1;
    current_step++;
  }

  while(num_checkpoints_avail && current_step<m-1) {
    current_step = current_step+k;
    if (store_stage) {
      printf("next checkpoint is at %d (solution+stages)\n",current_step+1);
      printf("%d %d\n",num_checkpoints_avail-l,m-current_step-1);
      k = Path(num_checkpoints_avail-l,m-current_step-1);
      //printf("Path(%d %d)=%d\n",num_checkpoints_avail-l,m-current_step-1,k);
      store_stage = S(num_checkpoints_avail-l,m-current_step-1);
      current_step++;
      num_checkpoints_avail -= l+1;
    } else {
      printf("next checkpoint is at %d\n",current_step);
      k = Path(num_checkpoints_avail,m-current_step);
      //printf("Path(%d %d)=%d\n",num_checkpoints_avail,m-current_step,k);
      store_stage = S(num_checkpoints_avail,m-current_step);
      num_checkpoints_avail--;
    }
  }
}

int offline_acms_create(int m, int s, int l)
{
  acms_ctx.initialize = 1;
  dp2(m,s,l,&acms_ctx.P,&acms_ctx.Path,&acms_ctx.S);
  return 0;
}

int offline_acms_destroy()
{
  acms_ctx.initialize = 0;
  if (acms_ctx.P) free(acms_ctx.P);
  if (acms_ctx.Path) free(acms_ctx.Path);
  if (acms_ctx.S) free(acms_ctx.S);
  return 0;
}

int offline_acms(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int num_stages, int *nextcheckpointstep, int *nextcheckpointtype)
{
  int m = endstep-lastcheckpointstep,s = num_checkpoints_avail,l = num_stages;
  int *P = acms_ctx.P,*Path = acms_ctx.Path,*S = acms_ctx.S;

  if (!acms_ctx.initialize) offline_acms_create(m,s,l);
  if (m <=0) {
    offline_acms_destroy();
    return 0;
  }
  if (lastcheckpointstep == -1 && m>1 && s>l+1 && P(s-l,m-1) <= P (s,m)) {
    *nextcheckpointtype = 1;
    *nextcheckpointstep = 1;
  } else {
    *nextcheckpointtype = S(s,m);
    if (*nextcheckpointtype == 1)
      *nextcheckpointstep = lastcheckpointstep + Path(s-l,m-1);
    if (*nextcheckpointtype == 0)
      *nextcheckpointstep = lastcheckpointstep + Path(s,m);
  }
  return 0;
}

int numfwdstep_acms(int m, int s, int l)
{
  int *P = acms_ctx.P;
  if (m>1 && s>l+1 && P(s-l,m-1) < P(s,m)) return P(s-l,m-1);
  else return P(s,m);
}

int offline_ac_create(int m, int s)
{
  ac_ctx.initialize = 1;
  dp(m,s,&ac_ctx.P,&ac_ctx.Path);
  return 0;
}

int offline_ac_destroy()
{
  ac_ctx.initialize = 0;
  if (ac_ctx.P) free(ac_ctx.P);
  if (ac_ctx.Path) free(ac_ctx.Path);
  return 0;
}

int offline_ac(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep)
{
  int m = endstep-lastcheckpointstep,s = num_checkpoints_avail;
  int *Path = ac_ctx.Path;

  if (!ac_ctx.initialize) offline_ac_create(m,s);
  if (m<=0) {
    offline_ac_destroy();
    return 0;
  }
  if (lastcheckpointstep == -1 && s>0)
    *nextcheckpointstep = 0;
  else
    *nextcheckpointstep = lastcheckpointstep + Path(s,m);
  return 0;
}

int numfwdstep_ac(int m, int s)
{
  int *P = ac_ctx.P;
  return P(m,s);
}