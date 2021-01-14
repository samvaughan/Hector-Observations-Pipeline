/*
 *  T I M E R . C
 *
 *  UNIX implementations of the routines timer_start and cpu_used
 *  and elapsed_time.
 *
 *     @(#) $Id: ACMM:sds/unix_timer.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
 */



static const char *rcsId="@(#) $Id: ACMM:sds/unix_timer.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <sys/types.h>
#include <sys/times.h>
#include <time.h>

static unsigned long start;
static time_t estart;

void timer_start()
{
   struct tms buffer;

   times (&buffer);
   start = buffer.tms_utime;
   time (&estart);
}

void cpu_used (secs)
   double *secs;
{
   struct tms buffer;
  
   unsigned long stop;
 
   times (&buffer);
   stop = buffer.tms_utime;
   *secs = (stop-start)/60.0;
} 

void elapsed_time (secs)
   double *secs;
{

   time_t estop;

   time (&estop);
   *secs = (double) estop - (double) estart;
}

