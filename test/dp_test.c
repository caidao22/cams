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
  int m,s,l = 1,currentstep = 0,lastcheckpointstep,lastcheckpointtype,nextcheckpointstep,nextcheckpointtype,numcheckpoints;

  if (argc!=3) {
      printf("Wrong command line options. Must input number of checkpoints available and number of time steps (two integers separated by space).\n");
      exit(0);
  }
  s = atoi(argv[1]);
  m = atoi(argv[2]);

  offline_ac_create(m,s);
  printf("AC result for P(%d,%d) = %d correct result should be %d\n",s,m,numfwdstep_ac(m,s),numfwdstep(m,s));

#if defined(DEBUG)
  printstates_ac(m,s);
  printpath_ac(m,s);
#endif
  numcheckpoints = s;
  offline_ac(-1,numcheckpoints,m,&nextcheckpointstep);
  while (currentstep<m) {
    if (currentstep == nextcheckpointstep) {
      printf("Store checkpoint at %d\n",currentstep);
      lastcheckpointstep = currentstep;
      offline_ac(lastcheckpointstep,numcheckpoints,m,&nextcheckpointstep);
      numcheckpoints--;
    }
    currentstep++;
  }
  offline_ac_destroy();

  offline_acms_create(m,s,l);
  printf("ACMS result for P(%d,%d) = %d \n",s,m,numfwdstep_acms(m,s,l));
#if defined(DEBUG)
  printstates_acms(m,s,l);
  printpath_acms(m,s,l);
#endif
  currentstep = 0;
  numcheckpoints = s;
  offline_acms(-1,0,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
  while (currentstep<m) {
    if (currentstep == nextcheckpointstep) {
      lastcheckpointtype = nextcheckpointtype;
      lastcheckpointstep = currentstep;
      offline_acms(lastcheckpointstep,lastcheckpointtype,numcheckpoints,m,l,&nextcheckpointstep,&nextcheckpointtype);
      if (lastcheckpointtype == 0) {
        printf("Store checkpoint at %d\n",currentstep);
        numcheckpoints--;
      } else {
        printf("Store checkpoint at %d with stage values\n",currentstep);
        numcheckpoints -= l+1;
      }
    }
    currentstep++;
  }
  offline_acms_destroy();
  return 1;
}
