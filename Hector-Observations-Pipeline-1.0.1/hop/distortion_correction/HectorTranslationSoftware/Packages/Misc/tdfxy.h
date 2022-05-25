#ifndef __TDFXY__
#define __TDFXY__ 1

/*
 *      @(#) $Id: ACMM:2dFutil/tdfxy.h,v 5.14 25-Jun-2020 14:50:31+10 tjf $
 */




#include "tdfxy_err.h"

#ifdef __cplusplus
 extern "C" {
#endif


/*
 * Distortion map - hold the Cannon Fpi to sky distortion map.
 *  This will be used in the linear model conversion (in theory, it
 *  should be used in the distortion model conversion, but since it
 *  is calculated using field plate coorindates it is better to apply
 *  it in the linear model conversion.
 */
#define TDFXY_GRID_WIDTH 23
typedef struct {
    double x[TDFXY_GRID_WIDTH*TDFXY_GRID_WIDTH];
    double y[TDFXY_GRID_WIDTH*TDFXY_GRID_WIDTH];
    int loaded;
} TdfDistMapType;



typedef struct TdfDistType
{
    double a;
    double b;
    double c;
    double d;
      double x0;
    double y0;
} TdfDistType;
   
typedef struct TdfLinType
{
    double coeffs[6];
    double extraScale;  /* An extra scale effect to apply */
    double extraRotation; /* An extra rotation effect to apply (degrees) */
    double extraNonPerp; /* an extra  Non-perpendicularity to apply */
    TdfDistMapType distMap; /* A map between the size and field plate
                               after all the other bits are taken out */
} TdfLinType;

typedef struct TdfXyType
   {
      double cenAoprms[14];  /*  Apparent to observed parameters- field cen  */
      double obsAoprms[14];  /*  Apparent to observed parameters - object */
      double rot[3][3];      /*  Rotation matrix Az,El to mount frame  */
#if 0
      double adc_a;          /*  ADC angle element A  */
      double adc_b;          /*  ADC angle element B  */
#else
      double reserved1;      /*  Reserved for binary compability */
      double reserved2;      /*   after adc_a and adc_b were dropped */
#endif
      TdfDistType dist;      /*  Distortion model parameters  */
#if 0
      double NP;             /*  Axis non-perpendicularity, radians  */
      double CH;             /*  Hour angle collimation error, radians  */
      double HF;             /*  Horseshoe flexure, radians  */
      double ZGT;            /*  Z gear term (correction to HA) radians */
#else
       double reserved3;     /* Reserved to ensure binary compatiblity */
       double reserved4;     /*  after NP etc were dropped */
       double reserved5;
       double reserved6;
#endif
      double cenLambda;      /*  Tel pointing wavelength  */
      double obsLambda;      /*  Observing wavelength  */
   }  TdfXyType;

void TdfXyInit(double mjd, double dut, double temp, double press, 
                double humid, double cenWave, double obsWave,
                double ma, double me, double np,
                double ch, double hf, double zgt, double adc_a, 
                double adc_b, TdfDistType dist, 
                TdfXyType *xypars, StatusType *status);

void TdfRd2xy(TdfXyType *xypars, double cra, double cdec, double ra, 
                double dec, double mjd, double *x, double *y, 
                StatusType *status);

void TdfDistXy(TdfDistType dist, double lambda, double adc_a, double adc_b,
             double mha, double mdec,
             double xi, double eta, double *x, double *y, StatusType *status);

void TdfDistXyInv(TdfDistType dist, double lambda, double adc_a, double adc_b,
             double mha, double mdec,
             double x, double y, double *xi, double *eta, StatusType *status);

void Tdf___App2mount(int cen, TdfXyType *xypars, double ra, double dec,
                    double *mha, double *mdec, StatusType *status);

void Tdf___DistLambda(double lambda, TdfDistType dist, double *a,
                  double *b, double *c, double *d);

void TdfXy2rd(TdfXyType *xypars, double cra, double cdec, double x,
                double y, double mjd, double *ra, double *dec,
                StatusType *status);
     
              
void TdfXy2pos(TdfLinType *linpars, double x, double y, double *xp, double *yp);

void TdfPos2xy(TdfLinType *linpars, double xp, double yp, double *x, double *y);

void TdfGetDist(char *path, TdfDistType *dist, StatusType *status);

void TdfGetDist2(char *path0, char* path1, 
                                  TdfDistType *dist, StatusType *status);

void TdfGetLin(char *path, TdfLinType *linpars, StatusType *status);

void TdfGetLin2(char *path0, char* path1, 
                               TdfLinType *linpars, StatusType *status);

void TdfSetLin(double coeffs[6], char *file, StatusType *status);

void TdfSetDist(TdfDistType dist, char *file, StatusType *status);

void TdfSave (const char * file, int usePath, const char *pathName,
	      SdsIdType sdsId, int show, const char *message,
	      StatusType *status);
#if 0
double TdfHfxTerm(double ha, double dec, double z);
#endif
/*
 * This structure allows the model parameters to be moved in and out 
 * of the TdfLinType and TdfDistType structures.
 */
#define TDFXY_NUM_PARS 20
typedef struct  {
    double p[TDFXY_NUM_PARS];
} TdfModelPars;

/*
 * Indicies to entries in the above structure.   These are appropiate
 * for use by Echidna Telescope model calibrate software - in 2dF mode.
 *
 *  NOTE - The EXTRA values (ROT/SCALE/NONPERP) are not yet handled correctly,
 *   they are currently just placeholders.
 *   
 */
#define TDFXY_PAR_FIRSTLIN 0           /* First linear model parameter index */
#define TDFXY_PAR_LASTLIN  5           /* Last linear model parameter index */
#define TDFXY_PAR_ROT      6           /* Extra Rotation parameter index    */
#define TDFXY_PAR_SCALE    7           /* Extra Scale parameter index       */
#define TDFXY_PAR_NONPERP  9           /* Non-perpendicularity              */

#define TDFXY_PAR_DIST_FIRST  11      /* First distortion parameter index */
#define TDFXY_PAR_DIST_LAST   16      /* First distortion parameter */


void TdfSetModel(const TdfModelPars *model,
                 TdfDistType *dist,
                 TdfLinType *linpars, 
                 StatusType *status);

void TdfGetModel(const TdfDistType dist,
                 const TdfLinType linpars, 
                 TdfModelPars *model,
                 StatusType *status);

extern void TdfNormalizeModel  (
    TdfDistType *distPars,
    TdfLinType  *linPars,
    StatusType *status);
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
    StatusType *status);
void TdfRd2tan(
    TdfXyType *xypars, double cra, double cdec, double ra, 
    double dec, double mjd, double *xi, double *eta, 
    double *cmha,
    double *cmdec,
    StatusType *status);


/*
 * Test are only for use by test program.
 */
void Tdf___ApplyDistMap(
    const TdfDistMapType * const map,
    const int positive,
    double *const x,
    double *const y);
void Tdf___DumpDistMap(
    FILE *file,
    const TdfDistMapType * const map,
    StatusType * const status);


#ifdef __cplusplus
}
#endif

#endif
