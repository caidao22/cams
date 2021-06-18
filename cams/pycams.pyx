# file: pycams.pyx
#cython: language_level=3

cdef extern from "offline_schedule.h":
    int offline_cams_create(int m, int s, int l, int stifflyaccurate) nogil
    int offline_cams_destroy() nogil
    int offline_cams(int lastcheckpointstep, int lastcheckpointtype, int num_checkpoints_avail, int endstep, int num_stage, int *nextcheckpointstep, int *nextcheckpointtype) nogil
    int numfwdstep_cams(int m, int s, int l) nogil

def offline_create(int m, int s, int l, int stifflyaccurate):
    return offline_cams_create(m,s,l,stifflyaccurate)

def offline_destroy():
    return offline_cams_destroy()

def offline(int lastcheckpointstep, int lastcheckpointtype, int num_checkpoints_avail, int endstep, int num_stage):
    cdef int nextcheckpointstep = 0
    cdef int nextcheckpointtype = 0
    offline_cams(lastcheckpointstep,lastcheckpointtype,num_checkpoints_avail,endstep,num_stage,&nextcheckpointstep,&nextcheckpointtype)
    return (nextcheckpointstep,nextcheckpointtype)

def numfwdstep(int m, int s, int l):
    return numfwdstep_cams(m,s,l)
