/*
;+
;                            G E N _ Q F M E D
;
;  Routine name:
;       GEN_QFMED
;
;  Function:
;       Finds the median value of a real data array.
;
;  Description:
;       Finds the median value of the NELM elements of the array DATA.  
;       DATA is conceptually sorted into ascending order and the median 
;       value is then either the value in the central element (if NELM 
;       is odd) or the average of the two central elements (if NELM is 
;       even). DATA is not in fact fully sorted, but its elements are 
;       re-aranged as far as is needed to determine the median.
;
;  Language:
;       Assembly, for use from Fortran or other high level languages.
;
;  Call:   (from FORTRAN)
;
;       VALUE = GEN_QFMED (DATA,N)
;
;  Parameters -   (">" input, "!" modified, "<" output)
;
;       (!) DATA    (Real array DATA(NELM), ref) Passed containing the
;                   array values to be sorted, returned with them
;                   jumbled somewhat.
;       (>) NELM    (Integer, ref) Number of elements in DATA.
;
;  Returns -
;
;       (<) VALUE   (Real, function value) The median value required.
;
;  External subroutines / functions used: None.
;
;  External variables used: None.
;
;  Author: Keith Shortridge, CIT
;
;  Date:  30th Jan 1984
;
;  Note:
;        This is a modification of the quicksort routine
;        GEN_QFSORT, which does not bother with a full sort.
;        Based on the Forth assembler routine by WRS (KPNO).
;        Converted to VAX/VMS assembler by KS (CIT)
;
;  Method:
;        See any book on sorting (eg Knuth, The Art of Computer
;        Programming, vol 3,  Addison-Wesley).
;
;  Modified: 
;
;  21st Nov 1988.  CKL/CIT. Converted to C for Sun.
;  31st Jul 1989.  SNS/CIT. Sun f77 bug fixed.
;  23rd Sep 1992.  HME/UoE, Starlink. No conditional compilations: Always
;                  include <math.h>, always use "float".
;  23rd Oct 1992.  hme / UoE, Starlink.  Do not declare qqfmed within the
;                  functions. Do not declare qqfmed as static, but prototype
;                  it as such.
;   9th Apr 1993.  KS/AAO. Changed order of routines, since the previous
;                  version didn't work under ULTRIX.
;  30th Apr 1993.  KS/AAO. Added the mess about FLOATFUNCTION etc. This now
;                  compiles without warnings and works under both CC and GCC
;                  on both a SUN and a DECStation. It assumes the pre-processor
;                  variable Sun will be set properly.
;  27th Jan 1996.  KS/AAO. Added gen_qfmed__ entry (2 underscores) to allow
;                  a Fortran program compiled under f2c to call this routine.
;+
*/

#include <math.h>

/*  The following is an awful mess, but is needed to get the correct
 *  result on SUNs using both CC and GCC without generating warnings.
 *  All this is because SUN CC doesn't return floats properly, but
 *  GNU C does.
 */


#ifdef Sun
#ifdef __GNUC__
/*  On SUNs using GCC, use the ordinary definitions */
#define float_function float
#define return_float return
#else
/*  On SUNS using CC, use the SUN supplied kluges */
#define float_function FLOATFUNCTIONTYPE
#define return_float RETURNFLOAT
#endif
#else
/*  On other machines, use the ordinary definitions */
#define float_function float
#define return_float return
#endif

/*  End of the awful mess. Back to the code. */

static void qqfmed(first,last,data,cent)
int first,last;
float data[];
int cent;
{
   int i,j;
   float temp,vmid;

   i = first;
   j = last;
   vmid = data[(i+j) / 2];
   do {
      while (data[i] < vmid)
         i++;
      while (vmid < data[j])
         j--;
      if (i <= j) {
         temp = data[i]; data[i] = data[j]; data[j] = temp;
         i++; j--;
      }
   } while (i <= j);

   if ((first < j) && (cent <= j))
      (void) qqfmed(first,j,data,cent);
   if ((i <= cent) && (i < last))
      (void) qqfmed(i,last,data,cent);
}


