/*
 * 2dF routines for the conversion of RA/Dec positions to robot
 * X/Y positions.  See TexDocs/2df_pos_16.texi for details.
 */

static char *rcsId="@(#) $Id: ACMM:2dFutil/tdfxy.c,v 5.14 25-Jun-2020 14:50:30+10 tjf $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "status.h"
#include "slalib.h"
#include "slamac.h" 
#include "sds.h"
#include "Ers.h"


#include "tdfxy.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define DEBUG printf

/*
 *  Macro used to tag parameters as known to be unused and hence
 * prevent warnings when compiled with strict warning under GNU C.
 */
#ifndef DUNUSED
#if defined(__GNUC__) || defined(GNUC)
#if __GNUC_MINOR__ >= 7 || __GNUC__ > 2
#       define DUNUSED __attribute__ ((unused))
#else
#       define DUNUSED
#endif
#endif
#endif

/*
 * Parameter ranges for the parameters to TdfXyInit() - see ValidateArguments.
 */
#define TEMP_MIN 250.0  /* Min temperature we expect to see  -23 C */
#define TEMP_MAX 330.0  /* Max temperature we expect to see  +57 C */
#define PRES_MIN 200.0  /* Min pressure we expect to see - millibars */
#define PRES_MAX 1000.0 /* Max pressure we expect to see - millibars */
#define HUMI_MIN 0.0    /* Min humidity we expect to see - relative (0%) */
#define HUMI_MAX 1.0    /* Max humidity we expect to see - relative (100%) */
#define WAVE_MIN 0.3    /* Min wavelength we expect to see - microns (300 nanometers) */
#define WAVE_MAX 1.0    /* Min wavelength we expect to see - microns (1000 nanometers) */
/*
 *      @(#) $Id: ACMM:2dFutil/tdfxy.c,v 5.14 25-Jun-2020 14:50:30+10 tjf $
 */

/*
 *  The MA and ME values of the AAT do not change by enougth
 *  to impact 2dF (it could impact the field rotation but that can
 *  be corrected by the 2dF plate rotator).  So, to avoid the user
 *  having to specify them and getting them wrong, specify them here in
 *  arc-seconds.  (ME updated as of polar axis realignment in March 2016).
 */
#define AAT_MA 30.2999999
#define AAT_ME 84.0


/*
 * Validate the input arguments - these can get messed up, if , for example,
 * the weather system fails.  Note - we might modify humidity to ensure
 * it is non-zero.
 */
static void ValidateArguments(
    double temp,
    double press,
    double *humid,
    double cenWave,
    double obsWave,
    StatusType *status)
{
    if (*status != STATUS__OK) return;
    if ((temp < TEMP_MIN)||(temp > TEMP_MAX))
    {
        *status = TDFXY__ILLTEMP;
        ErsRep(0, status, 
               "The temperature value of %g kelvin is outside the expected range of %g to %g",
               temp, TEMP_MIN, TEMP_MAX);
        return;
    }
   if ((press < PRES_MIN)||(press > PRES_MAX))
   {
        *status = TDFXY__ILLPRES;
        ErsRep(0, status,
               "The pressure value of %g miliibars is outside the expected range of %g to %g",
               press, PRES_MIN, PRES_MAX);
        return;
   }
   if ((*humid < HUMI_MIN)||(*humid > HUMI_MAX))
   {
       *status = TDFXY__ILLHUMI;
        ErsRep(0, status,
               "The relative humidity value of %g  is outside the expected range of %g to %g",
               *humid, HUMI_MIN, HUMI_MAX);
        return;
   }
   else if (*humid == 0)
   {
       /*
        * Just in case slaLib has trouble with humidty of zero, which is not
        * impossible, through is very unlikely at the AAT.
        */
       *humid = 0.1;
   }
   if ((cenWave < WAVE_MIN)||(cenWave > WAVE_MAX))
      {
        *status = TDFXY__ILLWAVE;
        ErsRep(0, status,
              "The centre wavelength value of %.3g microns is outside the expected range of %.3g to %.3g",
                cenWave, WAVE_MIN, WAVE_MAX);
        return;
      }

   if ((obsWave < WAVE_MIN)||(obsWave > WAVE_MAX))
      {
        *status = TDFXY__ILLWAVE;
        ErsRep(0, status,
              "The observation wavelength value of %.3g microns is outside the expected range of %.3g to %.3g",
                obsWave, WAVE_MIN, WAVE_MAX);
        return;
      }
}
static void ReportInvalidFieldDetails(
    double cra, 
    double cdec, 
    double ra, 
    double dec, 
    StatusType status)
{
    char ra_sign;
    char dec_sign;
    int hmsf[4];
    int dmsf[4];

    ErsRep(0, &status, "Invalid argument range in call to TdfRd2xy");

    slaDr2tf(2, cra, &ra_sign, hmsf);
    slaDr2af(2, cdec, &dec_sign, dmsf);
    ErsRep(0, &status,
           "Supplied Field Centre (app) RA = %.6f(%c%.2d:%.2d:%.2d.%d) Dec = %.6f(%c%.2d:%.2d:%.2d.%d)",
           cra,
           ra_sign, hmsf[0], hmsf[1], hmsf[2], hmsf[3],
           cdec,
           dec_sign, dmsf[0], dmsf[1], dmsf[2], dmsf[3]);

    slaDr2tf(2, ra, &ra_sign, hmsf);
    slaDr2af(2, dec, &dec_sign, dmsf);
    
    ErsRep(0, &status,
           "Supplied Object Position (app) RA = %.6f(%c%.2d:%.2d:%.2d.%d) Dec = %.6f(%c%.2d:%.2d:%.2d.%d)",
           cra,
           ra_sign, hmsf[0], hmsf[1], hmsf[2], hmsf[3],
           cdec,
           dec_sign, dmsf[0], dmsf[1], dmsf[2], dmsf[3]);


}
/*
 * Check if we can observer the the specified field center at the
 * specified MJD.
 */
static void CheckMjd(
    double ra,
    double dec,
    double mjd,
    TdfXyType xypars,  /* Pass by value - we will modify the copy*/
    StatusType *status)
{
   double a;                 /* Observed azimuth  */
   double zd;                /* Observed zenith distance  */
   double h;                 /* Observed hour angle  */
   double d;                 /* Observed declination  */
   double r;                 /* Observed right ascension  */

    if (*status != STATUS__OK) return;
    /*
     *  Note some defensive programming here. If the field is essentially
     *  unobservable because of an unrealistic mjd value, then slaAopqk()
     *  will take forever doing the refraction calculations. So we do an
     *  approximate calculation first, disabling refraction by setting the
     *  pressure parameter to zero. If the calculated zenith distance is
     *  less than 90 degrees (~ 1.57 rad) we know the full calculation is
     *  feasible (KS, 22-May-2001, OzPoz fpcolMdl.c).
     *
     */
    xypars.cenAoprms[6] = 0.0;
    slaAopqk(ra,dec, xypars.cenAoprms, &a,&zd,&h,&d,&r);
    if (zd > 70*DD2R)
    {
        char ra_sign;
        char dec_sign;
        char time_sign;
        int hmsf[4];
        int dmsf[4];
        int ymdf[4];
        int thmsf[4];
        int j;
        *status = TDFXY__ZDERR;
        ErsRep(0, status,
               "The selected observation date and time leads to an invalid zenith distance of %.0f degrees.",
               zd/DD2R);

        slaDr2tf(2, ra, &ra_sign, hmsf);
        slaDr2af(2, dec, &dec_sign, dmsf);
        slaDjcal(2, mjd, ymdf, &j);
        slaCd2tf(0, mjd - (int)mjd, &time_sign, thmsf);
 
        ErsRep(0, status,
               "Field Centre (apparent) RA = %c%.2d:%.2d:%.2d.%d, Dec = %c%.2d:%.2d:%.2d.%d  (%.6f, %.6f radians).",
               ra_sign, hmsf[0], hmsf[1], hmsf[2], hmsf[3],
               dec_sign, dmsf[0], dmsf[1], dmsf[2], dmsf[3],
               ra, dec);
        ErsRep(0, status,
               "Selected Observation Date/Time (UT) = %.4d/%.2d/%.2d %.2d:%.2d:%.2d (MJD = %.6f, ).",
               ymdf[0], ymdf[1], ymdf[2], thmsf[0], thmsf[1], thmsf[2], mjd);
    }

    
}

