/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#ifndef CR_THREADS_H
#define CR_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "chromium.h"
#include "cr_bits.h"

#include <pthread.h>
#include <semaphore.h>


/**
 * Thread ID/handle
 */
typedef pthread_t CRthread;

extern int crCreateThread(CRthread *thread, int flags,
                          void * (*threadFunc)(void *), void *arg);

extern CRthread crThreadID(void);


/*
 * Handle for Thread-Specific Data
 */
typedef struct {
	pthread_key_t key;
	int initMagic;
} CRtsd;


extern void crInitTSD(CRtsd *tsd);
extern void crInitTSDF(CRtsd *tsd, void (*destructor)(void *));
extern void crFreeTSD(CRtsd *tsd);
extern void crSetTSD(CRtsd *tsd, void *ptr);
#define crGetTSD(TSD) pthread_getspecific((TSD)->key)


/* Mutex datatype */
typedef pthread_mutex_t CRmutex;

extern void crInitMutex(CRmutex *mutex);
extern void crFreeMutex(CRmutex *mutex);
extern void crLockMutex(CRmutex *mutex);
extern void crUnlockMutex(CRmutex *mutex);


/* Condition variable datatype */
typedef pthread_cond_t CRcondition;

extern void crInitCondition(CRcondition *cond);
extern void crFreeCondition(CRcondition *cond);
extern void crWaitCondition(CRcondition *cond, CRmutex *mutex);
extern void crSignalCondition(CRcondition *cond);


/* Barrier datatype */
typedef struct {
	unsigned int count;
	unsigned int waiting;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
} CRbarrier;

extern void crInitBarrier(CRbarrier *b, unsigned int count);
extern void crFreeBarrier(CRbarrier *b);
extern void crWaitBarrier(CRbarrier *b);


/* Semaphores */
	typedef sem_t CRsemaphore;

extern void crInitSemaphore(CRsemaphore *s, unsigned int count);
extern void crWaitSemaphore(CRsemaphore *s);
extern void crSignalSemaphore(CRsemaphore *s);


#ifdef __cplusplus
}
#endif

#endif /* CR_THREADS_H */
