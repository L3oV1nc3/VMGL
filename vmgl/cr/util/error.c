/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_environment.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_net.h"
#include "cr_process.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

static char my_hostname[256];
static int my_pid = 0;
static int canada = 0;
static int swedish_chef = 0;
static int australia = 0;
static int warnings_enabled = 1;

static void __getHostInfo( void )
{
	char *temp;
	if ( crGetHostname( my_hostname, sizeof( my_hostname ) ) )
	{
		crStrcpy( my_hostname, "????" );
	}
	temp = crStrchr( my_hostname, '.' );
	if (temp)
	{
		*temp = '\0';
	}
	my_pid = crGetPID();
}

static void __crCheckCanada(void)
{
	static int first = 1;
	if (first)
	{
		const char *env = crGetenv( "CR_CANADA" );
		if (env)
			canada = 1;
		first = 0;
	}
}

static void __crCheckSwedishChef(void)
{
	static int first = 1;
	if (first)
	{
		const char *env = crGetenv( "CR_SWEDEN" );
		if (env)
			swedish_chef = 1;
		first = 0;
	}
}

static void __crCheckAustralia(void)
{
	static int first = 1;
	if (first)
	{
		const char *env = crGetenv( "CR_AUSTRALIA" );
		const char *env2 = crGetenv( "CR_AUSSIE" );
		if (env || env2)
			australia = 1;
		first = 0;
	}
}

static void outputChromiumMessage( FILE *output, char *str )
{
	fprintf( output, "%s%s%s%s\n", str, 
			swedish_chef ? " BORK BORK BORK!" : "",
			canada ? ", eh?" : "",
			australia ? ", mate!" : ""
			);
	fflush( output );
}

void crError( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;

	__crCheckCanada();
	__crCheckSwedishChef();
	__crCheckAustralia();

	if (!my_hostname[0])
	    __getHostInfo();
	offset = sprintf( txt, "CR Error(%s:%d): ", my_hostname, my_pid );

	va_start( args, format );
	vsprintf( txt + offset, format, args );
	va_end( args );

	outputChromiumMessage( stderr, txt );

	/* Give chance for things to close down */
	raise( SIGTERM );
	exit(1);
}

void crEnableWarnings(int onOff)
{
	warnings_enabled = onOff;
}

void crWarning( char *format, ... )
{
	if (warnings_enabled) {
		va_list args;
		static char txt[8092];
		int offset;

		__crCheckCanada();
		__crCheckSwedishChef();
		__crCheckAustralia();
		if (!my_hostname[0])
			__getHostInfo();
		offset = sprintf( txt, "CR Warning(%s:%d): ", my_hostname, my_pid );
		va_start( args, format );
		vsprintf( txt + offset, format, args );
		outputChromiumMessage( stderr, txt );
		va_end( args );
	}
}

void crInfo( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;

	__crCheckCanada();
	__crCheckSwedishChef();
	__crCheckAustralia();
	if (!my_hostname[0])
		__getHostInfo();
	offset = sprintf( txt, "CR Info(%s:%d): ", my_hostname, my_pid );
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	outputChromiumMessage( stderr, txt );
	va_end( args );
}

void crDebug( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;
	static FILE *output;
	static int first_time = 1;
	static int silent = 0;

	if (first_time)
	{
		const char *fname = crGetenv( "CR_DEBUG_FILE" );
		first_time = 0;
		if (fname)
		{
			char debugFile[1000], *p;
			crStrcpy(debugFile, fname);
			p = crStrstr(debugFile, "%p");
			if (p) {
				/* replace %p with process number */
				unsigned long n = (unsigned long) crGetPID();
				sprintf(p, "%lu", n);
			}
			fname = debugFile;
			output = fopen( fname, "w" );
			if (!output)
			{
				crError( "Couldn't open debug log %s", fname ); 
			}
		}
		else
		{
			output = stderr;
		}
#if NDEBUG
		/* Release mode: only emit crDebug messages if CR_DEBUG
		 * or CR_DEBUG_FILE is set.
		 */
		if (!fname && !crGetenv("CR_DEBUG"))
			silent = 1;
#endif
	}

	if (silent)
		return;

	__crCheckCanada();
	__crCheckSwedishChef();
	__crCheckAustralia();
	if (!my_hostname[0])
		__getHostInfo();

	offset = sprintf( txt, "CR Debug(%s:%d): ", my_hostname, my_pid );
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	outputChromiumMessage( output, txt );
	va_end( args );
}
