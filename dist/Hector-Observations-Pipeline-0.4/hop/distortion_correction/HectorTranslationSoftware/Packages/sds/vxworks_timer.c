/*
 *  T I M E R . C
 *
 *  VxWorks implementations of the routines timer_start and cpu_used
 *  and elapsed_time.  Note that there is no time-of-day clock used
 *  and the same value will be returned by both cpu_used and
 *  elapsed_time, since VxWorks doesn't report statistics for CPU
 *  usage of individual tasks.
 *
 *  Also note that the structure of the routines is not re-entrant -
 *  any task calling timer_start() will reset the timer.
 */



static const char *rcsId="@(#) $Id: ACMM:sds/vxworks_timer.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include "vxWorks.h"

static ULONG start;

timer_start()
{
   start = tickGet();
}

cpu_used (secs)
   double *secs;
{
   ULONG stop;
   stop = tickGet();
   *secs = (stop-start)/60.0;
} 

elapsed_time (secs)
   double *secs;
{

   ULONG stop;
   stop = tickGet();
   *secs = (stop-start)/60.0;
}

