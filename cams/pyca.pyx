# file: pyac.pyx
#cython: language_level=3

cdef extern from "offline_schedule.h":
    int offline_ca_create(int m, int s) nogil
    int offline_ca_destroy() nogil
    int offline_ca(int lastcheckpointstep, int num_checkpoints_avail, int endstep, int *nextcheckpointstep) nogil
    int numfwdstep_ca(int m, int s) nogil

def offline_create(int m, int s):
    return offline_ca_create(m,s)

def offline_destroy():
    return offline_ca_destroy()

def offline(int lastcheckpointstep,int num_checkpoints_avail, int endstep, int num_stage):
    cdef int nextcheckpointstep = 0
    offline_ca(lastcheckpointstep,num_checkpoints_avail,endstep,&nextcheckpointstep)
    return nextcheckpointstep

def numfwdstep(int m, int s):
    return numfwdstep_ca(m,s)
