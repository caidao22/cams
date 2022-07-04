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

_ctx cams_ctx;
_ctx ca_ctx;

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
void printstates_ca(int m, int s)
{
  int i,j;
  int *P = ca_ctx.P,*PPATH = ca_ctx.PPATH;

  if (m!=ca_ctx.m || s!=ca_ctx.s) {
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
        if (i > l) {
          for (k=1; k<j; k++) {
            if (k == 1) p1 = P(i-l,j-k,m);
            else p1 = k - 1 + P(i,k-1,m) + P(i-l,j-k,m);
            if (p1 <= P(i,j,m)) { /* prioritize type 1 */
              P(i,j,m) = p1;
              PPATH(i,j,m) = k;
              PTYPE(i,j,m) = 1;
            }
          }
        }
        for (k=1; k<j; k++) {
          p0 = k + P(i,k,m) + P(i-1,j-k,m);
          if (p0 < P(i,j,m)) {
            P(i,j,m) = p0;
            PPATH(i,j,m) = k;
            PTYPE(i,j,m) = 0;
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
  int p1,p2;

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
        if (i-1 >= l) {
          for (k=2; k<j; k++) {
            if (Q(i-1,j-k+1,m) < P(i-1,j-k+1,m)) {
              p2 = k - 1 + P(i,k-1,m) + Q(i-1,j-k+1,m);
              if (p2 <= P(i,j,m)) {
                P(i,j,m) = p2;
                PPATH(i,j,m) = k;
                PTYPE(i,j,m) = 1;
              }
            }
          }
        }
        for (k=1; k<j-1; k++) {
          p1 = k + P(i,k,m) + P(i-1,j-k,m);
          if (p1 < P(i,j,m)) {
            P(i,j,m) = p1;
            PPATH(i,j,m) = k;
            PTYPE(i,j,m) = 0;
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
void printstates_cams(int m, int s, int l)
{
  int i,j;
  int *P = cams_ctx.P,*PPATH = cams_ctx.PPATH,*PTYPE = cams_ctx.PTYPE;

  printf("PPATH:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,PPATH(i,j,cams_ctx.m));
    }
    printf("\n");
  }
  printf("\nP:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,P(i,j,cams_ctx.m));
    }
    printf("\n");
  }
  printf("\nPTYPE:\n");
  for (i=1; i<=s; i++) {
    for (j=1; j<=m; j++) {
      printf("(%-3d %3d) %3d    ",i,j,PTYPE(i,j,cams_ctx.m));
    }
    printf("\n");
  }
  if (!cams_ctx.stifflyaccurate) {
    int *Q = cams_ctx.Q,*QPATH = cams_ctx.QPATH,*QTYPE = cams_ctx.QTYPE;
    printf("\nQPATH:\n");
    for (i=1; i<=s; i++) {
      for (j=1; j<=m; j++) {
        printf("(%-3d %3d) %3d    ",i,j,QPATH(i,j,cams_ctx.m));
      }
      printf("\n");
    }
    printf("\nQ:\n");
    for (i=1; i<=s; i++) {
      for (j=1; j<=m; j++) {
        printf("(%-3d %3d) %3d    ",i,j,Q(i,j,cams_ctx.m));
      }
      printf("\n");
    }
    printf("\nQTYPE:\n");
    for (i=1; i<=s; i++) {
      for (j=1; j<=m; j++) {
        printf("(%-3d %3d) %3d    ",i,j,QTYPE(i,j,cams_ctx.m));
      }
      printf("\n");
    }
  }
}
#endif

int offline_cams_create(int m, int s, int l, int stifflyaccurate)
{
  cams_ctx.initialize = 1;
  cams_ctx.m = m;
  cams_ctx.s = s;
  cams_ctx.stifflyaccurate = stifflyaccurate;
  if (stifflyaccurate) {
    dp2(m,s,l,&cams_ctx.P,&cams_ctx.PPATH,&cams_ctx.PTYPE);
  } else {
    ddp(m,s,l,&cams_ctx.P,&cams_ctx.PPATH,&cams_ctx.PTYPE,&cams_ctx.Q,&cams_ctx.QPATH,&cams_ctx.QTYPE);
  }
  return 0;
}

int offline_cams_destroy()
{
  if (cams_ctx.initialize) {
    cams_ctx.initialize = 0;
    free(cams_ctx.P);
    free(cams_ctx.PPATH);
    free(cams_ctx.PTYPE);
    if (!cams_ctx.stifflyaccurate) {
      free(cams_ctx.Q);
      free(cams_ctx.QPATH);
      free(cams_ctx.QTYPE);
    }
  }
  return 0;
}

/* num_checkpoints_avail includes the last checkpoint. */
int offline_cams(int lastcheckpointstep, int lastcheckpointtype, int num_checkpoints_avail, int endstep, int num_stages, int *nextcheckpointstep, int *nextcheckpointtype)
{
  int m,s = num_checkpoints_avail,l = num_stages;
  int *P = cams_ctx.P,*PPATH = cams_ctx.PPATH,*PTYPE = cams_ctx.PTYPE;
  int *Q = cams_ctx.Q,*QPATH = cams_ctx.QPATH,*QTYPE = cams_ctx.QTYPE;

  if (lastcheckpointtype == -1) m = endstep;
  if (lastcheckpointtype == 0 || lastcheckpointtype == 2) m = endstep-lastcheckpointstep;
  if (lastcheckpointtype == 1) m = endstep-lastcheckpointstep+1;

  if (s < 1) return 1; /* error */

  if (lastcheckpointstep == -1) {
    if (cams_ctx.stifflyaccurate) {
      if (s >= l && P(s-l+1,m-1,cams_ctx.m) <= P(s,m,cams_ctx.m)) {
        *nextcheckpointtype = 1;
        *nextcheckpointstep = 1;
      } else {
        *nextcheckpointtype = 0;
        *nextcheckpointstep = 0;
      }
    } else {
      if (Q(s,m,cams_ctx.m) <= P(s,m,cams_ctx.m)) {
        // look ahead
        *nextcheckpointtype = QTYPE(s,m,cams_ctx.m) ? 1 : 2;
        *nextcheckpointstep = 1;
      } else {
        *nextcheckpointtype = 0;
        *nextcheckpointstep = 0;
      }
    }
    return 0;
  }

  if (s == 1 || m < 2 || (m == 2 && lastcheckpointtype==0 && s<l+1)) {
    *nextcheckpointtype = -1;
    *nextcheckpointstep = -1;
    return 0;
  }

  if (cams_ctx.stifflyaccurate && lastcheckpointtype) return offline_cams(lastcheckpointstep,0,num_checkpoints_avail-l+1,endstep,num_stages,nextcheckpointstep,nextcheckpointtype);

  if (!cams_ctx.stifflyaccurate && lastcheckpointtype==2) return offline_cams(lastcheckpointstep,0,num_checkpoints_avail-l,endstep,num_stages,nextcheckpointstep,nextcheckpointtype);

  if (cams_ctx.stifflyaccurate) {
    // lastcheckpointtype is always 0
      *nextcheckpointtype = PTYPE(s,m,cams_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + PPATH(s,m,cams_ctx.m);
    // if (lastcheckpointtype == 1) {
    //   *nextcheckpointtype = PTYPE(s-l+1,m-1,cams_ctx.m);
    //   *nextcheckpointstep = lastcheckpointstep + PPATH(s-l+1,m-1,cams_ctx.m);
    // }
  } else{
    if (lastcheckpointtype == 0) {
      *nextcheckpointtype = PTYPE(s,m,cams_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + PPATH(s,m,cams_ctx.m);
    }
    if (lastcheckpointtype == 1) { // QPATH = 0 or 1
      *nextcheckpointtype = QTYPE(s,m,cams_ctx.m);
      *nextcheckpointstep = lastcheckpointstep + QPATH(s,m,cams_ctx.m);
    }
    // look ahead
    if ((*nextcheckpointtype) == 1 && QTYPE(s-l,endstep-(*nextcheckpointstep)+1,cams_ctx.m) == 0) *nextcheckpointtype = 2;
  }
  return 0;
}

int numfwdstep_cams(int m, int s, int l)
{
  int *P = cams_ctx.P;
  if (s<1) return INT_MIN;
  if (cams_ctx.stifflyaccurate) {
    if (m > 1 && s >= l && P(s-l+1,m-1,cams_ctx.m) < P(s,m,cams_ctx.m)) return P(s-l+1,m-1,cams_ctx.m);
    else return P(s,m,cams_ctx.m);
  } else {
    int *Q = cams_ctx.Q;
    return MIN(P(s,m,cams_ctx.m),Q(s,m,cams_ctx.m));
  }
}

int offline_ca_create(int m, int s)
{
  ca_ctx.initialize = 1;
  ca_ctx.m = m;
  ca_ctx.s = s;
  dp(m,s,&ca_ctx.P,&ca_ctx.PPATH);
  return 0;
}

int offline_ca_destroy()
{
  if (ca_ctx.initialize) {
    ca_ctx.initialize = 0;
    free(ca_ctx.P);
    free(ca_ctx.PPATH);
  }
  return 0;
}

/*
  This is for solution_only mode.
  num_checkpoints_avail includes the last checkpoint.
*/
int offline_ca(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep)
{
  int m = endstep-lastcheckpointstep,s = num_checkpoints_avail;
  int *PPATH = ca_ctx.PPATH;

  if (!ca_ctx.initialize) offline_ca_create(m,s);
  if (m <= 0) {
    offline_ca_destroy();
    return 0;
  }
  if (s < 1) return 1;
  if (lastcheckpointstep == -1) {
    *nextcheckpointstep = 0;
    return 0;
  }
  if (s == 1 || m < 3) {
    *nextcheckpointstep = -1;
    return 0;
  }
  *nextcheckpointstep = lastcheckpointstep + PPATH(s,m,ca_ctx.m);
  return 0;
}

int numfwdstep_ca(int m, int s)
{
  int *P = ca_ctx.P;
  return P(s,m,ca_ctx.m);
}
