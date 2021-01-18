#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define P(x, y, m) (P[(x-1)*(m)+y-1])
#define Q(x, y, m) (Q[(x-1)*(m)+y-1])
#define PPATH(x, y, m) (PPATH[(x-1)*(m)+y-1])
#define QPATH(x, y, m) (QPATH[(x-1)*(m)+y-1])
#define PTYPE(x, y, m) (PTYPE[(x-1)*(m)+y-1])
#define QTYPE(x, y, m) (QTYPE[(x-1)*(m)+y-1])

typedef struct {
  int initialize;      // indicate if the context has been initialized
  int stifflyaccurate; // the last stage value = solution for stifflyaccurate methods
  int m;               // number of time steps
  int s;               // number of checkpointing units
  int *P;              // DP work array
  int *PPATH;          // DP work array that stores the path
  int *PTYPE;          // checkpoint type, 0 solution only, 1 stage values, 2 both
  int *Q;              // DP work array
  int *QPATH;          // DP work array that stores the path
  int *QTYPE;          // checkpoint type, 0 solution only, 1 stage values, 2 both
} _ctx;

_ctx acms_ctx;
_ctx ac_ctx;

/*
  P(s,m) - number of extra forward steps to adjoin m steps given s checkpoints
  PPATH(s,m) - the last checkpoint position

  It is assumed s>1 and m>2.
*/
void dp(int m, int s, int **ptr_P, int **ptr_Path)
{
  int i,j,k;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *PPATH = (int *)malloc(m*s*sizeof(int));

  /* Initialize P(1,*) */
  for (j=1; j<=m; j++) {
    P(1,j,m) = (j-1)*j/2;
    PPATH(1,j,m) = 0;
  }
  for (i=2; i<=s; i++) {
    P(i,1,m) = 0;
    PPATH(i,1,m) = -1;
    P(i,2,m) = 1;
    PPATH(i,2,m) = -1;
  }

  for (i=2; i<=s; i++) {
    for (j=3; j<=m; j++) {
      if (i >= j-1) {
        P(i,j,m) = j-1;
        PPATH(i,j,m) = 1;
      } else {
        P(i,j,m) = INT_MAX;
        for (k=1; k<j; k++) {
          //printf("i=%d j=%d k=%d pmin=%d pik=%d\n",i,j,k,P(i-1,j-k),P(i,k));
          if (k+P(i,k,m)+P(i-1,j-k,m) < P(i,j,m)) {
            P(i,j,m) = k+P(i,k,m)+P(i-1,j-k,m);
            PPATH(i,j,m) = k;
          }
        }
      }
    }
  }
  *ptr_P = P;
  *ptr_Path = PPATH;
}

