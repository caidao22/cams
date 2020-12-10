#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define P(x, y, m) (P[(x-1)*(m)+y-1])
#define Path(x, y, m) (Path[(x-1)*(m)+y-1])
#define S(x, y, m) (S[(x-1)*(m)+y-1])

typedef struct {
  int initialize;
  int m;
  int s;
  int *P;
  int *Path;
  int *S;
} _ctx;

_ctx acms_ctx;
_ctx ac_ctx;

/*
  P(s,m) - number of extra forward steps to adjoin m steps given s checkpoints
  Path(s,m) - the last checkpoint position

  It is assumed s>1 and m>2.
*/
void dp(int m, int s, int **ptr_P, int **ptr_Path)
{
  int i,j,k;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *Path = (int *)malloc(m*s*sizeof(int));

  /* Initialize P(1,*) */
  for (j=1; j<=m; j++) {
    P(1,j,m) = (j-1)*j/2;
    Path(1,j,m) = 0;
  }
  for (i=2; i<=s; i++) {
    P(i,1,m) = 0;
    Path(i,1,m) = -1;
    P(i,2,m) = 1;
    Path(i,2,m) = -1;
  }

  for (i=2; i<=s; i++) {
    for (j=3; j<=m; j++) {
      if (i >= j-1) {
        P(i,j,m) = j-1;
        Path(i,j,m) = 1;
      } else {
        P(i,j,m) = INT_MAX;
        for (k=1; k<j; k++) {
          //printf("i=%d j=%d k=%d pmin=%d pik=%d\n",i,j,k,P(i-1,j-k),P(i,k));
          if (k+P(i,k,m)+P(i-1,j-k,m) < P(i,j,m)) {
            P(i,j,m) = k+P(i,k,m)+P(i-1,j-k,m);
            Path(i,j,m) = k;
          }
        }
      }
    }
  }
  *ptr_P = P;
  *ptr_Path = Path;
}

#if defined(DEBUG)
void printstates_ac(int m, int s)
{
  int i,j;
  int *P = ac_ctx.P,*Path = ac_ctx.Path;

  if (m!=ac_ctx.m || s!=ac_ctx.s) {
    printf("Error in arguments.\n");
  }
  printf("Path:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d   ",i,j,Path(i,j,m));
    }
    printf("\n");
  }
  printf("\nP:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,P(i,j,m));
    }
    printf("\n");
  }
}

void printpath_ac(int m, int s)
{
  int k,current_step = 0,num_checkpoints_avail = s;
  int *Path = ac_ctx.Path;

  printf("next checkpoint is at 0\n");
  k = Path(s,m,ac_ctx.m);
  num_checkpoints_avail--;
  while(num_checkpoints_avail && k >0 && current_step<m-1) {
    current_step = current_step+k;
    printf("next checkpoint is at %d\n",current_step);
    k = Path(num_checkpoints_avail,m-current_step,ac_ctx.m);
    num_checkpoints_avail--;
  }
}
#endif

/* It is assumed that s>1 and m> 2*/
void dp2(int m, int s, int l, int **ptr_P, int **ptr_Path, int **ptr_S)
{
  int i,j,k;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *Path = (int *)malloc(m*s*sizeof(int));
  int *S = (int *)malloc(m*s*sizeof(int));
  int pmin,store_stage;

  /* Initialize P(1,*) */
  for (j=1; j<=m; j++) {
    P(1,j,m) = (j-1)*j/2;
    Path(1,j,m) = 0;
  }
  for (i=2; i<=s; i++) {
    P(i,1,m) = 0;
    Path(i,1,m) = -1;
    S(i,1,m) = 0;

    if (i>l+1) {
      P(i,2,m) = 0;
      Path(i,2,m) = 1;
      S(i,2,m) = 1;
    } else {
      P(i,2,m) = 1;
      Path(i,2,m) = -1;
      S(i,2,m) = 0;
    }
  }
  for (i=2; i<=s; i++)
    for (j=3; j<=m; j++) {
      if (i >= (j-1)*(l+1)) {
        P(i,j,m) = 0;
        Path(i,j,m) = 1;
        S(i,j,m) = 1;
      } else {
        P(i,j,m) = INT_MAX;
        for (k=0; k<j; k++) {
          if (i>l+1 && k != j-1 && P(i-1-l,j-k-1,m) < P(i-1,j-k,m)) {
            pmin = P(i-1-l,j-k-1,m);
            store_stage = 1;
          } else {
            pmin = P(i-1,j-k,m);
            store_stage = 0;
          }
          //printf("i=%d j=%d k=%d pmin=%d store=%d\n",i,j,k,pmin,store_stage);
          if ((k==0 && pmin < P(i,j,m)) || (k!=0 && k+P(i,k,m)+pmin < P(i,j,m))) {
            P(i,j,m) = k+P(i,k,m)+pmin;
            Path(i,j,m) = k;
            S(i,j,m) = store_stage;
          }
        }
      }
    }
  *ptr_P = P;
  *ptr_Path = Path;
  *ptr_S = S;
}

#if defined(DEBUG)
void printstates_acms(int m, int s, int l)
{
  int i,j;
  int *P = acms_ctx.P,*Path = acms_ctx.Path,*S = acms_ctx.S;

  printf("Path:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,Path(i,j,acms_ctx.m));
    }
    printf("\n");
  }
  printf("\nP:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,P(i,j,acms_ctx.m));
    }
    printf("\n");
  }
  printf("\nS:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,S(i,j,acms_ctx.m));
    }
    printf("\n");
  }
}

