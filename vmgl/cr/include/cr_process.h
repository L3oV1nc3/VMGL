/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PROCESS_H
#define CR_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Process ID type
 */
typedef unsigned long CRpid;


extern void crSleep( unsigned int seconds );

extern void crMsleep( unsigned int msec );

extern CRpid crSpawn( const char *command, const char *argv[] );

extern void crKill( CRpid pid );

extern void crGetProcName( char *name, int maxLen );

extern void crGetCurrentDir( char *dir, int maxLen );

extern CRpid crGetPID(void);


#ifdef __cplusplus
}
#endif

#endif /* CR_PROCESS_H */