/*+				T d f X y I n i t

 *  Function name:
      TdfXyInit

 *  Function:
      Initialize for transformation of RA, Dec to 2df x,y

 *  Description:
      This function sets up parameters for a transformation of
      apparent Ra, Dec to x,y on the 2dF field plate

      Note - in Nov 2005 it was determined that the only Telescope
      model parameters that were really significant were MA and ME.  The
      rest resulted in a very small rotation effect, which can be removed
      by the 2dF rotator (which was not working for many years).  The
      ADC releated parameters have never been used either.  The MA and
      ME don't change significantly over time and any change can be recovered
      by the 2dF rotator, so their values have been fixed.
      
      As a result, many arguments to this function are now 
      unused (telelscope model and ADC) they can be set as zero.

     
      
 *  Language:
      C

 *  Declaration:
       TdfXyInit(double mjd, double dut, double temp, double press, 
                double humid, double wave, double ma, double me, double np,
                double ch, double hf, double zgt, double adc_a, 
                double adc_b, TdfDistType dist, 
                TdfXyType *xypars, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) mjd       (double)  UTC date and time as modified julian date.
      (>) dut       (double)  Delta UT (UT1 - UTC) seconds.
      (>) temp      (double)  Atmospheric temperature (K).
      (>) press     (double)  Atmospheric pressure (mB).
      (>) humid     (double)  Atmospheric relative humidity (0 to 1.0).
      (>) wave      (double)  Wavelength (microns).
      (>) ma        (double)  Azimuth misalignment of polar axis (arcsec).(UNUSED)
      (>) me        (double)  Elevation misalignment of polar axis (arcsec). (UNUSED)
      (>) np        (double)  Non perpendicularity of axes (arcsec). (UNUSED)
      (>) ch        (double)  Collimation error in HA (arcsec). (NUSED)
      (>) hf        (double)  Horseshoe flexure coefficient  (arcsec). (UNUSED)
      (>) zgt       (double)  z-gear transducer reading . (UNUSED)
      (>) adc_a     (double)  ADC position angle for element A (UNUSED).
      (>) adc_b     (double)  ADC position angle for element B (UNUSED).
      (>) dist      (TdfDistType) 2dF Distortion model parameters.
      (<) xypars    (TdfXyType)  2dF xy Transformation parameters.
      (!) status    (StatusType*) Modified status.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 1-Jul-94
 *-
 */

void TdfXyInit(double mjd, double dut, double temp, double press, 
               double humid, 
               double cenWave, double obsWave, 
               double ma DUNUSED, double me DUNUSED, double np DUNUSED,
               double ch DUNUSED, double hf DUNUSED, 
               double zgt DUNUSED, 
               double adc_a DUNUSED, 
               double adc_b DUNUSED, TdfDistType dist, 
               TdfXyType *xypars, StatusType *status)
{
   double longit,lat,hm;
   double a,b;
   char name[80];

   if (*status != STATUS__OK) return;

   ValidateArguments(temp, press, &humid, cenWave, obsWave, status);
   if (*status != STATUS__OK) return;

/*  Get observatory parameters  */

   slaObs(-1,"AAT",name,&longit,&lat,&hm);

/*  Determine apparent to observed place parameters  */

   slaAoppa(mjd,dut,-longit,lat,hm,0.0,0.0,temp,press,humid,
            cenWave,0.0065,xypars->cenAoprms);
   slaAoppa(mjd,dut,-longit,lat,hm,0.0,0.0,temp,press,humid,
            obsWave,0.0065,xypars->obsAoprms);

  /*  Set up rotation matrix to mount Ha,Dec  */

   a = lat - DPIBY2 - DAS2R*AAT_ME;
   b = DAS2R*AAT_MA;

   xypars->rot[0][0] = cos(a);
   xypars->rot[0][1] = 0;
   xypars->rot[0][2] = -sin(a);
   xypars->rot[1][0] = -sin(a)*sin(b);
   xypars->rot[1][1] = cos(b);
   xypars->rot[1][2] = -cos(a)*sin(b);
   xypars->rot[2][0] = sin(a)*cos(b);
   xypars->rot[2][1] = sin(b);
   xypars->rot[2][2] = cos(a)*cos(b);


   xypars->cenLambda = cenWave;
   xypars->obsLambda = obsWave;
  
   xypars->dist = dist;

}
/*+				T d f R d 2 t a n
 *  Function name:
      TdfRd2tan

 *  Function:
      Determine 2df tanget plane coorindates from apparent RA and Dec

 *  Description:
      Determine coordinates in tangent plane, from the
      apparent RA and Dec of a source and the field centre.
      
 *  Language:
      C

 *  Declaration:
       TdfRd2tan(TdfXyType *xypars, double cra, double cdec, double ra, 
                double dec, double mjd, double *xi, double *eta, 
                StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) xypars    (TdfXyType)  2dF xy Transformation parameters - set up
                             by a call to TdfXyInit.
      (>) cra       (double) Apparent RA of field centre.
      (>) cdec      (double) Apparent Dec of field centre.
      (>) ra        (double) Apparent RA of source.
      (>) dec       (double) Apparent Dec of source.
      (>) mjd       (double) UTC date and time expressed as Modified
                              Julian Date.
      (<) xi      (double *) Tangent plane coordinates (radians)
      (<) eta     (double *) 
      (<) cmha    (double *) -ha,dec of field centre on AAT mount system
      (<) cmdec   (double *) dec of field centre on AAT mount system
      (!) status    (StatusType*) Modified status. 
                         TDFXY__ILLRA ->  Illegal RA.
                         TDFXY__ILLDEC ->  Illegal Dec.
                         TDFXY__ZDERR -> ZD greater than 70 degrees.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 16-Nov-93
 *-
 */
void TdfRd2tan(
    TdfXyType *xypars, double cra, double cdec, double ra, 
    double dec, double mjd, double *xi, double *eta, 
    double *cmha,
    double *cmdec,
    StatusType *status)

{
   double mha,mdec;           /* -ha,dec of source on AAT mount system  */
   int jstat;


   if (*status != STATUS__OK) return;

   if ((cra > D2PI) || (cra < 0.0))
     {
       *status = TDFXY__ILLRA;
       ErsRep(0,status,"Illegal Centre RA - %f radians",cra);
     }
   if ((cdec > DPI) || (cdec < (-DPI)))
     {
       *status = TDFXY__ILLDEC;
       ErsRep(0,status,"Illegal Centre Dec - %f radians",cdec);
     }
   if ((ra > D2PI) || (ra < 0.0))
     {
       *status = TDFXY__ILLRA;
       ErsRep(0,status,"Illegal RA - %f radians",ra);
     }
   if ((dec > DPI) || (dec < (-DPI)))
     {
       *status = TDFXY__ILLDEC;
       ErsRep(0,status,"Illegal Dec - %f radians",dec);
     }

   if (*status != STATUS__OK)
   {
       ReportInvalidFieldDetails(cra, cdec, ra, dec, *status);
       return;
   }


/*  Update Apparent to observed parameters  */

   slaAoppat(mjd,xypars->cenAoprms);
   slaAoppat(mjd,xypars->obsAoprms);

/*  Check we can observe this position */

   CheckMjd(cra, cdec, mjd, *xypars, status);
   if (*status != STATUS__OK) return;

/*  Mount frame position of field centre  */

   Tdf___App2mount(1, xypars,cra,cdec,cmha,cmdec,status);

/*  Mount frame position of source  */

   Tdf___App2mount(0, xypars,ra,dec,&mha,&mdec,status);
   if (*status != STATUS__OK) return;

/*  Project to tangent plane  */

   slaDs2tp(mha,mdec,*cmha,*cmdec,xi,eta,&jstat);


   switch (jstat)
   {
   case 0:
       break; /* No error - continue */
   case 1:
       *status = TDFXY__SLA_ERR_1;
       return;
   case 2:
       *status = TDFXY__SLA_ERR_2;
       return;
   case 3:
       *status = TDFXY__SLA_ERR_3;
       return;
   default:
          *status = TDFXY__SLA_ERR_UNKN;
          return;
   }


}