float_function gen_qfmed_(data,nelm)
float data[];
int *nelm;
{
   int cent,i;
   float median,temp;

   cent = (*nelm / 2);                 /* C indices start at 0 */
   (void) qqfmed(0,*nelm-1,data,cent);
   median = data[cent];
   if ((*nelm % 2) == 0) {             /* if NELM is even, look for   */
      temp = data[cent-1];             /* highest value in the low    */
      for (i = 0; i < cent-1; i++)     /* partition, then use average */
         if (data[i] > temp)
            temp = data[i];
      median = (median+temp)*0.5;
   }
   return_float (median);
}

/*  This version has the naming conventions followed by f2c, allowing
 *  a Fortran program compiled under f2c to call this routine.
 */
 
float_function gen_qfmed__(data,nelm)
float data[];
int *nelm;
{
	return_float (gen_qfmed_(data,nelm));
}

/*
;       Appendix: The following PASCAL program is as close as one
;       can get to the Macro version.  It was written to make sure
;       that the workings of the algorithm were understood before it
;       was coded in Macro.  PASCAL was used rather than FORTRAN
;       because you can't use Fortran recursively.  It may make the
;       operation  of the Macro a little clearer..
;       
;       MODULE QFMED(OUTPUT);
;
;       [GLOBAL] FUNCTION QFMED
;          (VAR DATA : ARRAY[LOW_INDEX..HIGH_INDEX : INTEGER] OF REAL;
;           VAR NELM : INTEGER): REAL;
;
;          { Finds the median value of the NELM elements of the array DATA }
;
;          VAR CENT,I: INTEGER;
;          VAR MEDIAN,TEMP: REAL;
;
;          PROCEDURE QQFMED(FIRST,LAST:INTEGER);
;
;             { Once QFMED has set CENT to give the center element of the
;               array DATA, it calls QQFMED which homes in on the value
;               that should be in that element.  It does that using an
;               algorithm that is a modified quicksort, splitting the
;               partition DATA[FIRST..LAST] into two partitions, one
;               with all values above the initial central value, one with
;               all values below it, and then calling itself to repeat the
;               operation on the partition that contains the central 
;               element }
; 
;            VAR
;               I,J       : INTEGER;
;               TEMP,VMID : REAL;
;     
;            BEGIN
;               I:=FIRST;
;               J:=LAST;
;               VMID:=DATA[(I+J) DIV 2];        
;               REPEAT
;                  WHILE DATA[I]<VMID DO I:=I+1;
;                  WHILE VMID<DATA[J] DO J:=J-1;
;                  IF I<=J THEN BEGIN
;                     TEMP:=DATA[I]; DATA[I]:=DATA[J]; DATA[J]:=TEMP;
;                     I:=I+1; J:=J-1;
;                  END;
;               UNTIL I>J;
;               IF (FIRST<J) AND (CENT<=J) THEN QQFMED(FIRST,J);
;               IF (I<=CENT) AND (I<LAST)  THEN QQFMED(I,LAST);
;             END;  
;
;         { End of QQFMED }
;
;         BEGIN
;            CENT:=(NELM DIV 2)+1;
;            QQFMED(1,NELM);
;            MEDIAN:=DATA[CENT];               { if NELM is even, look for   }
;            IF (NELM MOD 2) = 0 THEN BEGIN    { highest value in the low    }
;               TEMP:=DATA[CENT-1];            { partition, then use average }
;               FOR I:= 1 TO CENT-2 DO
;                  IF DATA[I] > TEMP THEN TEMP:=DATA[I];
;               MEDIAN:=(MEDIAN+TEMP)*0.5;
;            END;
;            QFMED:=MEDIAN
;         END;
;
;         { End of QFMED }
;
;      END.
;
*/
