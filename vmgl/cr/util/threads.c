/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include <stdio.h>
#include "cr_threads.h"
#include "cr_error.h"

/* perror() messages */
#define INIT_TSD_ERROR "InitTSD: failed to allocate key"
#define FREE_TSD_ERROR "FreeTSD: failed to destroy key"
#define SET_TSD_ERROR "InitTSD: thread failed to set thread specific data"
#define GET_TSD_ERROR "InitTSD: failed to get thread specific data"

/* Magic number to determine if a CRtsd has been initialized */
#define INIT_MAGIC 0xff8adc98

/**
 * Create a new thread.
 * \param thread  returns the new thread's info/handle
 * \param flags  optional flags (none at this time)
 * \param threadFunc  the function to run in new thread
 * \param arg  argument to pass to threadFunc
 * \return 0 for success, non-zero if error
 */
int
crCreateThread(CRthread *thread, int flags,
		void * (*threadFunc)(void *), void *arg)
{
	int i = pthread_create(thread, NULL, threadFunc, arg);
	return i;
}


/**
 * Return ID of calling thread.
 */
CRthread
crThreadID(void)
{
	return pthread_self();
}



/**
 * Initialize a thread-specific data handle, with destructor function.
 */
void crInitTSDF(CRtsd *tsd, void (*destructor)(void *))
{
	if (tsd->initMagic != (int) INIT_MAGIC) {
		if (pthread_key_create(&tsd->key, destructor) != 0) {
			perror(INIT_TSD_ERROR);
			crError("crInitTSD failed!");
		}
	}
	tsd->initMagic = INIT_MAGIC;
}
 
 
/**
 * Initialize a thread-specific data handle.
 */
void crInitTSD(CRtsd *tsd)
{
    crInitTSDF(tsd, NULL);
}


void crFreeTSD(CRtsd *tsd)
{
	if (pthread_key_delete(tsd->key) != 0) {
		perror(FREE_TSD_ERROR);
		crError("crFreeTSD failed!");
	}
	tsd->initMagic = 0x0;
}


/* Set thread-specific data */
void crSetTSD(CRtsd *tsd, void *ptr)
{
	CRASSERT(tsd->initMagic == (int) INIT_MAGIC);
	if (tsd->initMagic != (int) INIT_MAGIC) {
		/* initialize this CRtsd */
		crInitTSD(tsd);
	}
	if (pthread_setspecific(tsd->key, ptr) != 0) {
		crError("crSetTSD failed!");
	}
}


void crInitMutex(CRmutex *mutex)
{
	pthread_mutex_init(mutex, NULL);
}


void crFreeMutex(CRmutex *mutex)
{
	pthread_mutex_destroy(mutex);
}


void crLockMutex(CRmutex *mutex)
{
	pthread_mutex_lock(mutex);
}


void crUnlockMutex(CRmutex *mutex)
{
	pthread_mutex_unlock(mutex);
}


void crInitCondition(CRcondition *cond)
{
	int err = pthread_cond_init(cond, NULL);
	if (err) {
		crError("crInitCondition failed");
	}
}


void crFreeCondition(CRcondition *cond)
{
	int err = pthread_cond_destroy(cond);
	if (err) {
		crError("crFreeCondition error (threads waiting on the condition?)");
	}
}

/**
 * We're basically just wrapping the pthread condition var interface.
 * See the man page for pthread_cond_wait to learn about the mutex parameter.
 */
void crWaitCondition(CRcondition *cond, CRmutex *mutex)
{
	pthread_cond_wait(cond, mutex);
}


void crSignalCondition(CRcondition *cond)
{
	pthread_cond_signal(cond);
}


void crInitBarrier(CRbarrier *b, unsigned int count)
{
	b->count = count;
	b->waiting = 0;
	pthread_cond_init( &(b->cond), NULL );
	pthread_mutex_init( &(b->mutex), NULL );
}


void crFreeBarrier(CRbarrier *b)
{
	/* XXX anything to do? */
}


void crWaitBarrier(CRbarrier *b)
{
	pthread_mutex_lock( &(b->mutex) );
	b->waiting++;
	if (b->waiting < b->count) {
		pthread_cond_wait( &(b->cond), &(b->mutex) );
	}
	else {
		pthread_cond_broadcast( &(b->cond) );
		b->waiting = 0;
	}
	pthread_mutex_unlock( &(b->mutex) );
}


void crInitSemaphore(CRsemaphore *s, unsigned int count)
{
	sem_init(s, 0, count);
}


void crWaitSemaphore(CRsemaphore *s)
{
	sem_wait(s);
}


void crSignalSemaphore(CRsemaphore *s)
{
	sem_post(s);
}