/*+				T d f R d 2 x y

 *  Function name:
      TdfRd2xy

 *  Function:
      Determine 2df x,y from apparent RA and Dec

 *  Description:
      Determine the x,y coordinates in the field plate, from the
      apparent RA and Dec of a source and the field centre.
      
 *  Language:
      C

 *  Declaration:
       TdfRd2xy(TdfXyType *xypars, double cra, double cdec, double ra, 
                double dec, double mjd, double *x, double *y, 
                StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) xypars    (TdfXyType)  2dF xy Transformation parameters - set up
                             by a call to TdfXyInit.
      (>) cra       (double) Apparent RA of field centre.
      (>) cdec      (double) Apparent Dec of field centre.
      (>) ra        (double) Apparent RA of source.
      (>) dec       (double) Apparent Dec of source.
      (>) mjd       (double) UTC date and time expressed as Modified
                              Julian Date.
      (<) x         (double*) x position of source on field plate (microns).
      (<) y         (double*) y position of source on field plate (microns).
      (!) status    (StatusType*) Modified status. 
                         TDFXY__ILLRA ->  Illegal RA.
                         TDFXY__ILLDEC ->  Illegal Dec.
                         TDFXY__ZDERR -> ZD greater than 70 degrees.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 16-Nov-93
 *-
 */
void TdfRd2xy(TdfXyType *xypars, double cra, double cdec, double ra, 
                double dec, double mjd, double *x, double *y, 
                StatusType *status)

{
   double xi,eta;             /* tangent plane coordinates  */
   double cmha,cmdec;         /* -ha,dec of field centre on AAT mount system */


   if (*status != STATUS__OK) return;
   TdfRd2tan(xypars, cra, cdec, ra, dec, mjd, &xi, &eta, &cmha, &cmdec, 
             status);
   TdfDistXy(xypars->dist,xypars->obsLambda,0,0,-cmha,cmdec,xi,eta,x,y,status);

}



/*
 *   TdfDistXy
 *
 *   This function converts the standard coordinates (xi, eta
 *    in radians) to x,y in microns correcting for distortion.
 *
 *     (>)  dist  (TdfDistType)    Structure specifying parameters of
 *                                   distortion model
 *     (>)  lambda (double)        Wavelength of observation 
 *     (>)  adc_a (double)         Angle of ADC element A
 *     (>)  adc_b (double)         Angle of ADC element B
 *     (>)  mha (double)           Hour angle of telescope mount (radians)
 *     (>)  mdec (double)          Declination of telescope mount (radians)
 *     (>)  xi (double)            Tangent Plane coordinate (radians)
 *     (>)  eta (double)           Tangent Plane coordinate (radians)
 *     (<)  x  (double *)          Rectangular coordinates 
 *     (<)  y  (double *)             on field plate (microns)
 *     (!)  status (StatusType*)   Inherited status
 *
 */


void TdfDistXy(TdfDistType dist, double lambda, 
               double adc_a DUNUSED , double adc_b DUNUSED,
               double mha DUNUSED , double mdec DUNUSED ,
               double xi, double eta, double *x, double *y, 
               StatusType *status DUNUSED)

{
    double x1,y1,r2;
    double a,b,c,d;

/*  Get distortion model for observing wavelength  */

    Tdf___DistLambda(lambda,dist,&a,&b,&c,&d);    

/*  Correct for offset of distortion pattern centre  */

    x1 = (xi - dist.x0/a);
    y1 = (eta - dist.y0/a);
    r2 = x1*x1+y1*y1;

/*  Correct for radial distortion  */

    *x = xi*(a + r2*(b + r2*(c + d * r2)));
    *y = eta*(a + r2*(b + r2*(c + d * r2)));
}

/*
 *   TdfDistXyInv
 *
 *   This function converts to standard coordinates (xi, eta
 *    in radians) from x,y in microns correcting for distortion.
 *   It is the exact inverse of TdfDistXy.
 *
 *     (>)  dist  (TdfDistType)    Structure specifying parameters of
 *                                   distortion model
 *     (>)  lambda (double)        Wavelength of observation (microns)
 *     (>)  adc_a (double)         Angle of ADC element A
 *     (>)  adc_b (double)         Angle of ADC element B
 *     (>)  mha (double)           Hour angle of telescope mount (radians)
 *     (>)  mdec (double)          Declination of telescope mount (radians)
 *     (>)  x  (double)            Rectangular coordinates 
 *     (>)  y  (double)              on field plate (microns)
 *     (<)  xi (double*)           Tangent Plane coordinate (radians)
 *     (<)  eta (double*)          Tangent Plane coordinate (radians)
 *     (!)  status (StatusType*)   Inherited status
 *
 */

void TdfDistXyInv(TdfDistType dist, double lambda, 
                  double adc_a DUNUSED, double adc_b DUNUSED,
                  double mha   DUNUSED, double mdec  DUNUSED,
                  double x, double y, double *xi, double *eta, 
                  StatusType *status DUNUSED)

{

    double x1,y1,r2;
    double a,b,c,d;
    int i;

/*  Get distortion model for observing wavelength  */

    Tdf___DistLambda(lambda,dist,&a,&b,&c,&d); 

/*  Because the distortion model is expressed in terms of a radial
    distance in the form of xi and eta, and we have x and y, the calculation
    has to be done iteratively. We make an initial estimate of xi and eta
    ignoring distortion and improve it iteratively. Since the distortion
    corrections are relatively small the process normally converges 
    rapidly - Here we do 5 iterations which should be sufficient  */
   
/*  Initial approximation  */

    *xi = x/a;
    *eta = y/a;

/*  Recalculate iteratively to get improved estimates of xi and eta  */

    for (i=0;i<5;i++)
    {
      x1 = (*xi - dist.x0/a);
      y1 = (*eta - dist.y0/a);
      r2 = x1*x1+y1*y1;
      *xi = x/(a + r2*(b + r2*(c + d*r2)));
      *eta = y/(a + r2*(b + r2*(c + d*r2)));
    }
}


void Tdf___DistLambda(double lambda, TdfDistType dist, double *a,
       double *b, double *c, double *d)
/*
 *      Tdf___DistLambda
 *
 *     Return the distortion parameters for a given wavelength
 *
 */

{
    double l1;

    l1 = 1.0/lambda;
    *a = dist.a + l1*(-1.93829e4 + l1*(6.93693e3 - l1*1.42237e3));
    *b = dist.b + l1*(7.121023e7 + l1*(-3.61615e7 + l1*7.8599e6));
    *c = dist.c + l1*(-1.05989e11 + l1*(0.80613e11 - l1*0.1579e11));
    *d = dist.d;
    

}



/*
 *     Tdf___App2mount
 *
 *     Convert apparent ra,dec to -ha,dec on the AAT mount system -
 *     i.e. the AAT polar axis is the pole of the coordinate system.
 *
 */


void Tdf___App2mount(int cen, TdfXyType *xypars, double ra, double dec,
                    double *mha, double *mdec, StatusType *status)



{
   double a;                 /* Observed azimuth  */
   double zd;                /* Observed zenith distance  */
   double h;                 /* Observed hour angle  */
   double d;                 /* Observed declination  */
   double r;                 /* Observed right ascension  */
   double e;                 /* Observed elevation */
   double obs[3];            /* Observed Az,El - cartesian vector  */
   double mount[3];          /* Mount frame position - cartesian vector */


   if (*status != STATUS__OK) return;

/*  Apparent to Observed  */

   slaAopqk(ra,dec,
            (cen ? xypars->cenAoprms : xypars->obsAoprms),
            &a,&zd,&h,&d,&r);

/*  Check ZD  (should no longer be triggered, picked up by CheckMjd() call */
   
   if (zd > 70*DD2R) 
     {
        *status = TDFXY__ZDERR;
        ErsRep(0,status,"ZD greater than 70 degrees");
        return;
     }

/*  Convert to Az,El on Wallace convention (Keck Report section 5.1)  */

   a = DPI - a;
   e = DPIBY2 - zd;

/*  Convert to cartesian  */

   slaDcs2c(a,e,obs);

/*  Rotate to AAT -HA,Dec  */

   slaDmxv(xypars->rot,obs,mount);
  

/*  Convert to spherical  */

   slaDcc2s(mount,mha,mdec);
}





