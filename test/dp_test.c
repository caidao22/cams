/*
  Generate adjoint checkpointing schedule.
  To run the program, input number of checkpoints available (s) and number of time steps (m) in the command line.
  For example ./dp_test 5 10
*/

#include "offline_schedule.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc,char **argv)
{
  int m,s,l = 2,currentstep = 0,lastcheckpointstep,lastcheckpointtype,nextcheckpointstep,nextcheckpointtype,numcheckpoints;

  if (argc!=3) {
      printf("Wrong command line options. Must input number of checkpoints available and number of time steps (two integers separated by space).\n");
      exit(0);
  }
  s = atoi(argv[1]);
  m = atoi(argv[2]);

  offline_ca_create(m,s);
  printf("AC result for (%d,%d) = %d correct result should be %d\n",s,m,numfwdstep_ca(m,s),numfwdstep(m,s));

#if defined(DEBUG)
  printstates_ca(m,s);
#endif
  numcheckpoints = s;
  offline_ca(-1,numcheckpoints,m,&nextcheckpointstep);
  while (currentstep<m) {
    if (currentstep == nextcheckpointstep) {
      printf("Store checkpoint at %d\n",currentstep);
      lastcheckpointstep = currentstep;
      offline_ca(lastcheckpointstep,numcheckpoints,m,&nextcheckpointstep);
      numcheckpoints--;
    }
    currentstep++;
  }
  offline_ca_destroy();

  l = 2;
  offline_cams_create(m,s,l,1); // stiffly-accurate
  printf("\n\nCAMS gives (%d,%d) = %d \n",s,m,numfwdstep_cams(m,s,l));
#if defined(DEBUG)
  printstates_cams(m,s,l);
#endif
  currentstep = 0;
  numcheckpoints = s;
  offline_cams(-1,-1,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
  if (!nextcheckpointstep) {
    offline_cams(0,0,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
    printf("Store checkpoint at %d with solution\n",currentstep);
    numcheckpoints--;
  }
  while (currentstep<m) {
    currentstep++;
    if (currentstep == nextcheckpointstep) {
      lastcheckpointstep = currentstep;
      lastcheckpointtype = nextcheckpointtype;
      offline_cams(lastcheckpointstep,lastcheckpointtype,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
      //printf("laststep=%d lasttype=%d s=%d m=%d l=%d nextstep=%d nexttype=%d\n",lastcheckpointstep,lastcheckpointtype,numcheckpoints,m,l,nextcheckpointstep,nextcheckpointtype);
      if (lastcheckpointtype == 0) {
        printf("Store checkpoint at %d with solution\n",currentstep);
        numcheckpoints--;
      }
      if (lastcheckpointtype == 1) {
        printf("Store checkpoint at %d with stage values\n",currentstep);
        numcheckpoints -= l;
      }
      if (lastcheckpointtype == 2) {
        printf("Error\n");
        numcheckpoints -= l+1;
      }
    }
  }
  offline_cams_destroy();

  l = 2;
  offline_cams_create(m,s,l,0); // normal
  printf("\n\nCAMS result for (%d,%d) = %d \n",s,m,numfwdstep_cams(m,s,l));
#if defined(DEBUG)
  printstates_cams(m,s,l);
#endif
  currentstep = 0;
  numcheckpoints = s;
  offline_cams(-1,-1,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
  if (!nextcheckpointstep) {
    offline_cams(0,0,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
    printf("Store checkpoint at %d with solution\n",currentstep);
    numcheckpoints--;
  }
  while (currentstep<m) {
    currentstep++;
    if (currentstep == nextcheckpointstep) {
      lastcheckpointstep = currentstep;
      lastcheckpointtype = nextcheckpointtype;
      offline_cams(lastcheckpointstep,lastcheckpointtype,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
      //printf("laststep=%d lasttype=%d s=%d m=%d l=%d nextstep=%d nexttype=%d\n",lastcheckpointstep,lastcheckpointtype,numcheckpoints,m,l,nextcheckpointstep,nextcheckpointtype);
      if (lastcheckpointtype == 0) {
        printf("Store checkpoint at %d with solution\n",currentstep);
        numcheckpoints--;
      }
      if (lastcheckpointtype == 1) {
        printf("Store checkpoint at %d with stage values\n",currentstep);
        numcheckpoints -= l;
      }
      if (lastcheckpointtype == 2) {
        printf("Store checkpoint at %d with stage values and solution\n",currentstep);
        numcheckpoints -= l+1;
      }
    }
  }

  offline_cams_destroy();
  return 1;
}
