
#ifdef __cplusplus
extern "C"{
#endif

int revolve(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep);
int numfwdstep(int m, int s);

int offline_acms_create(int m, int s, int l);
int offline_acms_destroy();
int offline_acms(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep, int *nextcheckpointtype);
int numfwdstep_acms(int m, int s, int l);
void printpath_acms(int m, int s, int l);
void printstates_acms(int m, int s, int l);

int offline_ac_create(int m, int s);
int offline_ac_destroy();
int offline_ac(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep);
int numfwdstep_ac(int m, int s);

#ifdef __cplusplus
}
#endif