/*+				T d f X y 2 r d

 *  Function name:
      TdfXy2rd

 *  Function:
      Determine apparent RA and Dec from 2dF x and y.

 *  Description:
      Determine the apparent RA and Dec of a source from the
      x,y coordinates in the field plate and the field centre.
      
 *  Language:
      C

 *  Declaration:
       TdfXy2rd(TdfXyType *xypars, double cra, double cdec, double x,
                double y, double mjd, double *ra, double *dec,
                StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) xypars    (TdfXyType)  2dF xy Transformation parameters - set up by
                          a call to TdfXyInit.
      (>) cra       (double) Apparent RA of field centre.
      (>) cdec      (double) Apparent Dec of field centre.
      (>) x         (double) x position of source on field plate (microns).
      (>) y         (double) y position of source on field plate (microns).
      (>) mjd       (double) UTC date and time expressed as Modified
                              Julian Date.
      (<) ra        (double*) Apparent RA of source.
      (<) dec       (double*) Apparent Dec of source.
      (!) status    (StatusType*) Modified status. 


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 17-Nov-93
 *-
 */

void TdfXy2rd(TdfXyType *xypars, double cra, double cdec, double x,
                double y, double mjd, double *ra, double *dec,
                StatusType *status)

{
   double cmha,cmdec;         /* -ha,dec of field centre on AAT mount system */
   double mha,mdec;           /* -ha,dec of source on AAT mount system  */
   double xi,eta;             /* tangent plane coordinates  */

   double obs[3];             /* Observed Az, El cartesian vector  */
   double mount[3];           /* Mount -HA, Dec cartesian vector  */
   double a,e;                /* Observed Az, El */
   double zd;                 /* Observed Zd */

   if (*status != STATUS__OK) return;

/*  Update Apparent to observed parameters  */

   slaAoppat(mjd,xypars->cenAoprms);
   slaAoppat(mjd,xypars->obsAoprms);

/*  Check we can observe this position */

   CheckMjd(cra, cdec, mjd, *xypars, status);


/*  Mount frame position of field centre  */

   Tdf___App2mount(1, xypars,cra,cdec,&cmha,&cmdec,status);

   if (*status != STATUS__OK) return;


/*  Correct for distortion  */

   TdfDistXyInv(xypars->dist,xypars->obsLambda,0,0,
           -cmha,cmdec,x,y,&xi,&eta,status);


/*  Deproject from tangent plane to give mount -ha, dec  */

   slaDtp2s(xi,eta,cmha,cmdec,&mha,&mdec);

/*  Convert to cartesian  */

   slaDcs2c(mha,mdec,mount);

/*  Rotate to observed Az, El  (AZ south through east) */

   slaDimxv(xypars->rot,mount,obs);
  
/*  Convert to spherical  */

   slaDcc2s(obs,&a,&e);

/*  Change to standard Az, Zd convention  */

   a = DPI - a;
   zd = DPIBY2 - e;

/*  Observed to Apparent  */

   slaOapqk("A",a,zd,xypars->obsAoprms,ra,dec);

}

/*+		    T d f _ _ _ A p p l y D i s t M a p

 *  Function name:
      Tdf___ApplyDistMap

 *  Function:
      Apply the distortion map to an x/y pair.

 *  Description:
      The distoration map file gives us a mapping between field plate
      and sky - to be applied after our other corrections (by TdfXy2pos()
      and TdfPos2xy()).  See Tdf___LoadDistMap() for more details on this.

      This function applies the map to a given x/y position, if the 
      map is enabled.

 *  Language:
      C

 *  Declaration:
       Tdf___ApplyDistMap(const TdfDistMapType *map, int positive,
                          double *x, double *y);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) map      (const TdfDistMapType*)  Distortion map.
      (>) positive (int) If ture, apply in the positive direction, else 
                          apply in the negative direction.
      (!) x        (double *)   
      (!) Y        (double *)   
      (!) status   (StatusType *)  Modified status.


 *  Support: Tony Farrell, {AAO}

 *  Version date: 27-Sep-2007
 *-
 */
/*
 * Here we use a map devised by Russell Cannon to do the correction.
 */
#define STEP (318.0*66.66667)  /* Step between grid elements - from Russels program. */
#define CENTRE_OFFSET  STEP*((double)(TDFXY_GRID_WIDTH+1)/2.0) /* Offset to centre of grid */


/*
 * Return the index in the distortion map given a field plate x/y value.
 */
static unsigned Tdf___GetMapIndex(
    const double x,
    const double y)

{
   /*
     *  Work out our index into the map.
     */
    unsigned xIndex = (int)(round((x + CENTRE_OFFSET) / STEP));
    unsigned yIndex = (int)(round((y + CENTRE_OFFSET) / STEP));

    /*
     * Range validation checks.
     */
    if (xIndex < 1) xIndex = 1;
    if (xIndex > TDFXY_GRID_WIDTH) xIndex = TDFXY_GRID_WIDTH;
    if (yIndex < 1) yIndex = 1;
    if (yIndex > TDFXY_GRID_WIDTH) yIndex = TDFXY_GRID_WIDTH;

    /*
     * The indicies are 1 based, make them 0 based.
     */
    --xIndex;
    --yIndex;
    /*
     * And then this is the actual index into the map
     */
    return yIndex*TDFXY_GRID_WIDTH+xIndex;

}

/* Marked internal so that tdfxytest can call it, not meant for client user */
extern void Tdf___ApplyDistMap(
    const TdfDistMapType * const map,
    const int positive,
    double *const x,
    double *const y)
{
    unsigned mapIndex = 0;
    double dx;
    double dy;
    /*
    * Has the map been loaded?
    */
    if (!map->loaded) return;

    /*
     * GetMapIndex returns an index in the distortion map for this
     * x and y position.
     */
    mapIndex = Tdf___GetMapIndex(*x, *y);
    dx = map->x[mapIndex];
    dy = map->y[mapIndex];
    if (positive)
    {
        *x += dx;
        *y += dy;
    }
    else
    {
        *x -= dx;
        *y -= dy;
       
    }


}


/*+				T d f X y 2 p o s

 *  Function name:
      TdfXy2pos

 *  Function:
      Convert field plate coordinates to positioner coordinates

 *  Description:
      Convert the x,y coordinates of a star on the field plate - as output
      by TdfRd2xy to the coordinates required for input to the 2df positioner
      DRAMA task (TDFPT).

      
 *  Language:
      C

 *  Declaration:
       TdfXy2pos(TdfLinType *linpars, double x, double y, double *xp, double *yp);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) linpars    (TdfLinType)  Linear transformation parameters for the
                          gantry to be used.
      (>) x         (double) Field plate x coordinate of star.
      (>) y         (double) Field plate y coordinate of star.
      (<) xp        (double*) Positioner X coordinate (microns).
      (<) yp        (double*) Positioner Y coordinate (microns).



 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-May-93
 *-
 */


void TdfXy2pos(TdfLinType *linpars, double x, double y, double *xp, double *yp)   
   
{
    StatusType ignore = STATUS__OK;
    double a,b,c,d,e,f;
   
    a = linpars->coeffs[0];
    b = linpars->coeffs[1];
    c = linpars->coeffs[2];
    d = linpars->coeffs[3];
    e = linpars->coeffs[4];
    f = linpars->coeffs[5];

    if (linpars->extraScale == 0)
    {
        fprintf(stderr,"TdfXy2pos:Illegal extra scale of zero ignore\n");
        linpars->extraScale = 1;
    }
    TdfNormalizeLinear(linpars->extraRotation,
                       linpars->extraScale,
                       linpars->extraNonPerp,
                       &a, &b, &c, &d, &e, &f,
                       &ignore); /*Only bad status should be scale of 0 */

   

   
    *yp = a + b*x + c*y;
    *xp = d + e*x + f*y;
    
    /*
     * Apply the sky to fpi distortion map.
     */
    Tdf___ApplyDistMap(&linpars->distMap, 1, xp, yp);
}

