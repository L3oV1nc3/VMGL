#ifndef CR_TIMER_H
#define CR_TIMER_H

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Timer 
{
	double time0, elapsed;
	char running;

	int fd;
#if defined( Linux ) || defined (SunOS) || defined(FreeBSD)
	struct timeval timeofday;
#endif
} CRTimer;

CRTimer *crTimerNewTimer( void );
void crDestroyTimer( CRTimer *t );
void crStartTimer( CRTimer *t );
void crStopTimer( CRTimer *t );
void crResetTimer( CRTimer *t );
double crTimerTime( CRTimer *t );

#ifdef __cplusplus
}
#endif

#endif /* CR_TIMER_H */
