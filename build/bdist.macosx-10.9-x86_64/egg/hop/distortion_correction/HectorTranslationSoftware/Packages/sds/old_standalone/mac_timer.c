/*
 *  T I M E R . C
 *
 *  MAC implementation of the routines timer_start and cpu_used
 *  and elapsed_time.
 */
 


static char *rcsId="@(#) $Id: ACMM:sds/old_standalone/mac_timer.c,v 3.94 09-Dec-2020 17:15:48+11 ks $";
static void *use_rcsId = ((void)&use_rcsId,(void *) &rcsId);


#include <events.h>
 
static long StartTick;

timer_start()
{
	StartTick = TickCount (); 
}

cpu_used(secs)
   double *secs;
{
	*secs = ((double) (TickCount() - StartTick)) / 60.0;
}

elapsed_time(secs)
   double *secs;
{
	cpu_used (secs);
}