/*+				T d f P o s 2 x y

 *  Function name:
      TdfPos2xy

 *  Function:
      Convert positioner coordinates to field plate coordinates

 *  Description:
      Convert the x,y coordinates of a star on the system used by the 2df
      positioner task to the x,y coordinates on the field plate as used by
      TdfXy2rd.

      
 *  Language:
      C

 *  Declaration:
       TdfPos2xy(TdfLinType *linpars, double xp, double yp, double *x, double *y);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) linpars    (TdfLinType)  Linear transformation parameters for the
                          gantry to be used.
      (>) xp        (double) positioner X coordinate of star.
      (>) yp        (double) positioner Y coordinate of star.
      (<) x         (double*) field plate x coordinate (microns).
      (<) y         (double*) field plate y coordinate (microns).



 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-May-93
 *-
 */


void TdfPos2xy(TdfLinType *linpars, double xp, double yp, double *x, double *y)   
   
{
    StatusType ignore = STATUS__OK;
    double a,b,c,d,e,f;
    double a1,b1,c1,d1,e1,f1;

    /*
     * Apply the sky to fpi distortion map.
     */
    Tdf___ApplyDistMap(&linpars->distMap, 0, &xp, &yp);

   
    a = linpars->coeffs[0];
    b = linpars->coeffs[1];
    c = linpars->coeffs[2];
    d = linpars->coeffs[3];
    e = linpars->coeffs[4];
    f = linpars->coeffs[5];

    if (linpars->extraScale == 0)
    {
        fprintf(stderr,"TdfPos2xy:Illegal extra scale of zero ignore\n");
        linpars->extraScale = 1;
    }
    TdfNormalizeLinear(-1.0*linpars->extraRotation,
                       1/linpars->extraScale,
                       -1.0*linpars->extraNonPerp,
                       &a, &b, &c, &d, &e, &f,
                       &ignore); /*Only bad status should be scale of 0 */

    a = -a;
    b = -b;
    c = -c;
   
    a1 = (a*f - d*c)/(e*c - b*f);
    b1 = c/(e*c - b*f);
    c1 = f/(e*c - b*f);
    d1 = (a*e - d*b)/(b*f - e*c);
    e1 = b/(f*b - c*e);
    f1 = e/(f*b - c*e);
   
    *x = a1 + b1*xp + c1*yp;
    *y = d1 + e1*xp + f1*yp;
}


/*+				T d f G e t D i s t

 *  Function name:
      TdfGetDist

 *  Function:
      Get Distortion Parameters

 *  Description:
      Get the 2dF distortion parameters from an SDS file.

      
 *  Language:
      C

 *  Declaration:
       TdfGetDist(char *file, TdfDistType *dist, StatusType *status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) file    (char *)   Name of sds file to read parameters from.
      (<) dist    (TdfDistType*)  Distortion parameters.
      (!) status  (StatusType *)  Modified status.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-May-93
 *-
 */

void TdfGetDist(char *file, TdfDistType *dist, StatusType *status)

{
   SdsIdType id,id2;
   unsigned long actlen;
   
   SdsRead(file,&id,status);
   
   SdsFind(id,"a",&id2,status);
   SdsGet(id2,sizeof(double),0,&(dist->a),&actlen,status);
   SdsFreeId(id2,status);
   SdsFind(id,"b",&id2,status);
   SdsGet(id2,sizeof(double),0,&(dist->b),&actlen,status);
   SdsFreeId(id2,status);
   SdsFind(id,"c",&id2,status);
   SdsGet(id2,sizeof(double),0,&(dist->c),&actlen,status);
   SdsFreeId(id2,status);
   SdsFind(id,"d",&id2,status);
   SdsGet(id2,sizeof(double),0,&(dist->d),&actlen,status);
   SdsFreeId(id2,status);
   SdsFind(id,"x0",&id2,status);
   SdsGet(id2,sizeof(double),0,&(dist->x0),&actlen,status);
   SdsFreeId(id2,status);
   SdsFind(id,"y0",&id2,status);
   SdsGet(id2,sizeof(double),0,&(dist->y0),&actlen,status);
   SdsFreeId(id2,status);
   SdsReadFree(id, status);
   SdsFreeId(id,status);
}


/*+				T d f G e t D i s t 2

 *  Function name:
      TdfGetDist2

 *  Function:
      Get average Distortion Parameters from two files.

 *  Description:
      Get the 2dF distortion parameters from two SDS files and
      average them. This is used for the 2dF 'plate 2' which is
      a virtual plate that is supposed to represent an average of
      the two other plates, and so plate 2 configurations can be
      used for either plate (although they may need looser tolerances,
      and can only use the set of fibres available for both plates -
      a broken fibre on either plate shows up as broken on plate 2).
      
 *  Language:
      C

 *  Declaration:
       TdfGetDist2(char *file0, char *file1, 
                               TdfDistType *dist, StatusType *status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) file0   (char *)   Name of first sds file to read parameters from.
      (>) file1   (char *)   Name of second sds file to read parameters from.
      (<) dist    (TdfDistType*)  Distortion parameters.
      (!) status  (StatusType *)  Modified status.


 *  Support: Keith Shortridge, {AAO}

 *  Version date: 22-Mar-2007
 *-
 */

void TdfGetDist2(char *file0, char* file1, 
                          TdfDistType *dist, StatusType *status)

{
   SdsIdType fid0,id0,fid1,id1;
   double val0,val1;
   unsigned long actlen;
   
   SdsRead(file0,&fid0,status);
   SdsRead(file1,&fid1,status);
   
   SdsFind(fid0,"a",&id0,status);
   SdsGet(id0,sizeof(double),0,&val0,&actlen,status);
   SdsFreeId(id0,status);
   SdsFind(fid1,"a",&id1,status);
   SdsGet(id1,sizeof(double),0,&val1,&actlen,status);
   SdsFreeId(id1,status);
   dist->a = (val0 + val1) * 0.5;
   
   SdsFind(fid0,"b",&id0,status);
   SdsGet(id0,sizeof(double),0,&val0,&actlen,status);
   SdsFreeId(id0,status);
   SdsFind(fid1,"b",&id1,status);
   SdsGet(id1,sizeof(double),0,&val1,&actlen,status);
   SdsFreeId(id1,status);
   dist->b = (val0 + val1) * 0.5;
   
   SdsFind(fid0,"c",&id0,status);
   SdsGet(id0,sizeof(double),0,&val0,&actlen,status);
   SdsFreeId(id0,status);
   SdsFind(fid1,"c",&id1,status);
   SdsGet(id1,sizeof(double),0,&val1,&actlen,status);
   SdsFreeId(id1,status);
   dist->c = (val0 + val1) * 0.5;
   
   SdsFind(fid0,"d",&id0,status);
   SdsGet(id0,sizeof(double),0,&val0,&actlen,status);
   SdsFreeId(id0,status);
   SdsFind(fid1,"d",&id1,status);
   SdsGet(id1,sizeof(double),0,&val1,&actlen,status);
   SdsFreeId(id1,status);
   dist->d = (val0 + val1) * 0.5;
   
   SdsFind(fid0,"x0",&id0,status);
   SdsGet(id0,sizeof(double),0,&val0,&actlen,status);
   SdsFreeId(id0,status);
   SdsFind(fid1,"x0",&id1,status);
   SdsGet(id1,sizeof(double),0,&val1,&actlen,status);
   SdsFreeId(id1,status);
   dist->x0 = (val0 + val1) * 0.5;
   
   SdsFind(fid0,"y0",&id0,status);
   SdsGet(id0,sizeof(double),0,&val0,&actlen,status);
   SdsFreeId(id0,status);
   SdsFind(fid1,"y0",&id1,status);
   SdsGet(id1,sizeof(double),0,&val1,&actlen,status);
   SdsFreeId(id1,status);
   dist->y0 = (val0 + val1) * 0.5;
   
   SdsReadFree(fid1, status);
   SdsFreeId(fid1,status);
   SdsReadFree(fid0, status);
   SdsFreeId(fid0,status);
}

