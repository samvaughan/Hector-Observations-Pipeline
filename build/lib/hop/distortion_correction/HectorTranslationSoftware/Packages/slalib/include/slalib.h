#ifndef SLALIBHDEF
#define SLALIBHDEF

/*
**  - - - - - - - - -
**   s l a l i b . h
**  - - - - - - - - -
**
**  Prototype function declarations for slalib library.
**
**  Last revision:   26 September 2008
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

#include <math.h>

/* Star-independent ICRS-to-CIRS parameters */
typedef struct {
   double pmt;        /* time interval for proper motion (Julian years) */
   double eb[3];      /* SSB to Earth (AU) */
   double ehn[3];     /* Sun to Earth unit vector */
   double gr2e;       /* (grav rad Sun)*2/(Sun-Earth distance) */
   double abv[3];     /* barycentric Earth velocity in units of c */
   double ab1;        /* sqrt(1-v**2) where v=modulus(abv) */
   double bpn[3][3];  /* bias-precession-nutation matrix */
} CIpars;

/* Star-independent intermediate-to-observed parameters */
typedef struct {
   double along;      /* longitude + s' + dERA(DUT) (radians) */
   double phi;        /* geodetic latitude (radians) */
   double hm;         /* height above sea level (metres) */
   double xpl;        /* polar motion xp wrt local meridian (radians) */
   double ypl;        /* polar motion yp wrt local meridian (radians) */
   double sphi;       /* sine of geodetic latitude */
   double cphi;       /* cosine of geodetic latitude */
   double diurab;     /* magnitude of diurnal aberration vector */
   double p;          /* pressure (mb,hPa) */
   double tk;         /* ambient temperature (K) */
   double rh;         /* relative humidity (0-1) */
   double tlr;        /* tropospheric lapse rate (K per metre) */
   double wl;         /* wavelength (micron) or minus frequency (GHz) */
   double refa;       /* refraction constant A (radians) */
   double refb;       /* refraction constant B (radians) */
   double eral;       /* "Local" Earth Rotation Angle (radians) */
} IOpars;