void printpath_acms(int m, int s, int l)
{
  int k,current_step = 0,store_stage,num_checkpoints_avail = s;
  int *P = acms_ctx.P,*Path = acms_ctx.Path,*S = acms_ctx.S;

  if (m>1 && s>l+1 && P(s-l,m-1,acms_ctx.m) > P(s,m,acms_ctx.m)) {
    printf("next checkpoint is at 0\n");
    k = Path(s,m,acms_ctx.m);
    store_stage = S(s,m,acms_ctx.m);
    num_checkpoints_avail--;
  } else {
    printf("next checkpoint is at 1(solution+stages)\n");
    k = Path(s-l,m-1,acms_ctx.m);
    store_stage = S(s-l,m-1,acms_ctx.m);
    num_checkpoints_avail-= l+1;
    current_step++;
  }

  while(num_checkpoints_avail && current_step<m-1) {
    current_step = current_step+k;
    if (store_stage) {
      printf("next checkpoint is at %d (solution+stages)\n",current_step);
      k = Path(num_checkpoints_avail-l,m-current_step,acms_ctx.m);
      //printf("Path(%d %d)=%d\n",num_checkpoints_avail-l,m-current_step-1,k);
      store_stage = S(num_checkpoints_avail-l,m-current_step,acms_ctx.m);
      current_step++;
      num_checkpoints_avail -= l+1;
    } else {
      printf("next checkpoint is at %d\n",current_step);
      k = Path(num_checkpoints_avail,m-current_step,acms_ctx.m);
      //printf("Path(%d %d)=%d\n",num_checkpoints_avail,m-current_step,k);
      store_stage = S(num_checkpoints_avail,m-current_step,acms_ctx.m);
      num_checkpoints_avail--;
    }
  }
}
#endif

int offline_acms_create(int m, int s, int l)
{
  acms_ctx.initialize = 1;
  acms_ctx.m = m;
  acms_ctx.s = s;
  dp2(m,s,l,&acms_ctx.P,&acms_ctx.Path,&acms_ctx.S);
  return 0;
}

int offline_acms_destroy()
{
  if (acms_ctx.initialize) {
    acms_ctx.initialize = 0;
    free(acms_ctx.P);
    free(acms_ctx.Path);
    free(acms_ctx.S);
  }
  return 0;
}

/* num_checkpoints_avail includes the last checkpoint. */
int offline_acms(int lastcheckpointstep, int lastcheckpointtype, int num_checkpoints_avail, int endstep, int num_stages, int *nextcheckpointstep, int *nextcheckpointtype)
{
  int m = endstep-lastcheckpointstep,s = num_checkpoints_avail,l = num_stages;
  int *P = acms_ctx.P,*Path = acms_ctx.Path,*S = acms_ctx.S;

  if (!acms_ctx.initialize) offline_acms_create(m,s,l);
  if (m <= 0) {
    offline_acms_destroy();
    return 0;
  }
  if ((lastcheckpointtype && s<l+1) || s<1) return 1; /* error */
  if (lastcheckpointtype) {
    if (s == l+1 || m < 2 || (m == 2 && s<2*(l+1))) {
      *nextcheckpointtype = 1;
      *nextcheckpointstep = -1;
      return 0;
    }
  }

  if (!lastcheckpointtype) {
    if (s == 1 || m < 3) {
      *nextcheckpointtype = 0;
      *nextcheckpointstep = -1;
      return 0;
    }
  }

  if (lastcheckpointstep == -1) {
   if (m>1 && s>l+1 && P(s-l,m-1,acms_ctx.m) > P(s,m,acms_ctx.m)) {
    *nextcheckpointtype = 0;
    *nextcheckpointstep = 0;
   } else {
    *nextcheckpointtype = 1;
    *nextcheckpointstep = 1;
   }
  } else {
    if (lastcheckpointtype) {
      *nextcheckpointtype = S(s-l,m,acms_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + Path(s-l,m,acms_ctx.m);
    } else {
      *nextcheckpointtype = S(s,m,acms_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + Path(s,m,acms_ctx.m);
    }
  }
  return 0;
}

int numfwdstep_acms(int m, int s, int l)
{
  int *P = acms_ctx.P;
  if (m>1 && s>l+1 && P(s-l,m-1,acms_ctx.m) < P(s,m,acms_ctx.m)) return P(s-l,m-1,acms_ctx.m);
  else return P(s,m,acms_ctx.m);
}

int offline_ac_create(int m, int s)
{
  ac_ctx.initialize = 1;
  ac_ctx.m = m;
  ac_ctx.s = s;
  dp(m,s,&ac_ctx.P,&ac_ctx.Path);
  return 0;
}

int offline_ac_destroy()
{
  if (ac_ctx.initialize) {
    ac_ctx.initialize = 0;
    free(ac_ctx.P);
    free(ac_ctx.Path);
  }
  return 0;
}

int offline_ac(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep)
{
  int m = endstep-lastcheckpointstep,s = num_checkpoints_avail;
  int *Path = ac_ctx.Path;

  if (!ac_ctx.initialize) offline_ac_create(m,s);
  if (m <= 0) {
    offline_ac_destroy();
    return 0;
  }
  if (s < 1) return 1;
  if (s == 1 || m < 3) {
    *nextcheckpointstep = -1;
    return 0;
  }
  if (lastcheckpointstep == -1 && s>0)
    *nextcheckpointstep = 0;
  else
    *nextcheckpointstep = lastcheckpointstep + Path(s,m,ac_ctx.m);
  return 0;
}

int numfwdstep_ac(int m, int s)
{
  int *P = ac_ctx.P;
  return P(s,m,ac_ctx.m);
}