/*+				T d f _ _ _ L o a d D i s t M a p

 *  Function name:
      Tdf___LoadDistMap

 *  Function:
      Load the distortion map.

 *  Description:
      The distortion map file gives us a mapping between field plate
      and sky - to be applied after our other corrections (by TdfXy2pos()
      and TdfPos2xy()).

      If the environment variable TDF_DIST_MAP_FILE is defined, then
      that gives us the name of the file to use, otherwise, we pass 
      in the name of the linear model SDS file and look for
      this map file (2dF_distortion.map) in the same  directory.

      The environment variable TDF_DIST_MAP_SIGN_NEGATE is set, then the
      values from the map are negated.

      The file format - as specified by Russell Cannon in an e-mail on
      26-Sep-2007 is:
          Columns 1 and 2 are the xy grid coordinates
          columns 7 and 8 give the new combined corrections.

          Columns 3 and 4 are the old (July 07) corrections for the 
              Gripper-FPI gantries.  (Used by TDFGRIP)

          Columns 5 and 6 are from the new FPI-sky map (what we are after)

          Columns 7 and 8 are simply the sums of the previous two

          Columns 9 and 10 are an indication of the number of grid points 
             used in smoothing and interpolating the grids.

 *  Language:
      C

 *  Declaration:
       Tdf___LoadDistMap(char *file, TdfDistMapType *map, StatusType *status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) file    (char *)   Name of sds file from which the linear model
                    file was read.  We will read the distortion file from
                    the same directory.
      (<) map (TdfDistMapType*)  Distortion map.
      (!) status  (StatusType *)  Modified status.


 *  Support: Tony Farrell, {AAO}

 *  Version date: 27-Sep-2007
 *-
 */
#define MAP_FILE "2dF_distortion.map"
#define MAP_NAME_ENV "TDF_DIST_MAP_FILE"
#define MAP_FLAG_ENV "TDF_DIST_MAP_SIGN_NEGATE"

static void Tdf___LoadDistFile(
    const char * const linFileName, 
    TdfDistMapType * const map,
    StatusType * const status)
{
    char fileName[PATH_MAX];
    int negate = 0;
    unsigned lineNum = 0;
    FILE *mapFile = 0;
    unsigned numEntries = 0;
    if (*status != STATUS__OK) return;

    map->loaded = 0;
    memset(&map->x, 0, sizeof(map->x));
    memset(&map->y, 0, sizeof(map->y));
#if 0
    fprintf(stderr,"Tdf___LoadDistFile:SDS file is %s\n",
            linFileName);
#endif
    /*
     * Am Environment variable can override the file name.
     */
    if (getenv(MAP_NAME_ENV) != 0)
    {
        strncpy(fileName, getenv(MAP_NAME_ENV), sizeof(fileName));
        fileName[sizeof(fileName)-1] = 0;
    }
    else
    {
        /*
         * Otherwise- work out the file name - should be in the same
         * directory as the tdFlienar<0|1>.sds file.
         */ 
        unsigned i;
        strncpy(fileName, linFileName, sizeof(fileName));
        fileName[sizeof(fileName)-1] = 0;
        /*
         * want to find the begining of the file name component.
         */
        i = strlen(fileName) - 1;
        while ((i > 0)&&(fileName[i] != '/'))
            --i;
        
        if (fileName[i] == '/') 
        {
            /*
             * Replace tdFlinear<0|1>.sds with MAP_FILE value.
             */
            fileName[i+1] = 0;
            strncat(fileName, MAP_FILE, 
                    sizeof(fileName)-strlen(fileName)-1);
            fileName[sizeof(fileName)-1] = 0;                    
        } 
        else 
        {
            strcpy(fileName, MAP_FILE);
        }
    }
    /*
     * If this environment variable is set, negate the sign of
     * the map.
     */
    if (getenv(MAP_FLAG_ENV))
        negate = 1;
    
    /*
     *
     */
    mapFile = fopen(fileName, "r");
    if (!mapFile)
    {
        if (errno == ENOENT)
            fprintf(stderr,"Failed to read distortion map file %s - does not exist - ignored)\n",
                    fileName);
        else
        {
            *status = TDFXY__MAP_FILE_OPEN;
            ErsRep(0,status, "File %s open failed with status %d - %s",
                   fileName, errno, strerror(errno));
        }
        return;
    }
    {
        char line[200];
        while (fgets(line, sizeof(line), mapFile))
        {
            double xposition;
            double yposition;
            double ignore1;
            double ignore2;
            double xadjust;
            double yadjust;
            unsigned mapIndex;


            ++lineNum;
            /* Ignore comment lines */
            if (line[0] == '#') 
            {
                continue;
            }
            /*
             * It is the values in columns 5 and 6 that we want, but we
             * use the first two items to work out our map index.
             */
            if (sscanf(line,"%lg %lg %lg %lg %lg %lg\n",
                       &xposition, &yposition, &ignore1, &ignore2,
                       &xadjust, &yadjust) != 6)
            {
                *status = TDFXY__MAP_FILE_READ;
                ErsRep(0, status, "Error read distortion map line %d", lineNum);
                goto EXIT;
            }
            mapIndex = Tdf___GetMapIndex(xposition, yposition);

            if (negate)
            {
                xadjust *= -1;
                yadjust *= -1;
            }
            map->x[mapIndex] = xadjust;
            map->y[mapIndex] = yadjust;
            ++numEntries;

    
        }
    }

    if (numEntries != (TDFXY_GRID_WIDTH*TDFXY_GRID_WIDTH))
    {
        *status = TDFXY__MAP_FILE_OPEN;
        ErsRep(0,status, "Distortion map file %s has %d entries rather then the expected %d.",
               fileName, numEntries,(TDFXY_GRID_WIDTH*TDFXY_GRID_WIDTH) );
        goto EXIT;
    }

    fprintf(stderr,"Tdf___LoadDistFile:Loaded map from file %s.\n",
            fileName);


    map->loaded = 1;
EXIT:
    fclose(mapFile);
            
}

extern void Tdf___DumpDistMap(
    FILE *file,
    const TdfDistMapType * const map,
    StatusType * const status)
{
    unsigned xIndex = 0;
    if (*status != STATUS__OK) return;
    if (map->loaded == 0)
    {
        fprintf(file,"No distortion map loaded!\n");
        return;
    }
    fprintf(file,"Distortion map contents:\n");
    fprintf(file,"Grid X  Grid Y    ( Pos X   Pos Y ) Adjust X  Adjust Y\n");
    for (xIndex = 0; xIndex < TDFXY_GRID_WIDTH ; ++xIndex)
    {
        unsigned yIndex;
        for (yIndex = 0 ; yIndex < TDFXY_GRID_WIDTH ; ++yIndex)
        {

            double xpos;
            double ypos;

            /* 
             * Given the grid index, return the xpos/ypos value.
             */ 
            xpos = ((double)(xIndex + 1) * STEP) - CENTRE_OFFSET;
            ypos = ((double)(yIndex + 1) * STEP) - CENTRE_OFFSET;

            fprintf(file, "   %3u     %3u    (%7d %7d)  %6.1f    %6.1f\n",
                    xIndex, yIndex, 
                    (int)(round(xpos)), 
                    (int)(round(ypos)),
                    map->x[yIndex*TDFXY_GRID_WIDTH+xIndex],
                    map->y[yIndex*TDFXY_GRID_WIDTH+xIndex]);

        }
    }
    
}

/*+				T d f G e t L i n

 *  Function name:
      TdfGetLin

 *  Function:
      Get Linear Model Parameters

 *  Description:
      Get the linear model parameters from an SDS file.

      
 *  Language:
      C

 *  Declaration:
       TdfGetLin(char *file, TdfLinType *linpars, StatusType *status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) file    (char *)   Name of sds file to read parameters from.
      (<) linpars (TdfLinType*)  Linear model parameters.
      (!) status  (StatusType *)  Modified status.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-May-93
 *-
 */

void TdfGetLin(char *file, TdfLinType *linpars, StatusType *status)

{
   SdsIdType id,id2;
   unsigned long actlen;
   int i;
   
   if (*status != STATUS__OK) return;

   SdsRead(file,&id,status);
   
   SdsFind(id,"coeffs",&id2,status);
   for (i=0;i<6;i++) {
      SdsGet(id2,sizeof(double),i,&(linpars->coeffs[i]),&actlen,status);
   }
   SdsFreeId(id2,status);
   SdsReadFree(id, status);
   SdsFreeId(id,status);

   /*
    * These get default values.
    */
   linpars->extraScale = 1;
   linpars->extraRotation = 0;
   linpars->extraNonPerp = 0;

   /*
    * Load the distortion map.
    */
   Tdf___LoadDistFile(file, &linpars->distMap, status);
}


