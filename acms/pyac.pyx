# file: pyac.pyx
#cython: language_level=3

cdef extern from "offline_schedule.h":
    int offline_ac_create(int m, int s) nogil
    int offline_ac_destroy() nogil
    int offline_ac(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep) nogil
    int numfwdstep_ac(int m, int s) nogil

def offline_create(int m, int s):
    return offline_ac_create(m,s)

def offline_destroy():
    return offline_ac_destroy()

def offline(int lastcheckpointstep,int num_checkpoints_avail, int endstep, int num_stage):
    cdef int nextcheckpointstep = 0
    offline_ac(lastcheckpointstep,num_checkpoints_avail,endstep,&nextcheckpointstep)
    return nextcheckpointstep

def numfwdstep(int m, int s):
    return numfwdstep_ac(m,s)