#if defined(DEBUG)
void printstates_ac(int m, int s)
{
  int i,j;
  int *P = ac_ctx.P,*PPATH = ac_ctx.PPATH;

  if (m!=ac_ctx.m || s!=ac_ctx.s) {
    printf("Error in arguments.\n");
  }
  printf("PPATH:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d   ",i,j,PPATH(i,j,m));
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
#endif

/* For stiffly-accurate multistage methods (solution=last stage value). */
void dp2(int m, int s, int l, int **ptr_P, int **ptr_PPATH, int **ptr_PTYPE)
{
  int i,j,k,p0,p1;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *PPATH = (int *)malloc(m*s*sizeof(int));
  int *PTYPE = (int *)malloc(m*s*sizeof(int));
  int pmin,store_stage;

  /* Initialize P(1,*) */
  for (j=1; j<=m; j++) {
    P(1,j,m) = (j-1)*j/2;
    PPATH(1,j,m) = -1;
    PTYPE(1,j,m) = -1;
  }
  for (i=2; i<=s; i++) {
    P(i,1,m) = 0;
    PPATH(i,1,m) = -1;
    PTYPE(i,1,m) = -1;

    if (i > l) {
      P(i,2,m) = 0;
      PPATH(i,2,m) = 1;
      PTYPE(i,2,m) = 1;
    } else {
      P(i,2,m) = 1;
      PPATH(i,2,m) = -1;
      PTYPE(i,2,m) = -1;
    }
  }
  for (i=2; i<=s; i++)
    for (j=3; j<=m; j++) {
      if (i > (j-1)*l) {
        P(i,j,m) = 0;
        PPATH(i,j,m) = 1;
        PTYPE(i,j,m) = 1;
      } else {
        P(i,j,m) = INT_MAX;
        for (k=1; k<j; k++) {
          p0 = k + P(i,k,m) + P(i-1,j-k,m);
          pmin = p0;
          store_stage = 0;
          if (i > l) {
            if (k == 1) p1 = P(i-l,j-k,m);
            else p1 = k - 1 + P(i,k-1,m) + P(i-l,j-k,m);
            if (p1 <= p0) {
              pmin = p1;
              store_stage = 1;
            }
          }
          if (pmin < P(i,j,m)) {
            P(i,j,m) = pmin;
            PPATH(i,j,m) = k;
            PTYPE(i,j,m) = store_stage;
          }
        }
      }
    }
  *ptr_P = P;
  *ptr_PPATH = PPATH;
  *ptr_PTYPE = PTYPE;
}

/* For normal multistage methods */
void ddp(int m, int s, int l, int **ptr_P, int **ptr_PPATH, int **ptr_PTYPE, int **ptr_Q, int **ptr_QPATH, int **ptr_QTYPE)
{
  int i,j,k;
  int *P = (int *)malloc(m*s*sizeof(int));
  int *PPATH = (int *)malloc(m*s*sizeof(int));
  int *PTYPE = (int *)malloc(m*s*sizeof(int));
  int *Q = (int *)malloc(m*s*sizeof(int));
  int *QPATH = (int *)malloc(m*s*sizeof(int));
  int *QTYPE = (int *)malloc(m*s*sizeof(int));
  int pqmin,store_stage;

  /* Initialize P(1,*) (s=1) with one checkpoint */
  for (j=1; j<=m; j++) {
    P(1,j,m) = (j-1)*j/2;
    PPATH(1,j,m) = 0;
    PTYPE(1,1,m) = 0;
    Q(1,j,m) = INT_MAX;
    QPATH(1,j,m) = -1;
    QTYPE(1,j,m) = -1;
  }
  if (l == 1) { /* exceptions */
    Q(1,1,m) = 0;
    QPATH(1,1,m) = 0;
    QTYPE(1,1,m) = 1;
    Q(1,2,m) = 0;
    QPATH(1,2,m) = -1;
    QTYPE(1,2,m) = -1;
  }

  /* s>1 and m>2*/
  for (i=2; i<=s; i++) {
    /* one time step */
    P(i,1,m) = 0;
    PPATH(i,1,m) = -1;
    PTYPE(i,1,m) = 0;
    Q(i,1,m) = (i>=l) ? 0 :INT_MAX;
    QPATH(i,1,m) = -1;
    QTYPE(i,1,m) = -1;
    /* two time steps */
    if (i >= l+1) {
      P(i,2,m) = 0;
      PPATH(i,2,m) = 1;
      PTYPE(i,2,m) = 1;
    } else {
      P(i,2,m) = 1;
      PPATH(i,2,m) = -1;
      PTYPE(i,2,m) = -1;
    }
    Q(i,2,m) = (i >= l) ? 0 : INT_MAX;
    QPATH(i,2,m) = -1;
    QTYPE(i,2,m) = -1;
  }
  for (i=2; i<=s; i++)
    for (j=3; j<=m; j++) {
      if (i >= (j-1)*(l+1)) {
        P(i,j,m) = 0;
        PPATH(i,j,m) = 1;
        PTYPE(i,j,m) = 1;
        Q(i,j,m) = 0;
        QPATH(i,j,m) = 1;
        QTYPE(i,j,m) = 1;
      } else {
        P(i,j,m) = INT_MAX;
        Q(i,j,m) = INT_MAX;
        for (k=1; k<j-1; k++) {
          if (i-1 >= l && Q(i-1,j-k,m) < P(i-1,j-k,m)) {
            pqmin = k + P(i,k,m) + Q(i-1,j-k,m);
            store_stage = 1;
          } else {
             /*
               Storing a solution-only checkpoint has the priority. We dont want to see cases that stages values follow immediately after a solution.
              */
            pqmin = k + P(i,k,m) + P(i-1,j-k,m);
            store_stage = 0;
          }
          if (pqmin < P(i,j,m)) {
            P(i,j,m) = pqmin;
            PPATH(i,j,m) = store_stage ? k+1 : k ;
            PTYPE(i,j,m) = store_stage;
          }
        }
        if (i > l) {
          if (P(i-l,j-1,m) < Q(i-l,j-1,m)) {
            Q(i,j,m) = P(i-l,j-1,m);
            QTYPE(i,j,m) = 0;
            QPATH(i,j,m) = 0;
          } else { // prioritize storing stage values
            Q(i,j,m) = Q(i-l,j-1,m);
            QTYPE(i,j,m) = 1;
            QPATH(i,j,m) = 1;
          }
        }
      }
    }
  *ptr_P = P;
  *ptr_PPATH = PPATH;
  *ptr_PTYPE = PTYPE;
  *ptr_Q = Q;
  *ptr_QPATH = QPATH;
  *ptr_QTYPE = QTYPE;
}

#if defined(DEBUG)
void printstates_acms(int m, int s, int l)
{
  int i,j;
  int *P = acms_ctx.P,*PPATH = acms_ctx.PPATH,*PTYPE = acms_ctx.PTYPE;

  printf("PPATH:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,PPATH(i,j,acms_ctx.m));
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
  printf("\nPTYPE:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,PTYPE(i,j,acms_ctx.m));
    }
    printf("\n");
  }
  if (!acms_ctx.stifflyaccurate) {
    int *Q = acms_ctx.Q,*QPATH = acms_ctx.QPATH,*QTYPE = acms_ctx.QTYPE;
    printf("\nQPATH:\n");
    for (i=1; i<=s; i++) {
      for (j=1; j<=m; j++) {
        printf("(%-3d %3d) %3d    ",i,j,QPATH(i,j,acms_ctx.m));
      }
      printf("\n");
    }
    printf("\nQ:\n");
    for (i=1; i<=s; i++) {
      for (j=1; j<=m; j++) {
        printf("(%-3d %3d) %3d    ",i,j,Q(i,j,acms_ctx.m));
      }
      printf("\n");
    }
    printf("\nQTYPE:\n");
    for (i=1; i<=s; i++) {
      for (j=1; j<=m; j++) {
        printf("(%-3d %3d) %3d    ",i,j,QTYPE(i,j,acms_ctx.m));
      }
      printf("\n");
    }
  }
}
#endif

int offline_acms_create(int m, int s, int l, int stifflyaccurate)
{
  acms_ctx.initialize = 1;
  acms_ctx.m = m;
  acms_ctx.s = s;
  acms_ctx.stifflyaccurate = stifflyaccurate;
  if (stifflyaccurate) {
    dp2(m,s,l,&acms_ctx.P,&acms_ctx.PPATH,&acms_ctx.PTYPE);
  } else {
    ddp(m,s,l,&acms_ctx.P,&acms_ctx.PPATH,&acms_ctx.PTYPE,&acms_ctx.Q,&acms_ctx.QPATH,&acms_ctx.QTYPE);
  }
  return 0;
}

int offline_acms_destroy()
{
  if (acms_ctx.initialize) {
    acms_ctx.initialize = 0;
    free(acms_ctx.P);
    free(acms_ctx.PPATH);
    free(acms_ctx.PTYPE);
    if (acms_ctx.stifflyaccurate) {
      free(acms_ctx.Q);
      free(acms_ctx.QPATH);
      free(acms_ctx.QTYPE);
    }
  }
  return 0;
}

/* num_checkpoints_avail includes the last checkpoint. */
int offline_acms(int lastcheckpointstep, int lastcheckpointtype, int num_checkpoints_avail, int endstep, int num_stages, int *nextcheckpointstep, int *nextcheckpointtype)
{
  int m,s = num_checkpoints_avail,l = num_stages;
  int *P = acms_ctx.P,*PPATH = acms_ctx.PPATH,*PTYPE = acms_ctx.PTYPE;
  int *Q = acms_ctx.Q,*QPATH = acms_ctx.QPATH,*QTYPE = acms_ctx.QTYPE;

  if (lastcheckpointtype == -1) m = endstep;
  else if (lastcheckpointtype == 0) m = endstep-lastcheckpointstep;
  else m = endstep-lastcheckpointstep+1;

  if (s < 1) return 1; /* error */

  if (s == 1 || m < 2 || (m == 2 && lastcheckpointtype==0 && s<l+1)) {
    *nextcheckpointtype = -1;
    *nextcheckpointstep = -1;
    return 0;
  }

  if (lastcheckpointstep == -1) {
    if (acms_ctx.stifflyaccurate) {
      if (s >= l && P(s-l+1,m-1,acms_ctx.m) <= P(s,m,acms_ctx.m)) {
        *nextcheckpointtype = 1;
        *nextcheckpointstep = 1;
      } else {
        *nextcheckpointtype = 0;
        *nextcheckpointstep = 0;
      }
    } else {
      if (Q(s,m,acms_ctx.m) <= P(s,m,acms_ctx.m)) {
        // look ahead
        *nextcheckpointtype = QTYPE(s,m,acms_ctx.m) ? 1 : 2;
        *nextcheckpointstep = 1;
      } else {
        *nextcheckpointtype = 0;
        *nextcheckpointstep = 0;
      }
    }
    return 0;
  }

  if (acms_ctx.stifflyaccurate && lastcheckpointtype) return offline_acms(lastcheckpointstep,0,num_checkpoints_avail-l+1,endstep,num_stages,nextcheckpointstep,nextcheckpointtype);

  if (!acms_ctx.stifflyaccurate && lastcheckpointtype==2) return offline_acms(lastcheckpointstep,0,num_checkpoints_avail-l,endstep,num_stages,nextcheckpointstep,nextcheckpointtype);

  if (acms_ctx.stifflyaccurate) {
    // lastcheckpointtype is always 0
      *nextcheckpointtype = PTYPE(s,m,acms_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + PPATH(s,m,acms_ctx.m);
    // if (lastcheckpointtype == 1) {
    //   *nextcheckpointtype = PTYPE(s-l+1,m-1,acms_ctx.m);
    //   *nextcheckpointstep = lastcheckpointstep + PPATH(s-l+1,m-1,acms_ctx.m);
    // }
  } else{
    if (lastcheckpointtype == 0) {
      *nextcheckpointtype = PTYPE(s,m,acms_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + PPATH(s,m,acms_ctx.m);
      // look ahead
      if ((*nextcheckpointtype) == 1 && QTYPE(s-1,m-(*nextcheckpointstep)+1,acms_ctx.m) == 0) *nextcheckpointtype = 2;
    }
    if (lastcheckpointtype == 1) { // QPATH = 0 or 1
      *nextcheckpointtype = QTYPE(s,m,acms_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + QPATH(s,m,acms_ctx.m);
      // look ahead
      if ((*nextcheckpointtype) == 1 && QTYPE(s-l,m-(*nextcheckpointstep)+1,acms_ctx.m) == 0) { 
        *nextcheckpointtype = 2;
      }
    }
  }
  return 0;
}

int numfwdstep_acms(int m, int s, int l)
{
  int *P = acms_ctx.P;
  if (acms_ctx.stifflyaccurate) {
    if (m > 1 && s >= l && P(s-l+1,m-1,acms_ctx.m) < P(s,m,acms_ctx.m)) return P(s-l+1,m-1,acms_ctx.m);
    else return P(s,m,acms_ctx.m);
  } else {
    int *Q = acms_ctx.Q;
    return MIN(P(s,m,acms_ctx.m),Q(s,m,acms_ctx.m));
  }
}

int offline_ac_create(int m, int s)
{
  ac_ctx.initialize = 1;
  ac_ctx.m = m;
  ac_ctx.s = s;
  dp(m,s,&ac_ctx.P,&ac_ctx.PPATH);
  return 0;
}

int offline_ac_destroy()
{
  if (ac_ctx.initialize) {
    ac_ctx.initialize = 0;
    free(ac_ctx.P);
    free(ac_ctx.PPATH);
  }
  return 0;
}

int offline_ac(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep)
{
  int m = endstep-lastcheckpointstep,s = num_checkpoints_avail;
  int *PPATH = ac_ctx.PPATH;

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
    *nextcheckpointstep = lastcheckpointstep + PPATH(s,m,ac_ctx.m);
  return 0;
}

int numfwdstep_ac(int m, int s)
{
  int *P = ac_ctx.P;
  return P(s,m,ac_ctx.m);
}