/*+				T d f G e t L i n 2

 *  Function name:
      TdfGetLin2

 *  Function:
      Get Linear Model Parameters from two files

 *  Description:
      Get the linear model parameters from two SDS files and
      average them. This is used for the 2dF 'plate 2' which is
      a virtual plate that is supposed to represent an average of
      the two other plates, and so plate 2 configurations can be
      used for either plate (although they may need looser tolerances,
      and can only use the set of fibres available for both plates -
      a broken fibre on either plate shows up as broken on plate 2).


      
 *  Language:
      C

 *  Declaration:
       TdfGetLin2(char *file0, char* file1, TdfLinType *linpars, 
                                                   StatusType *status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) file0   (char *)   Name of first sds file to read parameters from.
      (>) file1   (char *)   Name of second sds file to read parameters from.
      (<) linpars (TdfLinType*)  Linear model parameters.
      (!) status  (StatusType *)  Modified status.


 *  Support: Keith Shortridge, {AAO}

 *  Version date: 1-Nov-2012
 *-
 *  Modified:
 *     22nd Mar 2007. First dated version - perhaps the original date?
 *      1st Nov 2012. An SdsReadFree() call was using an Id that had already
 *                    been freed. Reversed the order of the SdsReadFree() and
 *                    SdsFreeId() call for fid1. KS.
 */

void TdfGetLin2(char *file0, char* file1,
                           TdfLinType *linpars, StatusType *status)

{
   SdsIdType fid0,fid1,id0,id1;
   double val0,val1;
   unsigned long actlen;
   int i;
   
   SdsRead(file0,&fid0,status);
   SdsRead(file1,&fid1,status);
   
   SdsFind(fid0,"coeffs",&id0,status);
   SdsFind(fid1,"coeffs",&id1,status);
   for (i=0;i<6;i++) {
      SdsGet(id0,sizeof(double),i,&val0,&actlen,status);
      SdsGet(id1,sizeof(double),i,&val1,&actlen,status);
      linpars->coeffs[i] = (val0 + val1) * 0.5;
   }
   SdsFreeId(id1,status);
   SdsFreeId(id0,status);
   
   SdsReadFree(fid0, status);
   SdsReadFree(fid1, status);
   SdsFreeId(fid0,status);
   SdsFreeId(fid1,status);

   /*
    * These get default values.
    */
   linpars->extraScale = 1;
   linpars->extraRotation = 0;
   linpars->extraNonPerp = 0;

   /*
    * Load the distortion map.
    */
   Tdf___LoadDistFile(file0, &linpars->distMap, status);

}

void TdfSetDist(TdfDistType dist, char *file, StatusType *status)

{
    SdsIdType id,id2;
	          
	SdsNew(0,"distortion",0,NULL,SDS_STRUCT,0,NULL,&id,status);
	SdsNew(id,"a",0,NULL,SDS_DOUBLE,0,NULL,&id2,status);
	SdsPut(id2,sizeof(double),0,&(dist.a),status);
	SdsFreeId(id2,status);
	SdsNew(id,"b",0,NULL,SDS_DOUBLE,0,NULL,&id2,status);
	SdsPut(id2,sizeof(double),0,&(dist.b),status);
	SdsFreeId(id2,status);
	SdsNew(id,"c",0,NULL,SDS_DOUBLE,0,NULL,&id2,status);
	SdsPut(id2,sizeof(double),0,&(dist.c),status);
	SdsFreeId(id2,status);
	SdsNew(id,"d",0,NULL,SDS_DOUBLE,0,NULL,&id2,status);
	SdsPut(id2,sizeof(double),0,&(dist.d),status);
	SdsFreeId(id2,status);
	SdsNew(id,"x0",0,NULL,SDS_DOUBLE,0,NULL,&id2,status);
	SdsPut(id2,sizeof(double),0,&(dist.x0),status);
	SdsFreeId(id2,status);
	SdsNew(id,"y0",0,NULL,SDS_DOUBLE,0,NULL,&id2,status);
	SdsPut(id2,sizeof(double),0,&(dist.y0),status);
	SdsFreeId(id2,status);
    SdsWrite(id,file,status);
    SdsDelete(id,status);
    SdsFreeId(id,status);
}

void TdfSetLin(double coeffs[6], char *file, StatusType *status)

{
    SdsIdType id,id2;
    unsigned long int dims[7];
    int i;
	          
	SdsNew(0,"linear",0,NULL,SDS_STRUCT,0,NULL,&id,status);
	dims[0] = 6;
	SdsNew(id,"coeffs",0,NULL,SDS_DOUBLE,1,dims,&id2,status);
	for (i=0;i<6;i++)
	   SdsPut(id2,sizeof(double),i,&(coeffs[i]),status);

	SdsFreeId(id2,status);
    SdsWrite(id,file,status);
    SdsDelete(id,status);
    SdsFreeId(id,status);
}

/*+				T d f G e t M o d e l

 *  Function name:
      TdfGetModel

 *  Function:
      Get all model parameters.

 *  Description:
      Get the linear model parameters into one structure using an array of
      doubles.

 *  Language:
      C

 *  Declaration:
       TdfGetModel(dist, linpars, model, status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) dist    (TdfDistType) Distortion model parameters.
      (>) linpars (TdfLinType)  Linear model parameters.
      (<) model   (TdfModelPars *) The model parameters.
      (!) status  (StatusType *)  Modified status.


 *  Support: Tony Farrell, {AAO}

 *  Version date: 15-Jun-04
 *-
 */
void TdfGetModel(const TdfDistType dist,
                 const TdfLinType linpars, 
                 TdfModelPars *model,
                 StatusType *status)
{
    unsigned i,j;
    if (*status != STATUS__OK) return;

    /*
     * Ensure all values are zero.
     */
    for (i = 0 ; i < TDFXY_NUM_PARS; ++i)
        model->p[i] = 0;

    /*
     *  Copy the linear model
     */
    for (i = 0, j = TDFXY_PAR_FIRSTLIN ; i < 6; ++i, ++j)
    {
        model->p[j] = linpars.coeffs[i];
    }
    /*
     * Extra linear model affects.
     */
    model->p[TDFXY_PAR_SCALE]     = linpars.extraScale;
    model->p[TDFXY_PAR_ROT]       = linpars.extraRotation;
    model->p[TDFXY_PAR_NONPERP]   = linpars.extraNonPerp;
    /*
     * Copy the distortion model.
     */
    i = TDFXY_PAR_DIST_FIRST;
     model->p[i++] = dist.a;
     model->p[i++] = dist.b;
     model->p[i++] = dist.c;
     model->p[i++] = dist.d;
     model->p[i++] = dist.x0;
     model->p[i++] = dist.y0;
     /*
      *  Finished.
      */
}


/*+				T d f S e t M o d e l

 *  Function name:
      TdfSetModel

 *  Function:
      Set all model parameters.

 *  Description:
      Set the linear model parameters from structure using an array of
      doubles.
      
 *  Language:
      C

 *  Declaration:
       TdfSetModel(model, dist, linpars, status);

      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) model   (const TdfModelPars *) The model parameters.
      (<) dist    (TdfDistType *) Distortion model parameters.
      (<) linpars (TdfLinType *)  Linear model parameters.
      (!) status  (StatusType *)  Modified status.


 *  Support: Tony Farrell, {AAO}

 *  Version date: 15-Jun-04
 *-
 */
