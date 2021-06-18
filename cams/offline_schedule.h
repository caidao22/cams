
#ifdef __cplusplus
extern "C"{
#endif

int revolve(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep);
int numfwdstep(int m, int s);

int offline_cams_create(int m, int s, int l, int stifflyaccurate);
int offline_cams_destroy();
int offline_cams(int lastcheckpointstep, int lastcheckpointtype, int num_checkpoints_avail, int endstep, int num_stage, int *nextcheckpointstep, int *nextcheckpointtype);
int numfwdstep_cams(int m, int s, int l);

#if defined(DEBUG)
void printstates_cams(int m, int s, int l);
#endif

int offline_ca_create(int m, int s);
int offline_ca_destroy();
int offline_ca(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep);
int numfwdstep_ca(int m, int s);

#if defined(DEBUG)
void printpath_ca(int m, int s);
void printstates_ca(int m, int s);
#endif

#ifdef __cplusplus
}
#endif
