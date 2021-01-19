/*
 *  @(#) $Id: ACMM:sds/timer.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
 *
 *  T I M E R . C
 *
 *  VAX C implementation of the routines timer_start and cpu_used
 *  and elapsed_time.
 */

#define CPU   2

#define LIB$K_DELTA_SECONDS_F 30

typedef struct  { unsigned int lslw;
		      int mslw; } VmsTimeType;

extern long int lib$init_timer(unsigned long *context);
extern long int lib$stat_timer(long * code, void * value_arg, 
				unsigned long *handle);
extern long int sys$gettim(VmsTimeType *time);
extern long int lib$sub_times(VmsTimeType *time1, VmsTimeType *time2,
			      VmsTimeType *resultant);
extern long int lib$cvtf_from_internal_time(unsigned long int *operation,
					   float *resultant,
					   VmsTimeType *input_time);


static long start;
static VmsTimeType estart;
static unsigned long context;

timer_start()
{
   long int cpu = CPU ;
   context = 0;
   lib$init_timer(&context);
   lib$stat_timer (&cpu,&start,&context);     /* Get start cpu time */
   sys$gettim (&estart);                      /* Get start actual time */

}

cpu_used(secs)
   double *secs;
{
   long stop;
   long int cpu = CPU;

   lib$stat_timer(&cpu,&stop,&context);       /* Get end cpu time */
   *secs=(stop-start)/100.0;                  /* CPU time in secs. */
}

elapsed_time(secs)
   double *secs;
{
   VmsTimeType estop,etime;
   float fsec;
   unsigned long int code = LIB$K_DELTA_SECONDS_F;

   sys$gettim (&estop);
   lib$sub_times (&estop,&estart,&etime);
   lib$cvtf_from_internal_time(&code,&fsec,&etime);
   *secs = (double) fsec;
}

