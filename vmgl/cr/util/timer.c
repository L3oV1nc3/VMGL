
#include <stddef.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <unistd.h>

#include "cr_timer.h"
#include "cr_mem.h"
#include "cr_error.h"

static double crTimerGetTime( CRTimer *t )
{
#if defined( Linux ) || defined(SunOS) || defined(FreeBSD)
	gettimeofday( &t->timeofday, NULL );	
	return t->timeofday.tv_sec + t->timeofday.tv_usec / 1000000.0;
#else
#error TIMERS
#endif
}

CRTimer *crTimerNewTimer( void )
{
	CRTimer *t = (CRTimer *) crAlloc( sizeof(*t) );
	t->time0 = t->elapsed = 0;
	t->running = 0;
	return t;
}

void crDestroyTimer( CRTimer *t )
{
	crFree( t );
}

void crStartTimer( CRTimer *t )
{
    t->running = 1;
    t->time0 = crTimerGetTime( t );
}

void crStopTimer( CRTimer *t )
{
    t->running = 0;
    t->elapsed += crTimerGetTime( t ) - t->time0;
}

void crResetTimer( CRTimer *t )
{
    t->running = 0;
    t->elapsed = 0;
}

double crTimerTime( CRTimer *t )
{
	if (t->running) {
		crStopTimer( t );
		crStartTimer( t );
	}
	return t->elapsed;
}