void TdfSetModel(const TdfModelPars *model,
                 TdfDistType *dist,
                 TdfLinType *linpars, 
                 StatusType *status)
{
    unsigned i,j;
    if (*status != STATUS__OK) return;

    /*
     *  Copy the linear model
     */
    for (i = 0, j = TDFXY_PAR_FIRSTLIN ; i < 6; ++i, ++j)
    {
        linpars->coeffs[i] = model->p[j];
    }
    /*
     * Extra linear model affects.
     */
   linpars->extraScale    =  model->p[TDFXY_PAR_SCALE];
   linpars->extraRotation =  model->p[TDFXY_PAR_ROT];
   linpars->extraNonPerp  =  model->p[TDFXY_PAR_NONPERP];

   /*
    * Presume we have not loaded the map.  This may not be the
    * right approach - it is unclear at the moment.  
    */
   linpars->distMap.loaded = 0;
    /*
     * Copy the distortion model.
     */
    i = TDFXY_PAR_DIST_FIRST;
    dist->a  = model->p[i++];
    dist->b  = model->p[i++];
    dist->c  = model->p[i++];
    dist->d  = model->p[i++];
    dist->x0 = model->p[i++];
    dist->y0 = model->p[i++];
     /*
      *  Finished.
      */ 
}

/*^L
 *+                 T d f N o r m a l i z e M  o d e l

 * Function name:
      TdfMdlNormalizeModel

 * Function:
      Normalize our model.

 * Description:
      Apply rotation effects to the linear model parameters and scale
      effects the distortion parameters.

      Note that it would be nicer to put the scale into the distortion,
      but I (TJF) don't know how to do that, so am just putting it into
      the linear model at this point.

 * Language:
      C

 * Call:
        (void) = TdfNormalizeModel(dist, linpars,  status)

 * Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (!) dist      (TdfDistType) 2dF Distortion model parameters.
                            This is for future expansion and is not
                            currenlty uses.
      (!) linpars    (TdfLinType)  Linear transformation parameters for the
                          gantry to be used.
      (!) status  (StatusType *) modified status.


 * Include files: tdfxy.h
 
 *-

 * External functions used:

 * Support: Tony Farrell, AAO

 * History:
     16-Oct-2004 - TJF - Original version
 */
extern void TdfNormalizeModel  (
    TdfDistType *distPars DUNUSED,
    TdfLinType  *linPars,
    StatusType *status)
{
    if (*status != STATUS__OK) return;
  
    /*
     * Apply linear effects.
     */
    TdfNormalizeLinear(linPars->extraRotation,
                       linPars->extraScale,
                       linPars->extraNonPerp,
                       &linPars->coeffs[0],
                       &linPars->coeffs[1],
                       &linPars->coeffs[2],
                       &linPars->coeffs[3],
                       &linPars->coeffs[4],
                       &linPars->coeffs[5],
                       status);

    /*
     * If sucessfull - put these back to their default values.
     */
    if (*status == STATUS__OK)
    {
        linPars->extraRotation = 0;
        linPars->extraScale    = 1;
        linPars->extraNonPerp  = 0;
    }
}


/*^L
 *+                 T d f N o r m a l i z e L i n e a r

 * Function name:
      TdfNormalizeLinear.

 * Function:
      Normalize a linear model.

 * Description:
      Apply rotation and scale effects to linear model parameters.

 * Language:
      C

 * Call:
        (void) = TdfNormalizeLinear(rotation, scale, nonperp,
                                         a, b, c, d, e, f
                                         status)

 * Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) rotation (double) A rotation in degrees.
      (>) scale    (double) A scale
      (>) nonperp  (double)  Non-perpendicularity in degrees
      (!) a        (double) Linear model parameter a
      (!) b        (double) Linear model parameter b
      (!) c        (double) Linear model parameter c
      (!) d        (double) Linear model parameter d
      (!) e        (double) Linear model parameter e
      (!) f        (double) Linear model parameter f
      (!) status  (StatusType *) modified status.


 * Include files: fpcolMdl.h
 
 *-

 * See Also: {references}

 * External functions used:

 * Support: Tony Farrell, AAO

 * History:
      16-Oct-2006 - TJF - Original version, from ozpoz code.
 */
extern void TdfNormalizeLinear  (
    double rotation,
    double scale,
    double nonperp,
    double *a,
    double *b,
    double *c,
    double *d,
    double *e,
    double *f,
    StatusType *status)
{
    if (*status != STATUS__OK) return;
    
    if (scale == 0)
    { 
        *status = TDFXY__ILLSCALE;
        return;
    }
    else if (scale != 1)
    {
        *b *= scale;
        *c *= scale;
        *e *= scale;
        *f *= scale;
    }
    

    if (rotation != 0)
    {
        double cosine;
        double sine;
        double b2, c2, e2, f2;
        double an, bn, cn, dn, en, fn;

        cosine = cos(rotation*DD2R);
        sine   = sin(rotation*DD2R);
        /*printf("Rotation of %.20f\n", rotation);*/
        /*
         * We are rotating an axis, so we use
         *
         * x' =  cos(rotation)*x + sin(rotation)*y
         * y' = -sin(rotation)*x + cos(rotation)*y
         *
         * Short hand for rotation coefficents
         */
        b2 = cosine;
        c2 = sine;
        e2 = -1 * sine;
        f2 = cosine;

        /*
         * The order in which we do this is important (it was
         * not for rotation).  (Remember - in matrix multiplication,
         *  A.B != B.A  (through it happens to be ok for rotation).
         *
         * We apply the rotation first
         *
         * x1 = b2.x0 + c2.y0   (1)
         * y1 = e2.x0 + f2.y0   (2)
         *
         * Then the original linear model 
         *
         * x2 = a + b.x1 + c.y1     (3)
         * y2 = d + e.x1 + f.y1     (4)
         *
         * combining (1), (2) and (3) and (4), we get
         *
         * x2 = a + b.(b2.x0 + c2.y0) + c.(e2.x0 + f2.y0)
         * y2 = d + e.(b2.x0 + c2.y0) + f.(e2.x0 + f2.y0)
         *
         * Which is equivalent to this.
         *
         * x2 = a + (b.b2 + c.e2) x0 + (b.c2 + c.f2) y0
         * y2 = d + (e.b2 + f.e2) x0 + (e.c2 + f.f2) y0
         *
         * Hence we can see our new parameters.
         */
        an = *a;
        bn = (*b)*b2 + (*c)*e2;
        cn = (*b)*c2 + (*c)*f2;
        dn = *d;
        en = (*e)*b2 + (*f)*e2;
        fn = (*e)*c2 + (*f)*f2;

        *a = an;
        *b = bn;
        *c = cn;
        *d = dn;
        *e = en;
        *f = fn;
    }
    if (nonperp != 0)
    {
        double cosine;
        double sine;
        double b2, c2, e2, f2;
        double an, bn, cn, dn, en, fn;

        double radians = nonperp*DD2R;

        cosine = cos(radians/2.0);
        sine   = sin(radians/2.0);
            
        /*
         * A non-perpendicularity transformation of the model is given by
         *
         *   x' = cos(perp/2)*x + sin(perp/2)*y
         *   y' = sin(perp/2)*x + cos(perp/2)*y
         *
         * Short hand for coefficents.
         */
        b2 = cosine;
        c2 = sine;
        e2 = sine;
        f2 = cosine;

        /*
         * The order in which we do this is important (it was
         * not for rotation).  (Remember - in matrix multiplication,
         *  A.B != B.A  (through it happens to be ok for rotation).
         *
         * We apply the non-perp first
         *
         * x1 = b2.x0 + c2.y0   (1)
         * y1 = e2.x0 + f2.y0   (2)
         *
         * Then the original linear model 
         *
         * x2 = a + b.x1 + c.y1     (3)
         * y2 = d + e.x1 + f.y1     (4)
         *
         * combining (1), (2) and (3) and (4), we get
         *
         * x2 = a + b.(b2.x0 + c2.y0) + c.(e2.x0 + f2.y0)
         * y2 = d + e.(b2.x0 + c2.y0) + f.(e2.x0 + f2.y0)
         *
         * Which is equivalent to this.
         *
         * x2 = a + (b.b2 + c.e2) x0 + (b.c2 + c.f2) y0
         * y2 = d + (e.b2 + f.e2) x0 + (e.c2 + f.f2) y0
         *
         * Hence we can see our new parameters.
         */
        an = *a;
        bn = (*b)*b2 + (*c)*e2;
        cn = (*b)*c2 + (*c)*f2;
        dn = *d;
        en = (*e)*b2 + (*f)*e2;
        fn = (*e)*c2 + (*f)*f2;

        *a = an;
        *b = bn;
        *c = cn;
        *d = dn;
        *e = en;
        *f = fn;
    }


}


