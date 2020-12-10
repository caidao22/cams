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
  int m,s,l=1;

  if (argc!=3) {
      printf("Wrong command line options. Must input number of checkpoints available and number of time steps (two integers separated by space).\n");
      exit(0);
  }
  s = atoi(argv[1]);
  m = atoi(argv[2]);

  offline_ac_create(m,s);
  printf("AC result for P(%d,%d) = %d correct result should be %d\n",s,m,numfwdstep_ac(m,s),numfwdstep(m,s));
  offline_ac_destroy();

  offline_acms_create(m,s,l);
  printstates_acms(m,s,l);
  printf("ACMS result for P(%d,%d) = %d \n",s,m,numfwdstep_acms(m,s,l));
  printpath_acms(m,s,l);
  offline_acms_destroy();
  return 1;
}