#ifdef __cplusplus
extern "C" {
#endif

void slaAddet ( double, double, double, double*, double* );

void slaAfin ( const char*, int*, float*, int* );

double slaAirmas ( double zd );

void slaAltaz ( double, double, double, double*, double*, double*,
                double*, double*, double*, double*, double*, double* );

void slaAmp ( double, double, double, double, double*, double* );

void slaAmpqk ( double, double, double[21], double*, double* );

void slaAop ( double, double, double, double, double, double, double,
              double, double, double, double, double, double, double,
              double*, double*, double*, double*, double* );

void slaAoppa ( double, double, double, double,
                double, double, double, double, double,
                double, double, double, double[14] );

void slaAoppat ( double, double[14] );

void slaAopqk ( double, double, double[14], double*, double*, double*,
                double*, double* );

void slaAtmdsp ( double, double, double, double,
                 double, double, double, double*, double* );

void slaAv2m ( float[3], float[3][3] );

float slaBear ( float, float, float, float );

void slaC2i ( double, double, double, double,
              double, double, double, double, double,
              double*, double* );

void slaC2ipas ( double, double[2][3], double, double, CIpars*, double* );

void slaC2ipa ( double, double, double, CIpars*, double* );

void slaC2ipad ( CIpars* );

void slaC2iqk ( double, double, double, double, double, double, CIpars*,
                double*, double* );

void slaC2iqkz  ( double, double, CIpars*, double*, double* );

void slaCaf2r ( int, int, float, float*, int* );

void slaCaldj ( int, int, int, double*, int* );

void slaCalyd ( int, int, int, int*, int*, int* );

void slaCc2s ( float[3], float*, float* );

void slaCc62s ( float[6], float*, float*, float*, float*,
                float*, float* );

void slaCd2tf ( int, float, char*, int[4] );

void slaCldj ( int, int, int, double*, int* );

void slaClyd ( int, int, int, int*, int*, int* );

void slaCombn ( int, int, int[], int* );

void slaCr2af ( int, float, char*, int[4] );

void slaCr2tf ( int, float, char*, int[4] );

void slaCs2c ( float, float, float[3] );

void slaCs2c6 ( float, float, float, float, float, float, float[6] );

void slaCtf2d ( int, int, float, float*, int* );

void slaCtf2r ( int, int, float, float*, int* );

void slaDaf2r ( int, int, double, double*, int* );

void slaDafin ( const char*, int*, double*, int* );

double slaDat ( double );

void slaDav2m ( double[3], double[3][3] );

double slaDbear ( double, double, double, double );

void slaDbjin ( const char*, int*, double*, int*, int* );

void slaDc62s ( double[6], double*, double*, double*,
                double*, double*, double* );

void slaDcc2s ( double[3], double*, double* );

void slaDcmpf ( double[6], double*, double*, double*,
                double*, double*, double* );

void slaDcs2c ( double, double, double[3] );

void slaDd2tf ( int, double, char*, int[4] );

void slaDe2h ( double, double, double, double*, double* );

void slaDeuler ( const char*, double, double, double, double[3][3] );

void slaDfltin ( const char*, int*, double*, int* );

void slaDh2e ( double, double, double, double*, double* );

void slaDimxv ( double[3][3], double[3], double[3] );

void slaDjcal ( int, double, int[4], int* );

void slaDjcl ( double, int*, int*, int*, double*, int* );

void slaDm2av ( double[3][3], double[3] );

void slaDmat ( int, double*, double*, double*, int*, int* );

void slaDmoon ( double, double[6] );

void slaDmxm ( double[3][3], double[3][3], double[3][3] );

void slaDmxv ( double[3][3], double[3], double[3] );

double slaDpav ( double[3], double[3] );

void slaDr2af ( int, double, char*, int[4] );

void slaDr2tf ( int, double, char*, int[4] );

double slaDrange ( double );

double slaDranrm ( double );

void slaDs2c6 ( double, double, double, double, double, double,
                double[6] );

void slaDs2tp ( double, double, double, double, double*, double*, int* );

double slaDsep ( double, double, double, double );

double slaDsepv ( double[3], double[3] );

double slaDt ( double );

void slaDtf2d ( int, int, double, double*, int* );

void slaDtf2r ( int, int, double, double*, int* );

void slaDtp2s ( double, double, double, double, double*, double* );

void slaDtp2v ( double, double, double[3], double[3] );

void slaDtps2c ( double, double, double, double, double*, double*,
                 double*, double*, int* );

void slaDtpv2c ( double, double, double[3], double[3], double[3], int* );

double slaDtt ( double );

void slaDv2tp ( double[3], double[3], double*, double*, int* );

double slaDvdv ( double[3], double[3] );

void slaDvn ( double[3], double[3], double* );

void slaDvxv ( double[3], double[3], double[3] );

void slaE2h ( float, float, float, float*, float* );

void slaEarth ( int, int, float, float[6] );

void slaEcleq ( double, double, double, double*, double* );

void slaEcmat ( double, double[3][3] );

void slaEcor ( float, float, int, int, float, float*, float* );

void slaEg50 ( double, double, double*, double* );

void slaEl2ue ( double, int, double, double, double, double, double,
                double, double, double, double[], int* );
double slaEo ( double );

double slaEors ( double[3][3], double );

double slaEpb ( double );

double slaEpb2d ( double );

double slaEpco ( char, char, double );

double slaEpj ( double );

double slaEpj2d ( double );

void slaEpv ( double, double[3], double[3], double[3], double[3] );

void slaEqecl ( double, double, double, double*, double* );

double slaEqeqx ( double );

void slaEqgal ( double, double, double*, double* );

double slaEra ( double, double );

void slaEtrms ( double, double[3] );

void slaEuler ( const char*, float, float, float, float[3][3] );

void slaEvp ( double, double, double[3], double[3], double[3], double[3] );

void slaFitxy ( int, int, double[][2], double[][2], double[6], int* );

void slaFk425 ( double, double, double, double, double, double,
                double*, double*, double*, double*, double*, double* );

void slaFk45z ( double, double, double, double*, double* );

void slaFk524 ( double, double, double, double, double, double,
                double*, double*, double*, double*, double*, double* );

void slaFk52h ( double, double, double, double,
                double*, double*, double*, double* );

void slaFk54z ( double, double, double, double*, double*,
                double*, double* );

void slaFk5hz ( double, double, double, double*, double* );

void slaFlotin ( const char*, int*, float*, int* );

void slaFw2m ( double, double, double, double, double[3][3] );

void slaFw2xy ( double, double, double, double, double*, double* );

void slaG2ixys ( double, double, double, double[3][3] );

void slaGaleq ( double, double, double*, double* );

void slaGalsup ( double, double, double*, double* );

void slaGe50 ( double, double, double*, double* );

void slaGeoc ( double, double, double*, double* );

double slaGmst ( double );

double slaGmsta ( double, double );

double slaGst ( double, double, double );

void slaH2e ( float, float, float, float*, float* );

void slaH2fk5 ( double, double, double, double,
                double*, double*, double*, double* );

void slaHfk5z ( double, double, double,
                double*, double*, double*, double* );

void slaI2c ( double, double, double, double, double, double*, double* );

void slaI2cqk ( double, double, CIpars*, double*, double* );

void slaI2o ( double, double, double, double, double, double, double,
              double, double, double, double, double, double, double,
              double*, double*, double*, double*, double* );

void slaI2opa ( double, double, double, double, double, double, double,
                double, double, double, double, double, IOpars* );

void slaI2opad ( IOpars* );

void slaI2opat ( double, IOpars* );

void slaI2oqk ( double, double, IOpars*, double*, double*, double*,
                double*, double* );

void slaImxv ( float[3][3], float[3], float[3] );

void slaInt2in ( const char*, int*, int*, int* );

void slaIntin ( const char*, int*, long*, int* );

void slaInvf ( double[6], double[6], int* );

void slaKbj ( int, double, char*, int* );

void slaM2av ( float[3][3], float[3] );

void slaMap ( double, double, double, double, double, double,
              double, double, double*, double* );

void slaMappa ( double, double, double[21] );

void slaMapqk ( double, double, double, double, double, double,
                double[21], double*, double* );

void slaMapqkz ( double, double, double[21], double*, double* );

void slaMoon ( int, int, float, float[6] );

void slaMxm ( float[3][3], float[3][3], float[3][3] );

void slaMxv ( float[3][3], float[3], float[3] );

void slaNu ( double, double*, double* );

void slaNu00a ( double, double*, double* );

void slaNut ( double, double[3][3] );

void slaNutc ( double, double*, double*, double* );

void slaNutc80 ( double, double*, double*, double* );

void slaO2i ( const char*, double, double, double, double, double,
              double, double, double, double, double, double,
              double, double, double, double*, double* );

void slaO2iqk ( const char*,
                double, double, IOpars*, double*, double* );

void slaOap ( const char*, double, double, double, double, double,
              double, double, double, double, double, double,
              double, double, double, double*, double* );

void slaOapqk ( const char*,
                double, double, double[14], double*, double* );

void slaObs ( int, char*, char*, double*, double*, double* );

double slaPa ( double, double, double );

double slaPav ( float[3], float[3] );

void slaPcd ( double, double*, double* );

void slaPda2h ( double, double, double, double*, int*, double*, int* );

void slaPdq2h ( double, double, double, double*, int*, double*, int* );

void slaPermut ( int, int[], int[], int* );

void slaPertel (int, double, double, double, double, double, double,
                double, double, double, double*, double*, double*,
                double*, double*, double*, double*, int* );

void slaPertue ( double, double[], int* );

void slaPfw ( double, double*, double*, double*, double* );

void slaPlanel ( double, int, double, double, double, double, double,
                 double, double, double, double[6], int* );

void slaPlanet ( double, int, double[6], int* );

void slaPlante ( double, double, double, int, double, double, double,
                 double, double, double, double, double,
                 double*, double*, double*, int* );

void slaPlantu ( double, double, double, double[],
                 double*, double*, double*, int* );

void slaPm ( double, double, double, double, double, double, double,
             double, double*, double* );

void slaPncio ( double, double[3][3] );

void slaPneqx ( double, double[3][3] );

void slaPolmo ( double, double, double, double,
                double*, double*, double* );

void slaPomom ( double, double, double, double[3][3] );

void slaPrebn ( double, double, double[3][3] );

void slaPrec ( double, double, double[3][3] );

void slaPrecl ( double, double, double[3][3] );

void slaPreces ( const char[3], double, double, double*, double* );

void slaPrenut ( double, double, double[3][3] );

void slaPv2el ( double[], double, double, int,
                int*, double*, double*, double*, double*,
                double*, double*, double*, double*, int* );

void slaPv2ue ( double[], double, double, double[], int* );

void slaPvobs ( double, double, double, double[6] );

void slaPxy ( int, double[][2], double[][2], double[6],
              double[][2], double*, double*, double* );

float slaRange ( float );

float slaRanorm ( float );

double slaRcc ( double, double, double, double, double );

void slaRdplan ( double, int, double, double, double*, double*, double* );

void slaRefco ( double, double, double, double, double, double, double,
                double, double*, double* );

void slaRefcoq ( double, double, double, double, double*, double* );

void slaRefro ( double, double, double, double, double, double, double,
                double, double, double* );

void slaRefv ( double[3], double, double, double[3] );

void slaRefz ( double, double, double, double* );

float slaRverot ( float, float, float, float );

float slaRvgalc ( float, float );

float slaRvlg ( float, float );

float slaRvlsrd ( float, float );

float slaRvlsrk ( float, float );

double slaS ( double, double, double );

void slaS2tp ( float, float, float, float, float*, float*, int* );

float slaSep ( float, float, float, float );

float slaSepv ( float[3], float[3] );

void slaSmat ( int, float*, float*, float*, int*, int* );

double slaSp ( double );

void slaSubet ( double, double, double, double*, double* );

void slaSupgal ( double, double, double*, double* );

void slaSvd ( int, int, int, int, double*, double*, double*, double*, int* );

void slaSvdcov ( int, int, int, double*, double*, double*, double* );

void slaSvdsol ( int, int, int, int, double*, double*, double*, double*,
                 double*, double* );

void slaTp2s ( float, float, float, float, float*, float* );

void slaTp2v ( float, float, float[3], float[3] );

void slaTps2c ( float, float, float, float, float*, float*,
                float*, float*, int* );

void slaTpv2c ( float, float, float[3], float[3], float[3], int* );

void slaUe2el ( double[], int, int*, double*, double*, double*, double*,
                double*, double*, double*, double*, int* );

void slaUe2pv ( double, double[], double[], int* );

void slaUnpcd ( double, double*, double* );

void slaV2tp ( float[3], float[3], float*, float*, int* );

float slaVdv ( float[3], float[3] );

void slaVn ( float[3], float[3], float* );

void slaVxv ( float[3], float[3], float[3] );

void slaXy2xy ( double, double, double[6], double*, double* );

double slaZd ( double, double, double );

#ifdef __cplusplus
}
#endif

#endif
