/* Copyright (C) 2009 P.T.Wallace. */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <slalib.h>
#include <slamac.h>
void vcs ( char *s, char *sok, char *func, char *test, int *status );
void viv ( int ival, int ivalok, char *func, char *test, int *status );
void vlv ( long ival, long ivalok, char *func, char *test, int *status );
void vvd ( double val, double valok, double dval,
           char *func, char *test, int *status );
void err ( char *func, char *test, int *status );
void t_addet ( int *status );
void t_afin ( int *status );
void t_airmas ( int *status );
void t_altaz ( int *status );
void t_amp ( int *status );
void t_aop ( int *status );
void t_bear ( int *status );
void t_c2i ( int *status );
void t_caf2r ( int *status );
void t_caldj ( int *status );
void t_calyd ( int *status );
void t_cc2s ( int *status );
void t_cc62s ( int *status );
void t_cd2tf ( int *status );
void t_cldj ( int *status );
void t_cr2af ( int *status );
void t_cr2tf ( int *status );
void t_cs2c6 ( int *status );
void t_ctf2d ( int *status );
void t_ctf2r ( int *status );
void t_dat ( int *status );
void t_dbjin ( int *status );
void t_djcal ( int *status );
void t_dmat ( int *status );
void t_e2h ( int *status );
void t_earth ( int *status );
void t_ecleq ( int *status );
void t_ecmat ( int *status );
void t_ecor ( int *status );
void t_eg50 ( int *status );
void t_epb ( int *status );
void t_epb2d ( int *status );
void t_epco ( int *status );
void t_epj ( int *status );
void t_epj2d ( int *status );
void t_eqecl ( int *status );
void t_eqeqx ( int *status );
void t_eqgal ( int *status );
void t_etrms ( int *status );
void t_evp ( int *status );
void t_fitxy ( int *status );
void t_fk425 ( int *status );
void t_fk45z ( int *status );
void t_fk524 ( int *status );
void t_fk52h ( int *status );
void t_fk54z ( int *status );
void t_flotin ( int *status );
void t_galeq ( int *status );
void t_galsup ( int *status );
void t_ge50 ( int *status );
void t_gst ( int *status );
void t_i2o ( int *status );
void t_intin ( int *status );
void t_kbj ( int *status );
void t_map ( int *status );
void t_moon ( int *status );
void t_nut ( int *status );
void t_obs ( int *status );
void t_pa ( int *status );
void t_pcd ( int *status );
void t_pda2h ( int *status );
void t_pdq2h ( int *status );
void t_percom ( int *status );
void t_planet ( int *status );
void t_pm ( int *status );
void t_pncio ( int *status );
void t_pomom ( int *status );
void t_polmo ( int *status );
void t_prebn ( int *status );
void t_prec ( int *status );
void t_preces ( int *status );
void t_prenut ( int *status );
void t_pvobs ( int *status );
void t_range ( int *status );
void t_ranorm ( int *status );
void t_rcc ( int *status );
void t_ref ( int *status );
void t_rv ( int *status );
void t_sep ( int *status );
void t_smat ( int *status );
void t_supgal ( int *status );
void t_svd ( int *status );
void t_tp ( int *status );
void t_tpv ( int *status );
void t_vecmat ( int *status );
void t_zd ( int *status );
int main ()

{
   int status;    


   status = 0;


   t_addet ( &status );
   t_afin ( &status );
   t_airmas ( &status );
   t_altaz ( &status );
   t_amp ( &status );
   t_aop ( &status );
   t_bear ( &status );
   t_c2i ( &status );
   t_caf2r ( &status );
   t_caldj ( &status );
   t_calyd ( &status );
   t_cc2s ( &status );
   t_cc62s ( &status );
   t_cd2tf ( &status );
   t_cldj ( &status );
   t_cr2af ( &status );
   t_cr2tf ( &status );
   t_cs2c6 ( &status );
   t_ctf2d ( &status );
   t_ctf2r ( &status );
   t_dat ( &status );
   t_dbjin ( &status );
   t_djcal ( &status );
   t_dmat ( &status );
   t_e2h ( &status );
   t_earth ( &status );
   t_ecleq ( &status );
   t_ecmat ( &status );
   t_ecor ( &status );
   t_eg50 ( &status );
   t_epb ( &status );
   t_epb2d ( &status );
   t_epco ( &status );
   t_epj ( &status );
   t_epj2d ( &status );
   t_eqecl ( &status );
   t_eqeqx ( &status );
   t_eqgal ( &status );
   t_etrms ( &status );
   t_evp ( &status );
   t_fitxy ( &status );
   t_fk425 ( &status );
   t_fk45z ( &status );
   t_fk524 ( &status );
   t_fk52h ( &status );
   t_fk54z ( &status );
   t_flotin ( &status );
   t_galeq ( &status );
   t_galsup ( &status );
   t_ge50 ( &status );
   t_gst ( &status );
   t_i2o ( &status );
   t_intin ( &status );
   t_kbj ( &status );
   t_map ( &status );
   t_moon ( &status );
   t_nut ( &status );
   t_obs ( &status );
   t_pa ( &status );
   t_pcd ( &status );
   t_pda2h ( &status );
   t_pdq2h ( &status );
   t_percom ( &status );
   t_planet ( &status );
   t_pm ( &status );
   t_pncio ( &status );
   t_pomom ( &status );
   t_polmo ( &status );
   t_prebn ( &status );
   t_prec ( &status );
   t_preces ( &status );
   t_prenut ( &status );
   t_pvobs ( &status );
   t_range ( &status );
   t_ranorm ( &status );
   t_rcc ( &status );
   t_ref ( &status );
   t_rv ( &status );
   t_sep ( &status );
   t_smat ( &status );
   t_supgal ( &status );
   t_svd ( &status );
   t_tp ( &status );
   t_tpv ( &status );
   t_vecmat ( &status );
   t_zd ( &status );


   if ( status ) printf ( "\n! slalib validation failed !\n" );
   return status;
}
void vcs ( char *s, char *sok, char *func, char *test, int *status )

{
   if ( strcmp ( s, sok ) ) {
      err ( func, test, status );
      printf ( "  expected = %s\n  actual =   %s\n", sok, s );
   }
}
void viv ( int ival, int ivalok, char *func, char *test, int *status )

{
   if ( ival != ivalok ) {
      err ( func, test, status );
      printf ( "  expected = %d\n  actual =   %d\n", ivalok, ival );
   }
}
void vlv ( long ival, long ivalok, char *func, char *test, int *status )

{
   if ( ival != ivalok ) {
      err ( func, test, status );
      printf ( "  expected = %ld\n  actual =   %ld\n", ivalok, ival );
   }
}
void vvd ( double val, double valok, double dval,
           char *func, char *test, int *status )

{
   if ( fabs ( val - valok ) > dval ) {
      err ( func, test, status );
      printf ( "  expected = %30.19g\n  actual =   %30.19g\n", valok, val );
   }
}
void err ( char *func, char *test, int *status )

{
   printf ( "%s test %s%sfails:\n",
            func, test, ( ! strcmp ( test, "" ) ) ? "" : " " );
   *status = 1;
}
void t_addet ( int *status )

{
   double rm = 2.0;
   double dm = -1.0;
   double eq = 1975.0;
   double r1, d1, r2, d2;

   slaAddet ( rm, dm, eq, &r1, &d1 );
   vvd ( r1 - rm, 2.983864874295250e-6, 1e-12, "slaAddet", "r", status );
   vvd ( d1 - dm, 2.379650804185118e-7, 1e-12, "slaAddet", "d", status );

   slaSubet ( r1, d1, eq, &r2, &d2 );
   vvd ( r2 - rm, 0.0, 1e-12, "slaSubet", "r", status );
   vvd ( d2 - dm, 0.0, 1e-12, "slaSubet", "d", status );
}
void t_afin ( int *status )

{
   int i, j;
   float f;
   double d;
   char *s = "12 34 56.7 |";

   i = 1;
   slaAfin ( s, &i, &f, &j );
   viv ( i, 12, "slaAfin", "i", status );
   vvd ( (double) f , 0.2196045986911432, 1e-6, "slaAfin", "a", status );
   viv ( j, 0, "slaAfin", "j", status );
   i = 1;
   slaDafin ( s, &i, &d, &j );
   viv ( i, 12, "slaDafin", "i", status );
   vvd ( d, 0.2196045986911432, 1e-12, "slaDafin", "a", status );
   viv ( j, 0, "slaDafin", "j", status );
}
void t_airmas ( int *status )

{
   vvd ( slaAirmas ( 1.2354 ), 3.015698990074724, 1e-12,
         "slaAirmas", "", status );
}
void t_altaz ( int *status )

{
   double az, azd, azdd, el, eld, eldd, pa, pad, padd;

   slaAltaz ( 0.7, -0.7, -0.65,
              &az, &azd, &azdd, &el, &eld, &eldd, &pa, &pad, &padd );

   vvd ( az, 4.400560746660174, 1e-12, "slaAltaz", "az", status );
   vvd ( azd, -0.2015438937145421, 1e-13, "slaAltaz", "azd", status );
   vvd ( azdd, -0.4381266949668748, 1e-13, "slaAltaz", "azdd", status );
   vvd ( el, 1.026646506651396, 1e-12, "slaAltaz", "el", status );
   vvd ( eld, -0.7576920683826450, 1e-13, "slaAltaz", "eld", status );
   vvd ( eldd, 0.04922465406857453, 1e-14, "slaAltaz", "eldd", status );
   vvd ( pa, 1.707639969653937, 1e-12, "slaAltaz", "pa", status );
   vvd ( pad, 0.4717832355365627, 1e-13, "slaAltaz", "pad", status );
   vvd ( padd, -0.2957914128185515, 1e-13, "slaAltaz", "padd", status );
}
void t_amp ( int *status )

{
   double rm, dm;

   slaAmp ( 2.345, -1.234, 50100.0, 1990.0, &rm, &dm );
   vvd ( rm, 2.344472180027961, 1e-12, "slaAmp", "r", status );
   vvd ( dm, -1.233573099847705, 1e-12, "slaAmp", "d", status );
}
void t_aop ( int *status )

{
   int i;
   double rap, dap, date, dut, elongm, phim, hm, xp, yp, tdk, pmb,
          rh, wl=0.0, tlr, aob, zob, hob, dob, rob, aoprms[14];

   dap = -0.1234;
   date = 51000.1;
   dut = 25.0;
   elongm = 2.1;
   phim = 0.5;
   hm = 3000.0;
   xp = -0.5e-6;
   yp = 1e-6;
   tdk = 280.0;
   pmb = 550.0;
   rh = 0.6;
   tlr = 0.006;

   for ( i = 1; i <= 3; i++ ) {
      if ( i == 1 ) {
         rap = 2.7;
         wl = 0.45;
      } else if ( i == 2 ) {
         rap = 2.345;
      } else {
         wl = 1e6;
      }
      slaAop ( rap, dap, date, dut, elongm, phim, hm, xp, yp, tdk,
                pmb, rh, wl, tlr, &aob, &zob, &hob, &dob, &rob );
      if ( i == 1 ) {
         vvd ( aob,  1.812817787118544, 1e-10, 
                                          "slaAop", "lo aob", status );
         vvd ( zob,  1.3938608178356764, 1e-10,
                                          "slaAop", "lo zob", status );
         vvd ( hob, -1.2978080101328584, 1e-10,
                                          "slaAop", "lo hob", status );
         vvd ( dob, -0.12296706114595433, 1e-10,
                                          "slaAop", "lo dob", status );
         vvd ( rob,  2.6992702889033513, 1e-10,
                                          "slaAop", "lo rob", status );
      } else if ( i == 2 ) {
         vvd ( aob, 2.019928026664455, 1e-10,
                                          "slaAop", "aob/o", status );
         vvd ( zob,  1.1013161728493077, 1e-10,
                                          "slaAop", "zob/o", status );
         vvd ( hob, -0.94329235619171059, 1e-10,
                                          "slaAop", "hob/o", status );
         vvd ( dob, -0.12321447107004865, 1e-10,
                                          "slaAop", "dob/o", status );
         vvd ( rob,  2.3447546349622037, 1e-10,
                                          "slaAop", "rob/o", status );
      } else {
         vvd ( aob,  2.019928026664455, 1e-10,
                                          "slaAop", "aob/r", status );
         vvd ( zob,  1.1012675510474201, 1e-10,
                                          "slaAop", "zob/r", status );
         vvd ( hob, -0.94325332894747971, 1e-10,
                                          "slaAop", "hob/r", status );
         vvd ( dob, -0.12318507795209942, 1e-10,
                                          "slaAop", "dob/r", status );
         vvd ( rob,  2.3447156077179727, 1e-10,
                                          "slaAop", "rob/r", status );
      }
   }

   date = 48000.3;
   wl = 0.45;

   slaAoppa ( date, dut, elongm, phim, hm, xp, yp, tdk,
               pmb, rh, wl, tlr, aoprms );
   vvd ( aoprms[0], 0.4999993892136306, 1e-13, "slaAoppa", "0", status );
   vvd ( aoprms[1], 0.4794250025886467, 1e-13, "slaAoppa", "1", status );
   vvd ( aoprms[2], 0.8775828547167932, 1e-13, "slaAoppa", "2", status );
   vvd ( aoprms[3], 1.363180872136126e-6, 1e-13, "slaAoppa", "3", status );
   vvd ( aoprms[4], 3000.0, 1e-10, "slaAoppa", "4", status );
   vvd ( aoprms[5], 280.0, 1e-11, "slaAoppa", "5", status );
   vvd ( aoprms[6], 550.0, 1e-11, "slaAoppa", "6", status );
   vvd ( aoprms[7], 0.6, 1e-13, "slaAoppa", "7", status );
   vvd ( aoprms[8], 0.45, 1e-13, "slaAoppa", "8", status );
   vvd ( aoprms[9], 0.006, 1e-15, "slaAoppa", "9", status );
   vvd ( aoprms[10], 0.00015628012292619523, 1e-13, "slaAoppa", "10", status )
;
   vvd ( aoprms[11], -1.7922951075221043e-7, 1e-13, "slaAoppa", "11", status )
;
   vvd ( aoprms[12], 2.101874231495843, 1e-13, "slaAoppa", "12", status );
   vvd ( aoprms[13], 7.601916802079765, 1e-8, "slaAoppa", "13", status );

   slaOap ( "r", 1.6, -1.01, date, dut, elongm, phim, hm, xp, yp, tdk,
             pmb, rh, wl, tlr, &rap, &dap );
   vvd ( rap, 1.6011975673841539, 1e-10, "slaOap", "Rr", status );
   vvd ( dap, -1.0125285613817139, 1e-10, "slaOap", "Rd", status );

   slaOap ( "h", -1.234, 2.34, date, dut, elongm, phim, hm, xp, yp, tdk,
             pmb, rh, wl, tlr, &rap, &dap );
   vvd ( rap, 5.6930876899727245, 1e-10, "slaOap", "Hr", status );
   vvd ( dap, 0.80102811756140901, 1e-10, "slaOap", "Hd", status );

   slaOap ( "a", 6.1, 1.1, date, dut, elongm, phim, hm, xp, yp, tdk,
             pmb, rh, wl, tlr, &rap, &dap );
   vvd ( rap, 5.8943051776716944, 1e-10, "slaOap", "Ar", status );
   vvd ( dap, 1.406150707974922, 1e-10, "slaOap", "Ad", status );

   slaOapqk ( "r", 2.1, -0.345, aoprms, &rap, &dap );
   vvd ( rap, 2.100239627436018, 1e-10, "slaOapqk", "Rr", status );
   vvd ( dap, -0.34524286896037198, 1e-10, "slaOapqk", "Rd", status );

   slaOapqk ( "h", -0.01, 1.03, aoprms, &rap, &dap );
   vvd ( rap, 1.328731933587408, 1e-10, "slaOapqk", "Hr", status );
   vvd ( dap, 1.0300915385247269, 1e-10, "slaOapqk", "Hd", status );

   slaOapqk ( "a", 4.321, 0.987, aoprms, &rap, &dap );
   vvd ( rap, 0.43755071148409375, 1e-10, "slaOapqk", "Ar", status );
   vvd ( dap, -0.015208984621380744, 1e-10, "slaOapqk", "Ad", status );

   slaAoppat ( date + DS2R, aoprms );
   vvd ( aoprms[13], 7.602374979243502, 1e-8, "slaAoppat", "", status );
}
void t_bear ( int *status )

{
   double a1, b1, a2, b2;
   float f1[3], f2[3];
   double d1[3], d2[3];
   int i;


   a1 = 1.234;
   b1 = -0.123;
   a2 = 2.345;
   b2 = 0.789;
   vvd ( (double) slaBear ( (float) a1, (float) b1,
                            (float) a2, (float) b2 ),
         0.7045970341781791, 1e-6, "slaBear", "", status );
   vvd ( slaDbear ( a1, b1, a2, b2 ),
         0.7045970341781791, 1e-12, "slaDbear", "", status );
   slaDcs2c ( a1, b1, d1 );
   slaDcs2c ( a2, b2, d2 );
   for ( i = 0; i < 3; i++ ) {
      f1 [ i ] = (float) d1 [ i ];
      f2 [ i ] = (float) d2 [ i ];
   }
   vvd ( (double) slaPav ( f1, f2 ),
         0.7045970341781791, 1e-6, "slaPav", "", status );
   vvd ( slaDpav ( d1, d2 ),
         0.7045970341781791, 1e-12, "slaDpav", "", status );
}
void t_c2i ( int *status )

{
   double rc, dc, pr, pd, px, rv, date, dx, dy, ri, di, rc2, dc2, eo,
          ri2, di2;
   CIpars ciprms;

   rc = 5.432;
   dc = -1.234,
   pr = 4e-6;
   pd = -3e-6;
   px = 0.5;
   rv = -50.0;
   date = 45999.0;
   dx = 1e-9;
   dy = 2e-9;

   slaC2i ( rc, dc, pr, pd, px, rv, date, dx, dy, &ri, &di );
   vvd ( ri,  5.428752350856835385, 1e-12, "slaC2i", "r", status );
   vvd ( di, -1.235040355470759854, 1e-12, "slaC2i", "d", status );

   slaI2c ( ri, di, date, dx, dy, &rc2, &dc2 );
   vvd ( rc2, 5.43193226010882757, 1e-12, "slaI2c", "r", status );
   vvd ( dc2, -1.233955033408528079, 1e-12, "slaI2c", "d", status );

   slaC2ipa ( date, dx, dy, &ciprms, &eo );
   slaC2ipad ( &ciprms );
   slaC2iqkz ( rc2, dc2, &ciprms, &ri2, &di2 );
   vvd ( ri2,  5.431957246881433221, 1e-12, "slaC2iqkz", "r", status );
   vvd ( di2, -1.234032869713102221, 1e-12, "slaC2iqkz", "d", status );
}
void t_caf2r ( int *status )

{
   float r;
   double dr;
   int j;

   slaCaf2r ( 76, 54, 32.1f, &r, &j );
   vvd ( (double) r, 1.342313819975276, 1e-6, "slaCaf2r", "r", status );
   viv ( j, 0, "slaCaf2r", "j", status );
   slaDaf2r ( 76, 54, 32.1, &dr, &j );
   vvd ( dr, 1.342313819975276, 1e-12, "slaDaf2r", "r", status );
   viv ( j, 0, "slaCaf2r", "j", status );
}
void t_caldj ( int *status )

{
   double djm;
   int j;

   slaCaldj ( 1999, 12, 31, &djm, &j );
   vvd ( djm, 51543.0, 0.0, "slaCaldj", "", status );
}
void t_calyd ( int *status )

{
   int ny, nd, j;

   slaCalyd ( 46, 4, 30, &ny, &nd, &j );
   viv ( ny, 2046, "slaCalyd", "y", status );
   viv ( nd, 120, "slaCalyd", "d", status );
   viv ( j, 0, "slaCalyd", "j", status );
   slaClyd ( -5000, 1, 1, &ny, &nd, & j);
   viv ( j, 1, "slaClyd", "illegal year", status );
   slaClyd ( 1900, 0, 1, &ny, &nd, & j);
   viv ( j, 2, "slaClyd", "illegal month", status );
   slaClyd ( 1900, 2, 29, &ny, &nd, & j);
   viv ( ny, 1900, "slaClyd", "illegal day (y)", status );
   viv ( nd, 61, "slaClyd", "illegal day (d)", status );
   viv ( j, 3, "slaClyd", "illegal day (j)", status );
   slaClyd ( 2000, 2, 29, &ny, &nd, & j);
   viv ( ny, 2000, "slaClyd", "y", status );
   viv ( nd, 60, "slaClyd", "d", status );
   viv ( j, 0, "slaClyd", "j", status );
}
void t_cc2s ( int *status )

{

   float v[3] = { 100.0f, -50.0f, 25.0f }, a, b;
   double dv[3] = { 100.0, -50.0, 25.0 }, da, db;

   slaCc2s ( v, &a, &b );
   vvd ( (double) a, -0.4636476090008061, 1e-6, "slaCc2s", "a", status );
   vvd ( (double) b, 0.2199879773954594, 1e-6, "slaCc2s", "b", status );

   slaDcc2s ( dv, &da, &db );
   vvd ( da, -0.4636476090008061, 1e-12, "slaDcc2s", "a", status );
   vvd ( db, 0.2199879773954594, 1e-12, "slaDcc2s", "b", status );
}
void t_cc62s ( int *status )

{
   float v[6] = { 100.0f, -50.0f, 25.0f, -0.1f, 0.2f, 0.7f },
         a, b, r, ad, bd, rd;
   double dv[6] = { 100.0, -50.0, 25.0, -0.1, 0.2, 0.7 },
         da, db, dr, dad, dbd, drd;

   slaCc62s ( v, &a, &b, &r, &ad, &bd, &rd );
   vvd ( (double) a, -0.4636476090008061, 1e-6, "slaCc62s", "a", status );
   vvd ( (double) b, 0.2199879773954594, 1e-6, "slaCc62s", "b", status );
   vvd ( (double) r, 114.564392373896, 1e-3, "slaCc62s", "r", status );
   vvd ( (double) ad, 0.001200000000000000, 1e-9, "slaCc62s", "ad", status );
   vvd ( (double) bd, 0.006303582107999407, 1e-8, "slaCc62s", "bd", status );
   vvd ( (double) rd, -0.02182178902359925, 1e-7, "slaCc62s", "rd", status );

   slaDc62s ( dv, &da, &db, &dr, &dad, &dbd, &drd );
   vvd ( da, -0.4636476090008061, 1e-6, "slaDc62s", "a", status );
   vvd ( db, 0.2199879773954594, 1e-6, "slaDc62s", "b", status );
   vvd ( dr, 114.564392373896, 1e-9, "slaDc62s", "r", status );
   vvd ( dad, 0.001200000000000000, 1e-15, "slaDc62s", "ad", status );
   vvd ( dbd, 0.006303582107999407, 1e-14, "slaDc62s", "bd", status );
   vvd ( drd, -0.02182178902359925, 1e-13, "slaDc62s", "rd", status );
}
void t_cd2tf ( int *status )

{
   char s;
   int ihmsf[4];

   slaCd2tf ( 4, -0.987654321f, &s, ihmsf );
   viv ( (int) s, (int) '-', "slaCd2tf", "s", status );
   viv ( ihmsf[0], 23, "slaCd2tf", "[0]", status );
   viv ( ihmsf[1], 42, "slaCd2tf", "[1]", status );
   viv ( ihmsf[2], 13, "slaCd2tf", "[2]", status );
   vvd ( (double) ihmsf[3], 3333.0, 1000.0, "slaCd2tf", "[3]", status );

   slaDd2tf ( 4, -0.987654321, &s, ihmsf );
   viv ( (int) s, (int) '-', "slaDd2tf", "s", status );
   viv ( ihmsf[0], 23, "slaDd2tf", "[0]", status );
   viv ( ihmsf[1], 42, "slaDd2tf", "[1]", status );
   viv ( ihmsf[2], 13, "slaDd2tf", "[2]", status );
   viv ( ihmsf[3], 3333, "slaDd2tf", "[3]", status );
}
void t_cldj ( int *status )

{
   double d;
   int j;

   slaCldj ( 1899, 12, 31, &d, &j );
   vvd ( d, 15019.0, 0.0, "slaCldj", "d", status );
   viv ( j, 0, "slaCldj", "j", status );
}
void t_cr2af ( int *status )

{
   char s;
   int idmsf[4];

   slaCr2af ( 4, 2.345f, &s, idmsf );
   viv ( (int) s, (int) '+', "slaCr2af", "s", status );
   viv ( idmsf[0], 134, "slaCr2af", "[0]", status );
   viv ( idmsf[1], 21, "slaCr2af", "[1]", status );
   viv ( idmsf[2], 30, "slaCr2af", "[2]", status );
   vvd ( (double) idmsf[3], 9706.0, 1000.0, "slaCr2af", "[3]", status );

   slaDr2af ( 4, 2.345, &s, idmsf );
   viv ( (int) s, (int) '+', "slaDr2af", "s", status );
   viv ( idmsf[0], 134, "slaDr2af", "[0]", status );
   viv ( idmsf[1], 21, "slaDr2af", "[1]", status );
   viv ( idmsf[2], 30, "slaDr2af", "[2]", status );
   viv ( idmsf[3], 9706, "slaDr2af", "[3]", status );
}
void t_cr2tf ( int *status )

{
   char s;
   int ihmsf[4];

   slaCr2tf ( 4, -3.01234f, &s, ihmsf );
   viv ( (int) s, (int) '-', "slaCr2tf", "s", status );
   viv ( ihmsf[0], 11, "slaCr2tf", "[0]", status );
   viv ( ihmsf[1], 30, "slaCr2tf", "[1]", status );
   viv ( ihmsf[2], 22, "slaCr2tf", "[2]", status );
   vvd ( (double) ihmsf[3], 6484.0, 1000.0, "slaCr2tf", "[3]", status );

   slaDr2tf ( 4, -3.01234, &s, ihmsf );
   viv ( (int) s, (int) '-', "slaDr2tf", "s", status );
   viv ( ihmsf[0], 11, "slaDr2tf", "[0]", status );
   viv ( ihmsf[1], 30, "slaDr2tf", "[1]", status );
   viv ( ihmsf[2], 22, "slaDr2tf", "[2]", status );
   viv ( ihmsf[3], 6484, "slaDr2tf", "[3]", status );
}
void t_cs2c6 ( int *status )

{
   float v[6];
   double dv[6];

   slaCs2c6 ( -3.21f, 0.123f, 0.456f, -7.8e-6f, 9.01e-6f, -1.23e-5f, v );
   vvd ( (double) v[0], -0.4514964673880165,
                                    1e-6, "slaCs2c6", "x", status );
   vvd ( (double) v[1],  0.03093394277342585,
                                    1e-6, "slaCs2c6", "y", status );
   vvd ( (double) v[2],  0.05594668105108779,
                                    1e-6, "slaCs2c6", "z", status );
   vvd ( (double) v[3],  1.292270850663260e-5,
                                    1e-6, "slaCs2c6", "xd", status );
   vvd ( (double) v[4],  2.652814182060692e-6,
                                    1e-6, "slaCs2c6", "yd", status );
   vvd ( (double) v[5],  2.568431853930293e-6,
                                    1e-6, "slaCs2c6", "zd", status );

   slaDs2c6 ( -3.21, 0.123, 0.456, -7.8e-6, 9.01e-6, -1.23e-5, dv );
   vvd ( dv[0], -0.4514964673880165, 1e-12, "slaDs2c6", "x", status );
   vvd ( dv[1],  0.03093394277342585, 1e-12, "slaDs2c6", "y", status );
   vvd ( dv[2],  0.05594668105108779, 1e-12, "slaDs2c6", "z", status );
   vvd ( dv[3],  1.292270850663260e-5, 1e-12, "slaDs2c6", "xd", status );
   vvd ( dv[4],  2.652814182060692e-6, 1e-12, "slaDs2c6", "yd", status );
   vvd ( dv[5],  2.568431853930293e-6, 1e-12, "slaDs2c6", "zd", status );
}
void t_ctf2d ( int *status )

{
   float d;
   double dd;
   int j;

   slaCtf2d (23, 56, 59.1f, &d, &j);
   vvd ( (double) d, 0.99790625, 1e-6, "slaCtf2d", "d", status );
   viv ( j, 0, "slaCtf2d", "j", status );

   slaDtf2d (23, 56, 59.1, &dd, &j);
   vvd ( dd, 0.99790625, 1e-12, "slaDtf2d", "d", status );
   viv ( j, 0, "slaDtf2d", "j", status );
}
void t_ctf2r ( int *status )

{
   float r;
   double dr;
   int j;

   slaCtf2r (23, 56, 59.1f, &r, &j);
   vvd ( (double) r, 6.270029887942679, 1e-6, "slaCtf2r", "r", status );
   viv ( j, 0, "slaCtf2r", "j", status );

   slaDtf2r (23, 56, 59.1, &dr, &j);
   vvd ( dr, 6.270029887942679, 1e-12, "slaDtf2r", "r", status );
   viv ( j, 0, "slaDtf2r", "j", status );
}
void t_dat ( int *status )

{
   vvd ( slaDat ( 43900.0 ), 18.0, 0.0, "slaDat", "", status );
   vvd ( slaDtt ( 40404.0 ), 39.709746, 1e-12, "slaDtt", "", status );
   vvd ( slaDt ( 500.0 ), 4686.7, 1e-10, "slaDt", "500", status );
   vvd ( slaDt ( 1400.0 ), 408.0, 1e-11, "slaDt", "1400", status );
   vvd ( slaDt ( 1950.0 ), 27.99145626, 1e-12, "slaDt", "1950", status );
}
void t_dbjin ( int *status )

{
   char s[] = "  B1950, , J 2000, b1975 JE     ";
   double d;
   int i, ja, jb;

   i = 1;
   d = 0.0;

   slaDbjin ( s, &i, &d, &ja, &jb );
   viv ( i, 9, "slaDbjin", "i1", status );
   vvd ( d, 1950.0, 0.0, "slaDbjin", "d1", status );
   viv ( ja, 0, "slaDbjin", "ja1", status );
   viv ( jb, 1, "slaDbjin", "jb1", status );

   slaDbjin ( s, &i, &d, &ja, &jb );
   viv ( i, 11, "slaDbjin", "i2", status );
   vvd ( d, 1950.0, 0.0, "slaDbjin", "d2", status );
   viv ( ja, 1, "slaDbjin", "ja2", status );
   viv ( jb, 0, "slaDbjin", "jb2", status );

   slaDbjin ( s, &i, &d, &ja, &jb );
   viv ( i, 19, "slaDbjin", "i3", status );
   vvd ( d, 2000.0, 0.0, "slaDbjin", "d3", status );
   viv ( ja, 0, "slaDbjin", "ja3", status );
   viv ( jb, 2, "slaDbjin", "jb3", status );

   slaDbjin ( s, &i, &d, &ja, &jb );
   viv ( i, 26, "slaDbjin", "i4", status );
   vvd ( d, 1975.0, 0.0, "slaDbjin", "d4", status );
   viv ( ja, 0, "slaDbjin", "ja4", status );
   viv ( jb, 1, "slaDbjin", "jb4", status );

   slaDbjin ( s, &i, &d, &ja, &jb );
   viv ( i, 26, "slaDbjin", "i5", status );
   vvd ( d, 1975.0, 0.0, "slaDbjin", "d5", status );
   viv ( ja, 1, "slaDbjin", "ja5", status );
   viv ( jb, 0, "slaDbjin", "jb5", status );
}
void t_djcal ( int *status )

{
   double djm = 50123.9999;
   int iydmf[4], j, iy, im, id;
   double f;

   slaDjcal ( 4, djm, iydmf, &j );
   viv ( iydmf[0], 1996, "slaDjcal", "y", status );
   viv ( iydmf[1], 2, "slaDjcal", "m", status );
   viv ( iydmf[2], 10, "slaDjcal", "d", status );
   viv ( iydmf[3], 9999, "slaDjcal", "f", status );
   viv ( j, 0, "slaDjcal", "j", status );

   slaDjcl ( djm, &iy, &im, &id, &f, &j );
   viv ( iy, 1996, "slaDjcl", "y", status );
   viv ( im, 2, "slaDjcl", "m", status );
   viv ( id, 10, "slaDjcl", "d", status );
   vvd ( f, 0.9999, 1e-7, "slaDjcl", "f", status );
   viv ( j, 0, "slaDjcl", "j", status );
}
void t_dmat ( int *status )

{
   double da[3][3] = {
      { 2.22,     1.6578,     1.380522     },
      { 1.6578,   1.380522,   1.22548578   },
      { 1.380522, 1.22548578, 1.1356276122 }
   };
   double dv[3] = {  2.28625, 1.7128825, 1.429432225 };
   double dd;
   int j, iw[3];

   slaDmat ( 3, (double *) da, dv, &dd, &j, iw );

   vvd ( da[0][0], 18.02550629769198,
                             1e-10, "slaDmat", "a[0][0]", status );
   vvd ( da[0][1], -52.16386644917481,
                             1e-10, "slaDmat", "a[0][1]", status );
   vvd ( da[0][2], 34.37875949717994,
                             1e-10, "slaDmat", "a[0][2]", status );
   vvd ( da[1][0], -52.16386644917477,
                             1e-10, "slaDmat", "a[1][0]", status );
   vvd ( da[1][1], 168.1778099099869,
                             1e-10, "slaDmat", "a[1][1]", status );
   vvd ( da[1][2], -118.0722869694278,
                             1e-10, "slaDmat", "a[1][2]", status );
   vvd ( da[2][0], 34.37875949717988,
                             1e-10, "slaDmat", "a[2][0]", status );
   vvd ( da[2][1], -118.07228696942770,
                             1e-10, "slaDmat", "a[2][1]", status );
   vvd ( da[2][2], 86.50307003740468,
                             1e-10, "slaDmat", "a[2][2]", status );
   vvd ( dv[0], 1.002346480763383,
                             1e-12, "slaDmat", "v[0]", status );
   vvd ( dv[1], 0.0328559401697292,
                             1e-12, "slaDmat", "v[1]", status );
   vvd ( dv[2], 0.004760688414898454,
                             1e-12, "slaDmat", "v[2]", status );
   vvd ( dd, 0.003658344147359863,
                             1e-12, "slaDmat", "d", status );
   viv ( j, 0, "slaDmat", "j", status );
}
void t_e2h ( int *status )

{
   double dh, dd, dp, da, de;
   float h, d, p, a, e;

   dh = -0.3;
   dd = -1.1;
   dp = -0.7;

   h = (float) dh;
   d = (float) dd;
   p = (float) dp;

   slaDe2h ( dh, dd, dp, &da, &de );
   vvd ( da, 2.820087515852369, 1e-12, "slaDe2h", "az", status );
   vvd ( de, 1.132711866443304, 1e-12, "slaDe2h", "el", status );

   slaE2h ( h, d, p, &a, &e );
   vvd ( (double) a, 2.820087515852369, 1e-6, "slaE2h", "az", status );
   vvd ( (double) e, 1.132711866443304, 1e-6, "slaE2h", "el", status );

   slaDh2e ( da, de, dp, &dh, &dd );
   vvd ( dh, -0.3, 1e-12, "slaDh2e", "HA", status );
   vvd ( dd, -1.1, 1e-12, "slaDh2e", "dec", status );

   slaH2e ( a, e, p, &h, &d );
   vvd ( (double) h, -0.3, 1e-6, "slaH2e", "HA", status );
   vvd ( (double) d, -1.1, 1e-6, "slaH2e", "dec", status );
}
void t_earth ( int *status )

{
   float pv[6];

   slaEarth ( 1978, 174, 0.87f, pv );

   vvd ( (double) pv[0], 3.590843e-2, 1e-6, "slaEarth", "pv[0]", status );
   vvd ( (double) pv[1], -9.319286e-1, 1e-6, "slaEarth", "pv[1]", status );
   vvd ( (double) pv[2], -4.041040e-1, 1e-6, "slaEarth", "pv[2]", status );
   vvd ( (double) pv[3], 1.956930e-7, 1e-13, "slaEarth", "pv[3]", status );
   vvd ( (double) pv[4], 5.743768e-9, 1e-13, "slaEarth", "pv[4]", status );
   vvd ( (double) pv[5], 2.511983e-9, 1e-13, "slaEarth", "pv[5]", status );
}
void t_ecleq ( int *status )

{
   double r, d;

   slaEcleq ( 1.234, -0.123, 43210.0, &r, &d );

   vvd ( r, 1.229910118208851, 1e-12, "slaEcleq", "RA", status );
   vvd ( d, 0.2638461400411088, 1e-12, "slaEcleq", "Dec", status );
}
void t_ecmat ( int *status )

{
   double rm[3][3];

   slaEcmat ( 41234.0, rm );

   vvd ( rm[0][0], 1.0, 1e-12, "slaEcmat", "[0][0]", status );
   vvd ( rm[0][1], 0.0, 1e-12, "slaEcmat", "[0][1]", status );
   vvd ( rm[0][2], 0.0, 1e-12, "slaEcmat", "[0][2]", status );
   vvd ( rm[1][0], 0.0, 1e-12, "slaEcmat", "[1][0]", status );
   vvd ( rm[1][1], 0.917456575085716, 1e-12,
                               "slaEcmat", "[1][1]", status );
   vvd ( rm[1][2], 0.397835937079581, 1e-12,
                               "slaEcmat", "[1][2]", status );
   vvd ( rm[2][0], 0.0, 1e-12, "slaEcmat", "[2][0]", status );
   vvd ( rm[2][1], -0.397835937079581, 1e-12,
                               "slaEcmat", "[2][1]", status );
   vvd ( rm[2][2], 0.917456575085716, 1e-12,
                               "slaEcmat", "[2][2]", status );
}
void t_ecor ( int *status )

{
   float rv, tl;

   slaEcor ( 2.345f, -0.567f, 1995, 306, 0.037f, &rv, &tl );

   vvd ( (double) rv, -19.182460, 1e-4, "slaEcor", "rv", status );
   vvd ( (double) tl, -120.366300, 1e-3, "slaEcor", "tl", status );
}
void t_eg50 ( int *status )

{
   double dl, db;

   slaEg50 ( 3.012, 1.234, &dl, &db );

   vvd ( dl, 2.305557953813397, 1e-12, "slaEg50", "l", status );
   vvd ( db, 0.7903600886585871, 1e-12, "slaEg50", "b", status );
}
void t_epb ( int *status )

{
   vvd ( slaEpb ( 45123.0 ), 1982.419793168669, 1e-8,
                                           "slaEpb", "", status );
}
void t_epb2d ( int *status )

{
   vvd ( slaEpb2d ( 1975.5 ), 42595.5995279655, 1e-7,
                                           "slaEpb2d", "", status );
}
void t_epco ( int *status )

{
   vvd ( slaEpco ( 'b', 'J', 2000.0 ), 2000.001277513665, 1e-7,
                                           "slaEpco", "bJ", status );
   vvd ( slaEpco ( 'J', 'b', 1950.0 ), 1949.999790442300, 1e-7,
                                           "slaEpco", "Jb", status );
   vvd ( slaEpco ( 'j', 'J', 2000.0 ), 2000.0, 1e-7,
                                           "slaEpco", "jJ", status );
}
void t_epj ( int *status )

{
   vvd ( slaEpj ( 42999.0 ), 1976.603696098563, 1e-7,
                                           "slaEpj", "", status );
}
void t_epj2d ( int *status )

{
   vvd ( slaEpj2d ( 2010.077 ), 55225.124250, 1e-6,
                                           "slaEpj2d", "", status );
}
void t_eqecl ( int *status )

{
   double dl, db;

   slaEqecl ( 0.789, -0.123, 46555.0, &dl, &db );

   vvd ( dl, 0.7036566430349022, 1e-12, "slaEqecl", "l", status );
   vvd ( db, -0.4036047164116848, 1e-12, "slaEqecl", "b", status );
}
void t_eqeqx ( int *status )

{
   vvd ( slaEqeqx ( 41234.0 ), 5.376047445838220e-5, 1e-17,
                                        "slaEqeqx", "", status );
}
void t_eqgal ( int *status )

{
   double dl, db;

   slaEqgal ( 5.67, -1.23, &dl, &db );

   vvd ( dl, 5.6122707809298245, 1e-12, "slaEqgal", "dl", status );
   vvd ( db, -0.68005214489796961, 1e-12, "slaEqgal", "db", status );
}
void t_etrms ( int *status )

{
   double ev[3];

   slaEtrms ( 1976.9, ev );

   vvd ( ev[0], -1.621617102537041e-6, 1e-18, "slaEtrms", "x", status );
   vvd ( ev[1], -3.310070088507914e-7, 1e-18, "slaEtrms", "y", status );
   vvd ( ev[2], -1.435296627515719e-7, 1e-18, "slaEtrms", "z", status );
}
void t_evp ( int *status )

{
   double dvb[3], dpb[3], dvh[3], dph[3];

   slaEvp ( 50100.0, 1990.0, dvb, dpb, dvh, dph );

   vvd ( dvb[0], -1.807210068602723e-7, 1e-16, "slaEvp", "dvb[x]", status );
   vvd ( dvb[1], -8.385891022440320e-8, 1e-16, "slaEvp", "dvb[y]", status );
   vvd ( dvb[2], -3.635846882638055e-8, 1e-16, "slaEvp", "dvb[z]", status );
   vvd ( dpb[0], -0.4515615297360333, 1e-8, "slaEvp", "dpb[x]", status );
   vvd ( dpb[1],  0.8103788166239596, 1e-8, "slaEvp", "dpb[y]", status );
   vvd ( dpb[2],  0.3514505204144827, 1e-8, "slaEvp", "dpb[z]", status );
   vvd ( dvh[0], -1.806354061155555e-7, 1e-16, "slaEvp", "dvh[x]", status );
   vvd ( dvh[1], -8.383798678086174e-8, 1e-16, "slaEvp", "dvh[y]", status );
   vvd ( dvh[2], -3.635185843644782e-8, 1e-16, "slaEvp", "dvh[z]", status );
   vvd ( dph[0], -0.4478571659918565, 1e-8, "slaEvp", "dph[x]", status );
   vvd ( dph[1],  0.8036439916076232, 1e-8, "slaEvp", "dph[y]", status );
   vvd ( dph[2],  0.3484298459102053, 1e-8, "slaEvp", "dph[z]", status );

   slaEpv ( 53411.52501161, dph, dvh, dpb, dvb );

   vvd ( dph[0], -0.7757238809297653, 1e-12, "slaEpv", "dph[x]", status );
   vvd ( dph[1], 0.5598052241363390, 1e-12, "slaEpv", "dph[y]", status );
   vvd ( dph[2], 0.2426998466481708, 1e-12, "slaEpv", "dph[z]", status );
   vvd ( dvh[0], -0.0109189182414732, 1e-12, "slaEpv", "dvh[x]", status );
   vvd ( dvh[1], -0.0124718726844084, 1e-12, "slaEpv", "dvh[y]", status );
   vvd ( dvh[2], -0.0054075694180650, 1e-12, "slaEpv", "dvh[z]", status );
   vvd ( dpb[0], -0.7714104440491060, 1e-12, "slaEpv", "dpb[x]", status );
   vvd ( dpb[1], 0.5598412061824225, 1e-12, "slaEpv", "dpb[y]", status );
   vvd ( dpb[2], 0.2425996277722475, 1e-12, "slaEpv", "dpb[z]", status );
   vvd ( dvb[0], -0.0109187426811683, 1e-12, "slaEpv", "dvb[x]", status );
   vvd ( dvb[1], -0.0124652546173285, 1e-12, "slaEpv", "dvb[y]", status );
   vvd ( dvb[2], -0.0054047731809662, 1e-12, "slaEpv", "dvb[z]", status );
}
void t_fitxy ( int *status )

{
#define NPTS (8)
   double xye[NPTS][2] = {
      { -23.4, -12.1 },
      {  32.0, -15.3 },
      {  10.9,  23.7 },
      {  -3.0,  16.1 },
      {  45.0,  32.5 },
      {   8.6, -17.0 },
      {  15.3,  10.0 },
      { 121.7,  -3.8 }
   };
   double xym[NPTS][2] = {
      { -23.41,  12.12 },
      {  32.03,  15.34 },
      {  10.93, -23.72 },
      {  -3.01, -16.10 },
      {  44.90, -32.46 },
      {   8.55,  17.02 },
      {  15.31, -10.07 },
      { 120.92,   3.81 }
   };
   double coeffs[6], xyp[NPTS][2], xrms, yrms, rrms, bkwds[6],
          x2, y2, xz, yz, xs, ys, perp, orient;
   int j;


   slaFitxy ( 4, NPTS, xye, xym, coeffs, &j );
   vvd ( coeffs[0], -7.938263381515947e-3,
                            1e-12, "slaFitxy", "4/0", status );
   vvd ( coeffs[1], 1.004640925187200,
                            1e-12, "slaFitxy", "4/1", status );
   vvd ( coeffs[2], 3.976948048238268e-4,
                            1e-12, "slaFitxy", "4/2", status );
   vvd ( coeffs[3], -2.501031681585021e-2,
                            1e-12, "slaFitxy", "4/3", status );
   vvd ( coeffs[4], 3.976948048238268e-4,
                            1e-12, "slaFitxy", "4/4", status );
   vvd ( coeffs[5], -1.004640925187200,
                            1e-12, "slaFitxy", "4/5", status );
   viv ( j, 0, "slaFitxy", "4/j", status );


   slaFitxy ( 6, NPTS, xye, xym, coeffs, &j );
   vvd ( coeffs[0], -2.617232551841476e-2,
                            1e-12, "slaFitxy", "6/0", status );
   vvd ( coeffs[1], 1.005634905041421,
                            1e-12, "slaFitxy", "6/1", status );
   vvd ( coeffs[2], 2.133045023329208e-3,
                            1e-12, "slaFitxy", "6/2", status );
   vvd ( coeffs[3], 3.846993364431164e-3,
                            1e-12, "slaFitxy", "6/3", status );
   vvd ( coeffs[4], 1.301671386431460e-4,
                            1e-12, "slaFitxy", "6/4", status );
   vvd ( coeffs[5], -0.9994827065693964,
                            1e-12, "slaFitxy", "6/5", status );
   viv ( j, 0, "slaFitxy", "6/j", status );


   slaPxy ( NPTS, xye, xym, coeffs, xyp, &xrms, &yrms, &rrms );
   vvd ( xyp[0][0], -23.542232946855340, 1e-12, "slaPxy", "x0", status );
   vvd ( xyp[0][1], -12.112930622972290, 1e-12, "slaPxy", "y0", status );
   vvd ( xyp[1][0], 32.217034593616180, 1e-12, "slaPxy", "x1", status );
   vvd ( xyp[1][1], -15.324048471959370, 1e-12, "slaPxy", "y1", status );
   vvd ( xyp[2][0], 10.914821358630950, 1e-12, "slaPxy", "x2", status );
   vvd ( xyp[2][1], 23.712999520015880, 1e-12, "slaPxy", "y2", status );
   vvd ( xyp[3][0], -3.087475414568693, 1e-12, "slaPxy", "x3", status );
   vvd ( xyp[3][1], 16.095126766044400, 1e-12, "slaPxy", "y3", status );
   vvd ( xyp[4][0], 45.057596269384130, 1e-12, "slaPxy", "x4", status );
   vvd ( xyp[4][1], 32.452900153132120, 1e-12, "slaPxy", "y4", status );
   vvd ( xyp[5][0], 8.608310538882801, 1e-12, "slaPxy", "x5", status );
   vvd ( xyp[5][1], -17.006235743411300, 1e-12, "slaPxy", "y5", status );
   vvd ( xyp[6][0], 15.348618307280820, 1e-12, "slaPxy", "x6", status );
   vvd ( xyp[6][1], 10.070630707410880, 1e-12, "slaPxy", "y6", status );
   vvd ( xyp[7][0], 121.583327293629100, 1e-12, "slaPxy", "x7", status );
   vvd ( xyp[7][1], -3.788442308260240, 1e-12, "slaPxy", "y7", status );
   vvd ( xrms ,0.1087247110488075, 1e-13, "slaPxy", "xrms", status );
   vvd ( yrms, 0.03224481175794666, 1e-13, "slaPxy", "yrms", status );
   vvd ( rrms, 0.1134054261398109, 1e-13, "slaPxy", "rrms", status );


   slaInvf ( coeffs, bkwds, &j );
   vvd ( bkwds[0], 0.02601750208015891, 1e-12, "slaInvf", "0", status);
   vvd ( bkwds[1], 0.9943963945040283, 1e-12, "slaInvf", "1", status);
   vvd ( bkwds[2], 0.002122190075497872, 1e-12, "slaInvf", "2", status);
   vvd ( bkwds[3], 0.003852372795370861, 1e-12, "slaInvf", "3", status);
   vvd ( bkwds[4], 0.0001295047252932767, 1e-12, "slaInvf", "4", status);
   vvd ( bkwds[5], -1.000517284779212, 1e-12, "slaInvf", "5", status);
   viv ( j, 0, "slaInvf", "j", status );


   slaXy2xy ( 44.5, 32.5, coeffs, &x2, &y2 );
   vvd ( x2, 44.793904912083030, 1e-11, "slaXy2xy", "x", status);
   vvd ( y2, -32.473548532471330, 1e-11, "slaXy2xy", "y", status);


   slaDcmpf ( coeffs, &xz, &yz, &xs, &ys, &perp, &orient );
   vvd ( xz, -0.02601750208015891, 1e-12, "slaDcmpf", "xz", status);
   vvd ( yz, -0.003852372795371087, 1e-12, "slaDcmpf", "yz", status);
   vvd ( xs, -1.005634913465693, 1e-12, "slaDcmpf", "xs", status);
   vvd ( ys, 0.9994849826847614, 1e-12, "slaDcmpf", "ys", status);
   vvd ( perp, -0.002004707996156263, 1e-12, "slaDcmpf", "p", status);
   vvd ( orient, 3.140460861823333, 1e-12, "slaDcmpf", "o", status);
}
void t_fk425 ( int *status )

{
   double r2000, d2000, dr2000, dd2000, p2000, v2000;

   slaFk425 ( 1.234, -0.123, -1e-5, 2e-6, 0.5, 20.0,
              &r2000, &d2000, &dr2000, &dd2000, &p2000, &v2000 );

   vvd ( r2000, 1.244117554618727, 1e-12, "slaFk425", "r", status );
   vvd ( d2000, -0.1213164254458709, 1e-12, "slaFk425", "d", status );
   vvd ( dr2000, -9.964265838268711e-6, 1e-17, "slaFk425", "dr", status );
   vvd ( dd2000, 2.038065265773541e-6, 1e-17, "slaFk425", "dd", status );
   vvd ( p2000, 0.4997443812415410, 1e-12, "slaFk425", "p", status );
   vvd ( v2000, 20.010460915421010, 1e-11, "slaFk425", "v", status );
}
void t_fk45z ( int *status )

{
   double r2000, d2000;

   slaFk45z ( 1.234, -0.123, 1984.0, &r2000, &d2000 );

   vvd ( r2000, 1.244616510731691, 1e-12, "slaFk45z", "r", status );
   vvd ( d2000, -0.1214185839586555, 1e-12, "slaFk45z", "d", status );
}
void t_fk524 ( int *status )

{
   double r1950, d1950, dr1950, dd1950, p1950, v1950;

   slaFk524 ( 4.567, -1.23, -3e-5, 8e-6, 0.29, -35.0,
               &r1950, &d1950, &dr1950, &dd1950, &p1950, &v1950 );

   vvd ( r1950, 4.543778603272084, 1e-12, "slaFk524", "r", status );
   vvd ( d1950, -1.229642790187574, 1e-12, "slaFk524", "d", status );
   vvd ( dr1950, -2.957873121769244e-5, 1e-17, "slaFk524", "dr", status );
   vvd ( dd1950, 8.117725309659079e-6, 1e-17, "slaFk524", "dd", status );
   vvd ( p1950, 0.2898494999992917, 1e-12, "slaFk524", "p", status );
   vvd ( v1950, -35.026862824252680, 1e-11, "slaFk524", "v", status );
}
void t_fk52h ( int *status )

{
   double r5, d5, dr5, dd5, rh, dh, drh, ddh;

   slaFk52h ( 1.234, -0.987, 1e-6, -2e-6, &rh, &dh, &drh, &ddh );
   vvd ( rh, 1.234000000272114, 1e-13, "slaFk52h", "r", status );
   vvd ( dh, -0.986999923521850, 1e-13, "slaFk52h", "d", status );
   vvd ( drh, 0.000000993178295, 1e-13, "slaFk52h", "dr", status );
   vvd ( ddh, -0.000001997665915, 1e-13, "slaFk52h", "dd", status );
   slaH2fk5 ( rh, dh, drh, ddh, &r5, &d5, &dr5, &dd5 );
   vvd ( r5, 1.234, 1e-13, "slaH2fk5", "r", status );
   vvd ( d5, -0.987, 1e-13, "slaH2fk5", "d", status );
   vvd ( dr5, 1e-6, 1e-13, "slaH2fk5", "dr", status );
   vvd ( dd5, -2e-6, 1e-13, "slaH2fk5", "dd", status );
   slaFk5hz ( 1.234, -0.987, 1980.0, &rh, &dh );
   vvd ( rh, 1.234000136713603, 1e-13, "slaFk5hz", "r", status );
   vvd ( dh, -0.986999970202077, 1e-13, "slaFk5hz", "d", status );
   slaHfk5z ( rh, dh, 1980.0, &r5, &d5, &dr5, &dd5 );
   vvd ( r5, 1.234, 1e-13, "slaHfk5z", "r", status );
   vvd ( d5, -0.987, 1e-13, "slaHfk5z", "d", status );
   vvd ( dr5, 0.000000006822074, 1e-13, "slaHfk5z", "dr", status );
   vvd ( dd5, -0.000000002334012, 1e-13, "slaHfk5z", "dd", status );
}
void t_fk54z ( int *status )

{
   double r1950, d1950, dr1950, dd1950;

   slaFk54z ( 0.001, -1.55, 1900.0, &r1950, &d1950, &dr1950, &dd1950 );

   vvd ( r1950, 6.271585543439484, 1e-12, "slaFk54z", "r", status );
   vvd ( d1950, -1.554861715330319, 1e-12, "slaFk54z", "d", status );
   vvd ( dr1950, -4.175410876044272e-8, 1e-20, "slaFk54z", "dr", status );
   vvd ( dd1950, 2.118595098308522e-8, 1e-20, "slaFk54z", "dd", status );
}
void t_flotin ( int *status )

{
   char s[] = "  12.345, , -0 1e3-4 2000  E     ";
   float fv;
   double dv;
   int i, j;

   i = 1;
   fv = 0.0f;

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 10, "slaFlotin", "i1", status );
   vvd ( (double) fv, 12.345, 1e-6, "slaFlotin", "v1", status );
   viv ( j, 0, "slaFlotin", "j1", status );

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 12, "slaFlotin", "i2", status );
   vvd ( (double) fv, 12.345, 1e-6, "slaFlotin", "v2", status );
   viv ( j, 1, "slaFlotin", "j2", status );

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 16, "slaFlotin", "i3", status );
   vvd ( (double) fv, 0.0, 0.0, "slaFlotin", "v3", status );
   viv ( j, -1, "slaFlotin", "j3", status );

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 19, "slaFlotin", "i4", status );
   vvd ( (double) fv, 1000.0, 0.0, "slaFlotin", "v4", status );
   viv ( j, 0, "slaFlotin", "j4", status );

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 22, "slaFlotin", "i5", status );
   vvd ( (double) fv, -4.0, 0.0, "slaFlotin", "v5", status );
   viv ( j, -1, "slaFlotin", "j5", status );

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 28, "slaFlotin", "i6", status );
   vvd ( (double) fv, 2000.0, 0.0, "slaFlotin", "v6", status );
   viv ( j, 0, "slaFlotin", "j6", status );

   slaFlotin ( s, &i, &fv, &j );
   viv ( i, 34, "slaFlotin", "i7", status );
   vvd ( (double) fv, 2000.0, 0.0, "slaFlotin", "v7", status );
   viv ( j, 2, "slaFlotin", "j7", status );

   i = 1;
   dv = 0.0f;

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 10, "slaDfltin", "i1", status );
   vvd ( dv, 12.345, 1e-12, "slaDfltin", "v1", status );
   viv ( j, 0, "slaDfltin", "j1", status );

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 12, "slaDfltin", "i2", status );
   vvd ( dv, 12.345, 1e-12, "slaDfltin", "v2", status );
   viv ( j, 1, "slaDfltin", "j2", status );

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 16, "slaDfltin", "i3", status );
   vvd ( dv, 0.0, 0.0, "slaDfltin", "v3", status );
   viv ( j, -1, "slaDfltin", "j3", status );

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 19, "slaDfltin", "i4", status );
   vvd ( dv, 1000.0, 0.0, "slaDfltin", "v4", status );
   viv ( j, 0, "slaDfltin", "j4", status );

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 22, "slaDfltin", "i5", status );
   vvd ( dv, -4.0, 0.0, "slaDfltin", "v5", status );
   viv ( j, -1, "slaDfltin", "j5", status );

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 28, "slaDfltin", "i6", status );
   vvd ( dv, 2000.0, 0.0, "slaDfltin", "v6", status );
   viv ( j, 0, "slaDfltin", "j6", status );

   slaDfltin ( s, &i, &dv, &j );
   viv ( i, 34, "slaDfltin", "i7", status );
   vvd ( dv, 2000.0, 0.0, "slaDfltin", "v7", status );
   viv ( j, 2, "slaDfltin", "j7", status );
}
void t_galeq ( int *status )

{
   double dr, dd;

   slaGaleq ( 5.67, -1.23, &dr, &dd );

   vvd ( dr, 0.047292704199962211, 1e-12, "slaGaleq", "dr", status );
   vvd ( dd, -0.78340036669057256, 1e-12, "slaGaleq", "dd", status );
}
void t_galsup ( int *status )

{
   double dsl, dsb;

   slaGalsup ( 6.1, -1.4, &dsl, &dsb );

   vvd ( dsl, 4.567933268859171, 1e-12, "slaGalsup", "dsl", status );
   vvd ( dsb, -0.01862369899731829, 1e-12, "slaGalsup", "dsb", status );
}
void t_ge50 ( int *status )

{
   double dr, dd;

   slaGe50 ( 6.1, -1.55, &dr, &dd );

   vvd ( dr, 0.1966825219934508, 1e-12, "slaGe50", "dr", status );
   vvd ( dd, -0.4924752701678960, 1e-12, "slaGe50", "dd", status );
}
void t_gst ( int *status )

{
   vvd ( slaGmst ( 43999.999 ), 3.9074971356487318,
                                1e-9, "slaGmst", "", status );
   vvd ( slaGmsta ( 43999.0, 0.999 ), 3.9074971356487318,
                                1e-12, "slaGmsta", "", status );
   vvd ( slaGst ( 53750.892855138888889,
                  53750.0, 0.892104561342593),
                  1.332428947958645749,
                                1e-12, "slaGst", "", status );
}
void t_i2o ( int *status )

{
   double ri, di, utc, dut, elong, phi, hm, xp, yp, tk, phpa, rh,
          wlfq, tlr, aob, zob, hob, dob, rob, ri2, di2;
   IOpars ioprms;


   ri = 5.432;
   di = -1.234;
   utc = 45432.0;
   dut = 0.5;
   elong = -2.345;
   phi = 0.9;
   hm = 2000.0;
   xp = -3e-6;
   yp = 4e-6;
   tk = 280.0;
   phpa = 950.0;
   rh = 0.7;
   wlfq = -200.0;
   tlr = 0.005;

   slaI2o ( ri, di, utc, dut, elong, phi, hm, xp, yp, tk, phpa, rh,
            wlfq, tlr, &aob, &zob, &hob, &dob, &rob );
   vvd ( aob, -2.583855450814341648, 1e-12, "slaI2o", "a", status );
   vvd ( zob,  2.500919293883643757, 1e-12, "slaI2o", "z", status );
   vvd ( hob,  1.880038550622425753, 1e-12, "slaI2o", "h", status );
   vvd ( dob, -1.232250157820589109, 1e-12, "slaI2o", "d", status );
   vvd ( rob,  5.477379344780665349, 1e-12, "slaI2o", "r", status );

   slaO2i ( "A", aob, zob, utc, dut, elong, phi, hm, xp, yp, tk,
            phpa, rh, wlfq, tlr, &ri2, &di2 );
   vvd ( ri2, ri, 1e-12, "slaO2i", "a/r", status );
   vvd ( di2, di, 1e-12, "slaO2i", "a/d", status );
   slaO2i ( "H", hob, dob, utc, dut, elong, phi, hm, xp, yp, tk,
            phpa, rh, wlfq, tlr, &ri2, &di2 );
   vvd ( ri2, ri, 1e-12, "slaO2i", "h/r", status );
   vvd ( di2, di, 1e-12, "slaO2i", "h/d", status );
   slaO2i ( "R", rob, dob, utc, dut, elong, phi, hm, xp, yp, tk,
            phpa, rh, wlfq, tlr, &ri2, &di2 );
   vvd ( ri2, ri, 1e-12, "slaO2i", "r/r", status );
   vvd ( di2, di, 1e-12, "slaO2i", "r/d", status );

   slaI2opa ( utc, dut, elong, phi, hm, xp, yp, tk, phpa, rh,
              wlfq, tlr, &ioprms );
   slaI2opad ( &ioprms );
   slaI2oqk (  ri, di, &ioprms, &aob, &zob, &hob, &dob, &rob );
   vvd ( aob, -2.583854050088510768, 1e-12,
                                        "slaI2opad", "a", status );
   vvd ( zob,  2.500918879136150075, 1e-12,
                                        "slaI2opad", "z", status );
   vvd ( hob,  1.880037658186337612, 1e-12,
                                        "slaI2opad", "h", status );
   vvd ( dob, -1.232249271718325767, 1e-12,
                                        "slaI2opad", "d", status );
   vvd ( rob,  5.477380237216753045, 1e-12,
                                        "slaI2opad", "r", status );
}
void t_intin ( int *status )

{
   char s[] = "  -12345, , -0  2000  +     ";
   long n;
   int i, j, k;

   i = 1;
   n = 0;

   slaIntin ( s, &i, &n, &j );
   viv ( i, 10, "slaIntin", "i1", status );
   vlv ( n, -12345L, "slaIntin", "v1", status );
   viv ( j, -1, "slaIntin", "j1", status );

   slaIntin ( s, &i, &n, &j );
   viv ( i, 12, "slaIntin", "i2", status );
   vlv ( n, -12345L, "slaIntin", "v2", status );
   viv ( j, 1, "slaIntin", "j2", status );

   slaIntin ( s, &i, &n, &j );
   viv ( i, 17, "slaIntin", "i3", status );
   vlv ( n, 0L, "slaIntin", "v3", status );
   viv ( j, -1, "slaIntin", "j3", status );

   slaIntin ( s, &i, &n, &j );
   viv ( i, 23, "slaIntin", "i4", status );
   vlv ( n, 2000L, "slaIntin", "v4", status );
   viv ( j, 0, "slaIntin", "j4", status );

   slaIntin ( s, &i, &n, &j );
   viv ( i, 29, "slaIntin", "i5", status );
   vlv ( n, 2000L, "slaIntin", "v5", status );
   viv ( j, 2, "slaIntin", "j5", status );

   i = 1;
   k = 0;

   slaInt2in ( s, &i, &k, &j );
   viv ( i, 10, "slaInt2in", "i", status );
   viv ( k, -12345, "slaInt2in", "v", status );
   viv ( j, -1, "slaInt2in", "j", status );
}
void t_kbj ( int *status )

{
   double e;
   char k[] = "?";
   int j;

   e = 1950.0;
   slaKbj ( -1, e, k, &j );
   vcs ( k, " ", "slaKbj", "jb1", status );
   viv ( j, 1, "slaKbj", "j1", status );
   slaKbj ( 0, e, k, &j );
   vcs ( k, "B", "slaKbj", "jb2", status );
   viv ( j, 0, "slaKbj", "j2", status );
   slaKbj ( 1, e, k, &j );
   vcs ( k, "B", "slaKbj", "jb3", status );
   viv ( j, 0, "slaKbj", "j3", status );
   slaKbj ( 2, e, k, &j );
   vcs ( k, "J", "slaKbj", "jb4", status );
   viv ( j, 0, "slaKbj", "j4", status );
   slaKbj ( 3, e, k, &j );
   vcs ( k, " ", "slaKbj", "jb5", status );
   viv ( j, 1, "slaKbj", "j5", status );
   e = 2000.0;
   slaKbj ( 0, e, k, &j );
   vcs ( k, "J", "slaKbj", "jb6", status );
   viv ( j, 0, "slaKbj", "j6", status );
   slaKbj ( 1, e, k, &j );
   vcs ( k, "B", "slaKbj", "jb7", status );
   viv ( j, 0, "slaKbj", "j7", status );
   slaKbj ( 2, e, k, &j );
   vcs ( k, "J", "slaKbj", "jb8", status );
   viv ( j, 0, "slaKbj", "j8", status );
}
void t_map ( int *status )

{
   double ra, da, amprms[21];

   slaMap ( 6.123, -0.999, 1.23e-5, -0.987e-5,
                         0.123, 32.1, 1999.0, 43210.9, &ra, &da );

   vvd ( ra, 6.117130429775647, 1e-12, "slaMap", "ra", status );
   vvd ( da, -1.000880769038632, 1e-12, "slaMap", "da", status );

   slaMappa ( 2020.0, 45012.3, amprms );

   vvd ( amprms[0], -37.884188911704310,
                        1e-11, "slaMappa", "amprms[0]", status );
   vvd ( amprms[1],  -0.7888341859486424,
                        1e-8, "slaMappa", "amprms[1]", status );
   vvd ( amprms[2],   0.5405321789059870,
                        1e-8, "slaMappa", "amprms[2]", status );
   vvd ( amprms[3],   0.2340784267119091,
                        1e-8, "slaMappa", "amprms[3]", status );
   vvd ( amprms[4],  -0.8067807553151404,
                        1e-9, "slaMappa", "amprms[4]", status );
   vvd ( amprms[5],   0.5420884771335385,
                        1e-9, "slaMappa", "amprms[5]", status );
   vvd ( amprms[6],   0.2350423277032729,
                        1e-9, "slaMappa", "amprms[6]", status );
   vvd ( amprms[7],   1.999729472165140e-8,
                        1e-19, "slaMappa", "amprms[7]", status );
   vvd ( amprms[8],  -6.035531043781443e-5,
                        1e-13, "slaMappa", "amprms[8]", status );
   vvd ( amprms[9],  -7.381891582517198e-5,
                        1e-13, "slaMappa", "amprms[9]", status );
   vvd ( amprms[10], -3.200897749854970e-5,
                        1e-13, "slaMappa", "amprms[10]", status );
   vvd ( amprms[11],  0.9999999949417148,
                        1e-12, "slaMappa", "amprms[11]", status );
   vvd ( amprms[12],  0.9999566751478850,
                        1e-12, "slaMappa", "amprms[12]", status );
   vvd ( amprms[13],  8.537308717676752e-3,
                        1e-12, "slaMappa", "amprms[13]", status );
   vvd ( amprms[14],  3.709742180572510e-3,
                        1e-12, "slaMappa", "amprms[14]", status );
   vvd ( amprms[15], -8.537361890149777e-3,
                        1e-12, "slaMappa", "amprms[15]", status );
   vvd ( amprms[16],  0.9999635560607690,
                        1e-12, "slaMappa", "amprms[16]", status );
   vvd ( amprms[17], -1.502613373498668e-6,
                        1e-12, "slaMappa", "amprms[17]", status );
   vvd ( amprms[18], -3.709619811228171e-3,
                        1e-12, "slaMappa", "amprms[18]", status );
   vvd ( amprms[19], -3.016886324169151e-5,
                        1e-12, "slaMappa", "amprms[19]", status );
   vvd ( amprms[20],  0.9999931188816729,
                        1e-12, "slaMappa", "amprms[20]", status );

   slaMapqk ( 1.234, -0.987, -1.2e-5, -0.99, 0.75, -23.4, amprms,
                        &ra, &da );

   vvd ( ra, 1.223337584930993, 1e-12, "slaMapqk", "ra", status );
   vvd ( da, 0.5558838650379129, 1e-12, "slaMapqk", "da", status );

   slaMapqkz ( 6.012, 1.234, amprms, &ra, &da );

   vvd ( ra, 6.006091119756597, 1e-12, "slaMapqkz", "ra", status );
   vvd ( da, 1.23045846622498, 1e-12, "slaMapqkz", "da", status );
}
void t_moon ( int *status )

{
   float pv[6];

   slaMoon ( 1999, 365, 0.9f, pv );

   vvd ( (double) pv[0], -2.155729505970773e-3, 1e-6,
                                        "slaMoon", "[0]", status );
   vvd ( (double) pv[1], -1.538107758633427e-3, 1e-6,
                                        "slaMoon", "[1]", status );
   vvd ( (double) pv[2], -4.003940552689305e-4, 1e-6 ,
                                        "slaMoon", "[2]", status );
   vvd ( (double) pv[3],  3.629209419071314e-9, 1e-12,
                                        "slaMoon", "[3]", status );
   vvd ( (double) pv[4], -4.989667166259157e-9, 1e-12,
                                        "slaMoon", "[4]", status );
   vvd ( (double) pv[5], -2.160752457288307e-9, 1e-12,
                                        "slaMoon", "[5]", status );
}
void t_nut ( int *status )

{
   double rmatn[3][3], dpsi, deps, eps0;

   slaNut ( 46012.34, rmatn );

   vvd ( rmatn[0][0],  9.999999969492166e-1, 1e-12,
                                 "slaNut", "[0][0]", status );
   vvd ( rmatn[0][1],  7.166577986249302e-5, 1e-12,
                                 "slaNut", "[0][1]", status );
   vvd ( rmatn[0][2],  3.107382973077677e-5, 1e-12,
                                 "slaNut", "[0][2]", status );
   vvd ( rmatn[1][0], -7.166503970900504e-5, 1e-12,
                                 "slaNut", "[1][0]", status );
   vvd ( rmatn[1][1],  9.999999971483732e-1, 1e-12,
                                 "slaNut", "[1][1]", status );
   vvd ( rmatn[1][2], -2.381965032461830e-5, 1e-12,
                                 "slaNut", "[1][2]", status );
   vvd ( rmatn[2][0], -3.107553669598237e-5, 1e-12,
                                 "slaNut", "[2][0]", status );
   vvd ( rmatn[2][1],  2.381742334472628e-5, 1e-12,
                                 "slaNut", "[2][1]", status );
   vvd ( rmatn[2][2],  9.999999992335207e-1, 1e-12,
                                 "slaNut", "[2][2]", status );

   slaNutc ( 50123.4, &dpsi, &deps, &eps0 );

   vvd ( dpsi, 3.523550954747942e-5, 1e-17, "slaNutc", "dpsi", status );
   vvd ( deps, -4.143371566683342e-5, 1e-17, "slaNutc", "deps", status );
   vvd ( eps0, 0.4091014592901651, 1e-12, "slaNutc", "eps0", status );

   slaNutc80 ( 50123.4, &dpsi, &deps, &eps0 );

   vvd ( dpsi, 3.537714281665895e-5, 1e-17, "slaNutc80", "dpsi", status );
   vvd ( deps, -4.140590085987164e-5, 1e-17, "slaNutc80", "deps", status );
   vvd ( eps0, 0.4091016349007751, 1e-12, "slaNutc", "eps0", status );
}
void t_obs ( int *status )

{
   int n;
   char c[10], name[40];
   double w, p, h;

   n = 0;
   strcpy ( c, "MMT" );
   slaObs ( n, c, name, &w, &p, &h );
   vcs ( c, "MMT", "slaObs", "1/c", status );
   vcs ( name, "MMT 6.5m, Mt Hopkins", "slaObs", "1/name", status );
   vvd ( w, 1.935300584055477, 1e-8, "slaObs", "1/w", status );
   vvd ( p, 0.5530735081568836, 1e-10, "slaObs", "1/p", status );
   vvd ( h, 2608.0, 1e-10, "slaObs", "1/h", status );

   n = 61;
   slaObs ( n, c, name, &w, &p, &h );
   vcs ( c, "KECK1", "slaObs", "2/c", status );
   vcs ( name, "Keck 10m Telescope #1", "slaObs", "2/name", status );
   vvd ( w, 2.713545757918895, 1e-8, "slaObs", "2/w", status );
   vvd ( p, 0.3460280563536619, 1e-8, "slaObs", "2/p", status );
   vvd ( h, 4160.0, 1e-10, "slaObs", "2/h", status );

   n = 84;
   slaObs ( n, c, name, &w, &p, &h );
   vcs ( c, "HET", "slaObs", "3/c", status );
   vcs ( name, "Hobby-Eberly Telescope",
                                        "slaObs", "3/name", status );
   vvd ( w, 1.8153993733250908, 1e-8, "slaObs", "3/w", status );
   vvd ( p, 0.53549222482327796, 1e-8, "slaObs", "3/p", status );
   vvd ( h, 2026.0, 1e-10, "slaObs", "3/h", status );

   n = 85;
   slaObs ( n, c, name, &w, &p, &h );
   vcs ( name, "?", "slaObs", "4/name", status );
}
void t_pa ( int *status )

{
   vvd ( slaPa ( -1.567, 1.5123, 0.987 ), -1.486288540423851,
                    1e-12, "slaPa", "", status );
   vvd ( slaPa ( 0.0, 0.789, 0.789 ), 0.0,
                    0.0, "slaPa", "zenith", status );
}
void t_pcd ( int *status )

{
   double disco = 178.585, x, y;

   x = 0.0123;
   y = -0.00987;
   slaPcd ( disco, &x, &y );
   vvd ( x, 0.01284630845735895, 1e-14, "slaPcd", "x", status );
   vvd ( y, -0.01030837922553926, 1e-14, "slaPcd", "y", status );

   slaUnpcd ( disco, &x, &y );
   vvd (x, 0.0123, 1e-14, "slaUnpcd", "x", status );
   vvd ( y, -0.00987, 1e-14, "slaUnpcd", "y", status );
}
void t_pda2h ( int *status )

{
   double h1, h2;
   int j1, j2;

   slaPda2h ( -0.51, -1.31, 3.1, &h1, &j1, &h2, &j2 );
   vvd ( h1, -0.1161784556585303, 1e-14, "slaPda2h", "h1", status );
   viv ( j1, 0, "slaPda2h", "j1", status );
   vvd ( h2, -2.984787179226459, 1e-13, "slaPda2h", "h2", status );
   viv ( j2, 0, "slaPda2h", "j2", status );
}
void t_pdq2h ( int *status )

{
   double h1, h2;
   int j1, j2;

   slaPdq2h ( 0.9, 0.2, 0.1, &h1, &j1, &h2, &j2 );
   vvd ( h1, 0.1042809894435257, 1e-14, "slaPdq2h", "h1", status );
   viv ( j1, 0, "slaPdq2h", "j1", status );
   vvd ( h2, 2.997450098818439, 1e-13, "slaPdq2h", "h2", status );
   viv ( j2, 0, "slaPdq2h", "j2", status );
}
void t_percom ( int *status )

{
   int list[3], i, j, istate[4], iorder[4];


   list[0] = 0;
   for ( i = 0; i < 11; i++ ) {
      slaCombn ( 3, 5, list, &j );
   }
   viv ( j, 1, "slaCombn", "j", status );
   viv ( list[0], 1, "slaCombn", "list[0]", status );
   viv ( list[1], 2, "slaCombn", "list[1]", status );
   viv ( list[2], 3, "slaCombn", "list[2]", status );

   istate[0] = -1;
   for ( i = 0; i < 25; i++ ) {
      slaPermut ( 4, istate, iorder, &j );
   }
   viv ( j, 1, "slaPermut", "j", status );
   viv ( iorder[0], 4, "slaPermut", "iorder[0]", status );
   viv ( iorder[1], 3, "slaPermut", "iorder[1]", status );
   viv ( iorder[2], 2, "slaPermut", "iorder[2]", status );
   viv ( iorder[3], 1, "slaPermut", "iorder[3]", status );
}
void t_planet ( int *status )

{
   double u[13], pv[6], ra, dec, r, diam, epoch, orbinc, anode, perih,
          aorq, e, aorl, dm;
   int j, jform;


   slaEl2ue ( 50000.0, 1, 49000.0, 0.1, 2.0, 0.2, 3.0, 0.05, 3, 0.003312,
               u, &j );
   vvd ( u[0], 1.000878908362435, 1e-12, "slaEl2ue", "u[0]", status );
   vvd ( u[1], -0.3336263027874777, 1e-12, "slaEl2ue", "u[1]", status );
   vvd ( u[2], 50000.0, 1e-12, "slaEl2ue", "u[2]", status );
   vvd ( u[3], 2.840425801310305, 1e-12, "slaEl2ue", "u[3]", status );
   vvd ( u[4], 0.1264380368034784, 1e-12, "slaEl2ue", "u[4]", status );
   vvd ( u[5], -0.2287711835229231, 1e-12, "slaEl2ue", "u[5]", status );
   vvd ( u[6], -0.01301062595105583, 1e-12, "slaEl2ue", "u[6]", status );
   vvd ( u[7], 0.5657102158104654, 1e-12, "slaEl2ue", "u[7]", status );
   vvd ( u[8], 0.2189745287281790, 1e-12, "slaEl2ue", "u[8]", status );
   vvd ( u[9], 2.852427310959998, 1e-12, "slaEl2ue", "u[9]", status );
   vvd ( u[10], -0.01552349065434892, 1e-12, "slaEl2ue", "u[10]", status );
   vvd ( u[11], 50000.0, 1e-12, "slaEl2ue", "u[11]", status );
   vvd ( u[12], 0.0, 1e-12, "slaEl2ue", "u[12]", status );
   viv ( j, 0, "slaEl2ue", "j", status );

   slaPertel ( 2, 43000.0, 43200.0,
               43000.0, 0.2, 3.0, 4.0, 5.0, 0.02, 6.0,
               &epoch, &orbinc, &anode, &perih, &aorq, &e, &aorl, &j );
   vvd ( epoch, 43200.0, 1e-10, "slaPv2el", "epoch", status );
   vvd ( orbinc, 0.1995661466545422, 1e-12, "slaPertel", "orbinc", status );
   vvd ( anode, 2.998052737821591, 1e-12, "slaPertel", "anode", status );
   vvd ( perih, 4.0095164484411363, 1e-12, "slaPertel", "perih", status );
   vvd ( aorq, 5.014216294790208, 1e-12, "slaPertel", "aorq", status );
   vvd ( e, 0.02281386258296773, 1e-12, "slaPertel", "e", status );
   vvd ( aorl, 0.017352486487806705, 1e-12, "slaPertel", "aorl", status );
   viv ( j, 0, "slaPertel", "j", status );

   slaPertue ( 50100.0, u, &j );
   vvd ( u[0], 1.000000000000000, 1e-12, "slaPertue", "u[0]", status );
   vvd ( u[1], -0.3329769417028022, 1e-12, "slaPertue", "u[1]", status );
   vvd ( u[2], 50100.0, 1e-12, "slaPertue", "u[2]", status );
   vvd ( u[3], 2.638884303608535, 1e-12, "slaPertue", "u[3]", status );
   vvd ( u[4], 1.070994304747803, 1e-12, "slaPertue", "u[4]", status );
   vvd ( u[5], 0.1544112080167478, 1e-12, "slaPertue", "u[5]", status );
   vvd ( u[6], -0.2188240619161382, 1e-12, "slaPertue", "u[6]", status );
   vvd ( u[7], 0.5207557453451925, 1e-12, "slaPertue", "u[7]", status );
   vvd ( u[8], 0.2217782439275219, 1e-12, "slaPertue", "u[8]", status );
   vvd ( u[9], 2.852118859689218, 1e-12, "slaPertue", "u[9]", status );
   vvd ( u[10], 0.01452010174372095, 1e-12, "slaPertue", "u[10]", status );
   vvd ( u[11], 50100.0, 1e-12, "slaPertue", "[11]", status );
   vvd ( u[12], 0.0, 1e-12, "slaPertue", "[12]", status );
   viv ( j, 0, "slaPertue", "j", status );

   slaPlanel ( 50600.0, 2, 50500.0, 0.1, 3.0, 5.0, 2.0, 0.3, 4.0, 0.0,
               pv, &j );
   vvd ( pv[0], 1.9476289592889056, 1e-12, "slaPlanel", "pv[0]", status );
   vvd ( pv[1], -1.0137360587522046, 1e-12, "slaPlanel", "pv[1]", status );
   vvd ( pv[2], -0.3536409947735744, 1e-12, "slaPlanel", "pv[2]", status );
   vvd ( pv[3], 2.742247411576664e-8, 1e-19, "slaPlanel", "pv[3]", status );
   vvd ( pv[4], 1.170467244078822e-7, 1e-19, "slaPlanel", "pv[4]", status );
   vvd ( pv[5], 3.709878268216680e-8, 1e-19, "slaPlanel", "pv[5]", status );
   viv ( j, 0, "slaPlanel", "j", status );

   slaPlanet ( 1e6, 0, pv, &j );
   vvd ( pv[0], 0.0, 0.0, "slaPlanet", "pv[0] 1", status );
   vvd ( pv[1], 0.0, 0.0, "slaPlanet", "pv[1] 1", status );
   vvd ( pv[2], 0.0, 0.0, "slaPlanet", "pv[2] 1", status );
   vvd ( pv[3], 0.0, 0.0, "slaPlanet", "pv[3] 1", status );
   vvd ( pv[4], 0.0, 0.0, "slaPlanet", "pv[4] 1", status );
   vvd ( pv[5], 0.0, 0.0, "slaPlanet", "pv[5] 1", status );
   viv ( j, -1, "slaPlanet", "j 1", status );

   slaPlanet ( 1e6, 9, pv, &j );
   viv ( j, -1, "slaPlanet", "j 2", status );

   slaPlanet ( -320000.0, 3, pv, &j );
   vvd ( pv[0], 0.9308038666826181, 1e-11, "slaPlanet", "pv[0] 3", status );
   vvd ( pv[1], 0.3258319040254594, 1e-11, "slaPlanet", "pv[1] 3", status );
   vvd ( pv[2], 0.1422794544478194, 1e-11, "slaPlanet", "pv[2] 3", status );
   vvd ( pv[3], -7.441503423894762e-8, 1e-18, "slaPlanet", "pv[3] 3", status )
;
   vvd ( pv[4], 1.699734557528462e-7, 1e-18, "slaPlanet", "pv[4] 3", status );
   vvd ( pv[5], 7.415505123000807e-8, 1e-18, "slaPlanet", "pv[5] 3", status );
   viv ( j, 1, "slaPlanet", "j 3", status );

   slaPlanet ( 43999.9, 1, pv, &j );
   vvd ( pv[0], 0.2945293959257864, 1e-11, "slaPlanet", "pv[0] 4", status );
   vvd ( pv[1], -0.2452204176600444, 1e-11, "slaPlanet", "pv[1] 4", status );
   vvd ( pv[2], -0.1615427700571700, 1e-11, "slaPlanet", "pv[2] 4", status );
   vvd ( pv[3], 1.636421147458610e-7, 1e-18, "slaPlanet", "pv[3] 4", status );
   vvd ( pv[4], 2.252949422575255e-7, 1e-18, "slaPlanet", "pv[4] 4", status );
   vvd ( pv[5], 1.033542799062612e-7, 1e-18, "slaPlanet", "pv[5] 4", status );
   viv ( j, 0, "slaPlanet", "j 4", status );

   slaPlante ( 50600.0, -1.23, 0.456, 2, 50500.0, 0.1, 3.0,
               5.0, 2.0, 0.3, 4.0, 0.0, &ra, &dec, &r, &j );
   vvd ( ra, 6.222958101333312619, 1e-10, "slaPlante", "RA", status );
   vvd ( dec, 0.0114222030572741183, 1e-10, "slaPlante", "dec", status );
   vvd ( r, 2.288902494078670191, 1e-8, "slaPlante", "r", status );
   viv ( j, 0, "slaPlante", "j", status );

   u[0] = 1.0005;
   u[1] = -0.3;
   u[2] = 55000.0;
   u[3] = 2.8;
   u[4] = 0.1;
   u[5] = -0.2;
   u[6] = -0.01;
   u[7] = 0.5;
   u[8] = 0.22;
   u[9] = 2.8;
   u[10] = -0.015;
   u[11] = 55001.0;
   u[12] = 0.0;
   slaPlantu ( 55001.0, -1.23, 0.456, u, &ra, &dec, &r, &j );
   vvd ( ra, 0.3531814831241686381, 1e-10, "slaPlantu", "RA", status );
   vvd ( dec, 0.06940344580567135713, 1e-10, "slaPlantu", "dec", status );
   vvd ( r, 3.031687170873276216, 1e-8, "slaPlantu", "r", status );
   viv ( j, 0, "slaPlantu", "j", status );

   pv[0] = 0.3;
   pv[1] = -0.2;
   pv[2] = 0.1;
   pv[3] = -0.9e-7;
   pv[4] = 0.8e-7;
   pv[5] = -0.7e-7;
   slaPv2el ( pv, 50000.0, 0.00006, 1,
              &jform, &epoch, &orbinc, &anode, &perih,
              &aorq, &e, &aorl, &dm, &j );
   viv ( jform, 1, "slaPv2el", "jform", status );
   vvd ( epoch, 50000.0, 1e-10, "slaPv2el", "epoch", status );
   vvd ( orbinc, 1.52099895268912, 1e-12, "slaPv2el", "orbinc", status );
   vvd ( anode, 2.720503180538650, 1e-12, "slaPv2el", "anode", status );
   vvd ( perih, 2.194081512031836, 1e-12, "slaPv2el", "perih", status );
   vvd ( aorq, 0.2059371035373771, 1e-12, "slaPv2el", "aorq", status );
   vvd ( e, 0.9866822985810528, 1e-12, "slaPv2el", "e", status );
   vvd ( aorl, 0.2012758344836794, 1e-12, "slaPv2el", "aorl", status );
   vvd ( dm, 0.1840740507951820, 1e-12, "slaPv2el", "dm", status );
   viv ( j, 0, "slaPv2el", "j", status );

   slaPv2ue ( pv, 50000.0, 0.00006, u, &j );
   vvd ( u[0], 1.00006, 1e-12, "slaPv2ue", "u[0]", status );
   vvd ( u[1], -4.856142884511782, 1e-12, "slaPv2ue", "u[1]", status );
   vvd ( u[2], 50000.0, 1e-12, "slaPv2ue", "u[2]", status );
   vvd ( u[3], 0.3, 1e-12, "slaPv2ue", "u[3]", status );
   vvd ( u[4], -0.2, 1e-12, "slaPv2ue", "u[4]", status );
   vvd ( u[5], 0.1, 1e-12, "slaPv2ue", "u[5]", status );
   vvd ( u[6], -0.4520378601821727, 1e-12, "slaPv2ue", "u[6]", status );
   vvd ( u[7], 0.4018114312730424, 1e-12, "slaPv2ue", "u[7]", status );
   vvd ( u[8], -.3515850023639121, 1e-12, "slaPv2ue", "u[8]", status );
   vvd ( u[9], 0.3741657386773941, 1e-12, "slaPv2ue", "u[9]", status );
   vvd ( u[10], -0.2511321445456515, 1e-12, "slaPv2ue", "u[10]", status );
   vvd ( u[11], 50000.0, 1e-12, "slaPv2ue", "u[11]", status );
   vvd ( u[12], 0.0, 1e-12, "slaPv2ue", "u[12]", status );
   viv ( j, 0, "slaPv2ue", "j", status );

   slaRdplan ( 40999.9, 0, 0.1, -0.9, &ra, &dec, &diam );
   vvd ( ra, 5.772270359389343, 1e-10, "slaRdplan", "RA 0", status );
   vvd ( dec, -0.2089207338795175, 1e-10, "slaRdplan", "dec 0", status );
   vvd ( diam, 9.415338935229712e-3, 1e-15, "slaRdplan", "diam 0", status );
   slaRdplan ( 41999.9, 1, 1.1, -0.9, &ra, &dec, &diam );
   vvd ( ra, 3.866363420052745, 1e-10, "slaRdplan", "RA 1", status );
   vvd ( dec, -0.2594430577549303, 1e-10, "slaRdplan", "dec 1", status );
   vvd ( diam, 4.638468996799415e-5, 1e-15, "slaRdplan", "diam 1", status );
   slaRdplan ( 42999.9, 2, 2.1, 0.9, &ra, &dec, &diam );
   vvd ( ra, 2.695383203184066, 1e-10, "slaRdplan", "RA 2", status );
   vvd ( dec, 0.2124044506294852, 1e-10, "slaRdplan", "dec 2", status );
   vvd ( diam, 4.892222838694240e-5, 1e-15, "slaRdplan", "diam 2", status );
   slaRdplan ( 43999.9, 3, 3.1, 0.9, &ra, &dec, &diam );
   vvd ( ra, 2.908326678461974, 1e-10, "slaRdplan", "RA 3", status );
   vvd ( dec, 0.08729783126907663, 1e-10, "slaRdplan", "dec 3", status );
   vvd ( diam, 8.581305866035831e-3, 1e-15, "slaRdplan", "diam 3", status );
   slaRdplan ( 44999.9, 4, -0.1, 1.1, &ra, &dec, &diam );
   vvd ( ra, 3.429840787472659, 1e-10, "slaRdplan", "RA 4", status );
   vvd ( dec, -0.06979851055253126, 1e-10, "slaRdplan", "dec 4", status );
   vvd ( diam, 4.540536678443060e-5, 1e-15, "slaRdplan", "diam 4", status );
   slaRdplan ( 45999.9, 5, -1.1, 0.1, &ra, &dec, &diam );
   vvd ( ra, 4.864669466449428, 1e-10, "slaRdplan", "RA 5", status );
   vvd ( dec, -0.407771449790895, 1e-10, "slaRdplan", "dec 5", status );
   vvd ( diam, 1.727945579027964e-4, 1e-15, "slaRdplan", "diam 5", status );
   slaRdplan ( 46999.9, 6, -2.1, -0.1, &ra, &dec, &diam );
   vvd ( ra, 4.432929829176694, 1e-10, "slaRdplan", "RA 6", status );
   vvd ( dec, -0.3682820877855141, 1e-10, "slaRdplan", "dec 6", status );
   vvd ( diam, 8.670829016098516e-5, 1e-15, "slaRdplan", "diam 6", status );
   slaRdplan ( 47999.9, 7, -3.1, -1.1, &ra, &dec, &diam );
   vvd ( ra, 4.894972492286819, 1e-10, "slaRdplan", "RA 7", status );
   vvd ( dec, -0.4084068901053652, 1e-10, "slaRdplan", "dec 7", status );
   vvd ( diam, 1.793916783976053e-5, 1e-15, "slaRdplan", "diam 7", status );
   slaRdplan ( 48999.9, 8, 0.0, 0.0, &ra, &dec, &diam );
   vvd ( ra, 5.066050284760061, 1e-10, "slaRdplan", "RA 8", status );
   vvd ( dec, -0.3744690779683935, 1e-10, "slaRdplan", "dec 8", status );
   vvd ( diam, 1.062210086082802e-5, 1e-15, "slaRdplan", "diam 8", status );
   slaRdplan ( 49999.9, 9, 0.0, 0.0, &ra, &dec, &diam );
   vvd ( ra, 4.179543143097202, 1e-10, "slaRdplan", "RA 9", status );
   vvd ( dec, -0.1258021632894040, 1e-10, "slaRdplan", "dec 9", status );
   vvd ( diam, 5.034057475666013e-7, 1e-15, "slaRdplan", "diam 9", status );

   slaUe2el ( u, 1, &jform, &epoch, &orbinc, &anode, &perih,
              &aorq, &e, &aorl, &dm, &j );
   viv ( jform, 1, "slaUe2el", "jform", status );
   vvd ( epoch, 50000.00000000000 , 1e-10, "slaPv2el", "epoch", status );
   vvd ( orbinc, 1.520998952689120 , 1e-12, "slaUe2el", "orbinc", status );
   vvd ( anode, 2.720503180538650 , 1e-12, "slaUe2el", "anode", status );
   vvd ( perih, 2.194081512031836 , 1e-12, "slaUe2el", "perih", status );
   vvd ( aorq, 0.2059371035373771 , 1e-12, "slaUe2el", "aorq", status );
   vvd ( e, 0.9866822985810528 , 1e-12, "slaUe2el", "e", status );
   vvd ( aorl, 0.2012758344836794 , 1e-12, "slaUe2el", "aorl", status );
   viv ( j, 0, "slaUe2el", "j", status );

   slaUe2pv ( 50010.0, u, pv, &j );
   vvd ( u[0], 1.00006, 1e-12, "slaUe2pv", "u[0]", status );
   vvd ( u[1], -4.856142884511782, 1e-12, "slaUe2pv", "u[1]", status );
   vvd ( u[2], 50000.0, 1e-12, "slaUe2pv", "u[2]", status );
   vvd ( u[3], 0.3, 1e-12, "slaUe2pv", "u[3]", status );
   vvd ( u[4], -0.2, 1e-12, "slaUe2pv", "u[4]", status );
   vvd ( u[5], 0.1, 1e-12, "slaUe2pv", "u[5]", status );
   vvd ( u[6], -0.4520378601821727, 1e-12, "slaUe2pv", "u[6]", status );
   vvd ( u[7], 0.4018114312730424, 1e-12, "slaUe2pv", "u[7]", status );
   vvd ( u[8], -0.3515850023639121, 1e-12, "slaUe2pv", "u[8]", status );
   vvd ( u[9], 0.3741657386773941, 1e-12, "slaUe2pv", "u[9]", status );
   vvd ( u[10],-.2511321445456515, 1e-12, "slaUe2pv", "u[10]", status );
   vvd ( u[11], 50010.00000000000, 1e-12, "slaUe2pv", "u[11]", status );
   vvd ( u[12], 0.7194308220038889, 1e-12, "slaUe2pv", "u[12]", status );
   vvd ( pv[0], 0.07944764084631670, 1e-12, "slaUe2pv", "pv[0]", status );
   vvd ( pv[1], -0.04118141077419017, 1e-12, "slaUe2pv", "pv[1]", status );
   vvd ( pv[2], 0.002915180702063631, 1e-12, "slaUe2pv", "pv[2]", status );
   vvd ( pv[3], -0.6890132370721106e-6, 1e-18,"slaUe2pv", "pv[3]", status );
   vvd ( pv[4], 0.4326690733487621e-6, 1e-18, "slaUe2pv", "pv[4]", status );
   vvd ( pv[5], -0.1763249096254134e-6, 1e-18, "slaUe2pv", "pv[5]", status );
   viv ( j, 0, "slaUe2pv", "j", status );
}
void t_pm ( int *status )

{
   double r1, d1;

   slaPm ( 5.43, -0.87, -0.33e-5, 0.77e-5, 0.7, 50.3*365.2422/365.25,
           1899.0, 1943.0, &r1, &d1 );
   vvd ( r1, 5.429855087793875, 1e-12, "slaPm", "r", status );
   vvd ( d1, -0.8696617307805072, 1e-12, "slaPm", "d", status );
}
void t_pncio ( int *status )

{
   double rm[3][3];

   slaPncio ( 45987.0, rm );
   vvd ( rm[0][0],  0.9999988609056197353, 1e-12,
                                 "slaPncio", "[0][0]", status );
   vvd ( rm[0][1],  1.237557853924426473e-8, 1e-15,
                                 "slaPncio", "[0][1]", status );
   vvd ( rm[0][2],  0.001509366576708142445, 1e-15,
                                 "slaPncio", "[0][2]", status );
   vvd ( rm[1][0],  2.095982163074237902e-8, 1e-15,
                                 "slaPncio", "[1][0]", status );
   vvd ( rm[1][1],  0.999999999756111313, 1e-12,
                                 "slaPncio", "[1][1]", status );
   vvd ( rm[1][2], -2.208567276258980791e-5, 1e-15,
                                 "slaPncio", "[1][2]", status );
   vvd ( rm[2][0], -0.001509366576613348047, 1e-15,
                                 "slaPncio", "[2][0]", status );
   vvd ( rm[2][1], 2.208567924097829872e-5, 1e-15,
                                 "slaPncio", "[2][1]", status );
   vvd ( rm[2][2], 0.9999988606617310483, 1e-12,
                                 "slaPncio", "[2][2]", status );
}
void t_polmo ( int *status )

{
   double elong, phi, daz;

   slaPolmo ( 0.7, -0.5, 1e-6, -2e-6, &elong, &phi, &daz );

   vvd ( elong,  0.7000004837322044,   1e-12, "slaPolmo", "elong", status );
   vvd ( phi, -0.4999979467222241,   1e-12, "slaPolmo", "phi", status );
   vvd ( daz,  1.008982781275728e-6, 1e-12, "slaPolmo", "daz", status );
}
void t_pomom ( int *status )

{
   double rm[3][3];

   slaPomom ( 41234.0, 1e-6, 2e-6, rm );

   vvd ( rm[0][0],  0.9999999999994999555, 1e-12,
                               "slaPomom", "[0][0]", status );
   vvd ( rm[0][1],  6.432239796759380879e-11, 1e-20,
                               "slaPomom", "[0][1]", status );
   vvd ( rm[0][2],  9.99999999999833301e-7, 1e-20,
                               "slaPomom", "[0][2]", status );
   vvd ( rm[1][0], -6.232239796749900012e-11, 1e-20,
                               "slaPomom", "[1][0]", status );
   vvd ( rm[1][1],  0.9999999999980000442, 1e-12,
                               "slaPomom", "[1][1]", status );
   vvd ( rm[1][2], -1.999999999997666757e-6, 1e-20,
                               "slaPomom", "[1][2]", status );
   vvd ( rm[2][0], -1.000000000126478067e-6, 1e-20,
                               "slaPomom", "[2][0]", status );
   vvd ( rm[2][1],  1.999999999934344268e-6, 1e-20,
                               "slaPomom", "[2][1]", status );
   vvd ( rm[2][2],  0.9999999999974999998, 1e-12,
                               "slaPomom", "[2][2]", status );

}
void t_prebn ( int *status )

{
   double rmatp[3][3];

   slaPrebn ( 1925.0, 1975.0, rmatp );
   vvd ( rmatp[0][0],  9.999257613786738e-1, 1e-12,
                                 "slaPrebn", "[0][0]", status );
   vvd ( rmatp[0][1], -1.117444640880939e-2, 1e-12,
                                 "slaPrebn", "[0][1]", status );
   vvd ( rmatp[0][2], -4.858341150654265e-3, 1e-12,
                                 "slaPrebn", "[0][2]", status );
   vvd ( rmatp[1][0],  1.117444639746558e-2, 1e-12,
                                 "slaPrebn", "[1][0]", status );
   vvd ( rmatp[1][1],  9.999375635561940e-1, 1e-12,
                                 "slaPrebn", "[1][1]", status );
   vvd ( rmatp[1][2], -2.714797892626396e-5, 1e-12,
                                 "slaPrebn", "[1][2]", status );
   vvd ( rmatp[2][0],  4.858341176745641e-3, 1e-12,
                                 "slaPrebn", "[2][0]", status );
   vvd ( rmatp[2][1], -2.714330927085065e-5, 1e-12,
                                 "slaPrebn", "[2][1]", status );
   vvd ( rmatp[2][2],  9.999881978224798e-1, 1e-12,
                                 "slaPrebn", "[2][2]", status );
}
void t_prec ( int *status )

{
   double rmatp[3][3];

   slaPrec ( 1925.0, 1975.0, rmatp );
   vvd ( rmatp[0][0],  9.999257249850045e-1, 1e-12,
                                 "slaPrec", "[0][0]", status );
   vvd ( rmatp[0][1], -1.117719859160180e-2, 1e-12,
                                 "slaPrec", "[0][1]", status );
   vvd ( rmatp[0][2], -4.859500474027002e-3, 1e-12,
                                 "slaPrec", "[0][2]", status );
   vvd ( rmatp[1][0],  1.117719858025860e-2, 1e-12,
                                 "slaPrec", "[1][0]", status );
   vvd ( rmatp[1][1],  9.999375327960091e-1, 1e-12,
                                 "slaPrec", "[1][1]", status );
   vvd ( rmatp[1][2], -2.716114374174549e-5, 1e-12,
                                 "slaPrec", "[1][2]", status );
   vvd ( rmatp[2][0],  4.859500500117173e-3, 1e-12,
                                 "slaPrec", "[2][0]", status );
   vvd ( rmatp[2][1], -2.715647545167383e-5, 1e-12,
                                 "slaPrec", "[2][1]", status );
   vvd ( rmatp[2][2],  9.999881921889954e-1, 1e-12,
                                 "slaPrec", "[2][2]", status );

   slaPrecl ( 1925.0, 1975.0, rmatp );
   vvd ( rmatp[0][0],  9.999257331781050e-1, 1e-12,
                                 "slaPrec", "[0][0]", status );
   vvd ( rmatp[0][1], -1.117658038434041e-2, 1e-12,
                                 "slaPrec", "[0][1]", status );
   vvd ( rmatp[0][2], -4.859236477249598e-3, 1e-12,
                                 "slaPrec", "[0][2]", status );
   vvd ( rmatp[1][0],  1.117658037299592e-2, 1e-12,
                                 "slaPrec", "[1][0]", status );
   vvd ( rmatp[1][1],  9.999375397061558e-1, 1e-12,
                                 "slaPrec", "[1][1]", status );
   vvd ( rmatp[1][2], -2.715816653174189e-5, 1e-12,
                                 "slaPrec", "[1][2]", status );
   vvd ( rmatp[2][0],  4.859236503342703e-3, 1e-12,
                                 "slaPrec", "[2][0]", status );
   vvd ( rmatp[2][1], -2.715349745834860e-5, 1e-12,
                                 "slaPrec", "[2][1]", status );
   vvd ( rmatp[2][2],  9.999881934719490e-1, 1e-12,
                                 "slaPrec", "[2][2]", status );
}
void t_preces ( int *status )

{
   double ra, dc;

   ra = 6.28;
   dc = -1.123;
   slaPreces ( "Fk4", 1925.0, 1950.0, &ra, &dc );
   vvd ( ra,  0.002403604864728447, 1e-12, "slaPreces", "r", status );
   vvd ( dc, -1.120570643322045, 1e-12, "slaPreces", "d", status );

   ra = 0.0123;
   dc = 1.0987;
   slaPreces ( "fK5", 2050.0, 1990.0, &ra, &dc );
   vvd ( ra, 6.282003602708382, 1e-12, "slaPreces", "r", status );
   vvd ( dc, 1.092870326188383, 1e-12, "slaPreces", "d", status );
}
void t_prenut ( int *status )

{
   double rmatpn[3][3];

   slaPrenut ( 1985.0, 50123.4567, rmatpn );

   vvd ( rmatpn[0][0],  9.999962358680738e-1, 1e-12,
                                 "slaPrenut", "[0][0]", status );
   vvd ( rmatpn[0][1], -2.516417057665452e-3, 1e-12,
                                 "slaPrenut", "[0][1]", status );
   vvd ( rmatpn[0][2], -1.093569785342370e-3, 1e-12,
                                 "slaPrenut", "[0][2]", status );
   vvd ( rmatpn[1][0],  2.516462370370876e-3, 1e-12,
                                 "slaPrenut", "[1][0]", status );
   vvd ( rmatpn[1][1],  9.999968329010883e-1, 1e-12,
                                 "slaPrenut", "[1][1]", status );
   vvd ( rmatpn[1][2],  4.006159587358310e-5, 1e-12,
                                 "slaPrenut", "[1][2]", status );
   vvd ( rmatpn[2][0],  1.093465510215479e-3, 1e-12,
                                 "slaPrenut", "[2][0]", status );
   vvd ( rmatpn[2][1], -4.281337229063151e-5, 1e-12,
                                 "slaPrenut", "[2][1]", status );
   vvd ( rmatpn[2][2],  9.999994012499173e-1, 1e-12,
                                 "slaPrenut", "[2][2]", status );
}
void t_pvobs ( int *status )

{
   double pv[6];

   slaPvobs ( 0.5123, 3001.0, -0.567, pv );

   vvd ( pv[0], 0.3138647803054939e-4, 1e-16, "slaPvobs", "[0]", status );
   vvd ( pv[1],-0.1998515596527082e-4, 1e-16, "slaPvobs", "[1]", status );
   vvd ( pv[2], 0.2078572043443275e-4, 1e-16, "slaPvobs", "[2]", status );
   vvd ( pv[3], 0.1457340726851264e-8, 1e-20, "slaPvobs", "[3]", status );
   vvd ( pv[4], 0.2288738340888011e-8, 1e-20, "slaPvobs", "[4]", status );
   vvd ( pv[5], 0.0,                   0.0,   "slaPvobs", "[5]", status );
}
void t_range ( int *status )

{
   vvd ( (double) slaRange ( -4.0f ), 2.283185307179586,
                                    1e-6, "slaRange", "", status );
   vvd ( slaDrange ( -4.0 ), 2.283185307179586,
                                    1e-12, "slaDrange", "", status );
}
void t_ranorm ( int *status )

{
   vvd ( (double) slaRanorm ( -0.1f ), 6.183185307179587,
                                    1e-6, "slaRanorm", "1", status );
   vvd ( slaDranrm ( -0.1 ), 6.183185307179587,
                                    1e-12, "slaDranrm", "2", status );
}
void t_rcc ( int *status )

{
   vvd ( slaRcc ( 48939.123, 0.76543, 5.0123, 5525.242, 3190.0 ),
                   -1.280131613589158e-3, 1e-15, "slaRcc", "", status );
}
void t_ref ( int *status )

{
   double ref, refa, refb, refa2, refb2, vu[3], vr[3], zr;

   slaRefro ( 1.4, 3456.7, 280.0, 678.9, 0.9, 0.55,
               -0.3, 0.006, 1e-9, &ref );
   vvd ( ref, 0.0010671561661249461, 1e-12, "slaRefro", "o", status );

   slaRefro ( 1.4, 3456.7, 280.0, 678.9, 0.9, 1000.0,
               -0.3, 0.006, 1e-9, &ref );
   vvd ( ref, 0.0013224681803634694, 1e-12, "slaRefro", "r", status );

   slaRefcoq ( 275.9, 709.3, 0.9, -200.0, &refa, &refb );
   vvd ( refa, 4.0569420833444118e-4, 1e-12, "slaRefcoq", "a/r", status );
   vvd ( refb, -4.1591742130714425e-7, 1e-15, "slaRefcoq", "b/r", status );

   slaRefco ( 2111.1, 275.9, 709.3, 0.9, -100.0,
               -1.03, 0.0067, 1e-12, &refa, &refb );
   vvd ( refa, 2.3235009971139711e-4, 1e-12, "slaRefco", "a/r", status );
   vvd ( refb, -2.2637555590936948e-7, 1e-15, "slaRefco", "b/r", status );

   slaRefcoq ( 275.9, 709.3, 0.9, 0.77, &refa, &refb );
   vvd ( refa, 2.0074065215965877e-4, 1e-12, "slaRefcoq", "a", status );
   vvd ( refb, -2.2642100925898246e-7, 1e-15, "slaRefcoq", "b", status );

   slaRefco ( 2111.1, 275.9, 709.3, 0.9, 0.77,
               -1.03, 0.0067, 1e-12, &refa, &refb );
   vvd ( refa, 2.0072013956711393e-4, 1e-12, "slaRefco", "a", status );
   vvd ( refb, -2.2230386571272496e-7, 1e-15, "slaRefco", "b", status );

   slaAtmdsp ( 275.9, 709.3, 0.9, 0.77,
                refa, refb, 0.5, &refa2, &refb2 );
   vvd ( refa2, 2.0345223164474501e-4, 1e-12, "slaAtmdsp", "a", status );
   vvd ( refb2, -2.2508575553417372e-7, 1e-15, "slaAtmdsp", "b", status );

   slaDcs2c ( 0.345, 0.456, vu );
   slaRefv ( vu, refa, refb, vr );
   vvd ( vr[0], 0.84474870489113119, 1e-12, "slaRefv", "x1", status );
   vvd ( vr[1], 0.30357948909651361, 1e-12, "slaRefv", "y1", status );
   vvd ( vr[2], 0.44072567361640702, 1e-12, "slaRefv", "z1", status );

   slaDcs2c ( 3.7, 0.03, vu );
   slaRefv ( vu, refa, refb, vr );
   vvd ( vr[0], -0.84761876931171143, 1e-12, "slaRefv", "x2", status );
   vvd ( vr[1], -0.52953548037016562, 1e-12, "slaRefv", "y2", status );
   vvd ( vr[2], 0.032291454982218852, 1e-12, "slaRefv", "z2", status );

   slaRefz ( 0.567, refa, refb, &zr );
   vvd ( zr, 0.56687228599484107, 1e-12, "slaRefz", "hi el", status );

   slaRefz ( 1.55, refa, refb, &zr );
   vvd ( zr, 1.5456973538521697, 1e-12, "slaRefz", "lo el", status );
}
void t_rv ( int *status )

{
   vvd ( (double) slaRverot ( -0.777f, 5.67f, -0.3f, 3.19f ),
                     -0.1948098355075913, 1e-6, "slaRverot", "", status );
   vvd ( (double) slaRvgalc ( 1.11f, -0.99f ),
                     158.9630759840254, 1e-3, "slaRvgalc", "", status );
   vvd ( (double) slaRvlg ( 3.97f, 1.09f ),
                     -197.818762175363, 1e-3, "slaRvlg", "", status );
   vvd ( (double) slaRvlsrd ( 6.01f, 0.1f ),
                      -4.082811335150567, 1e-5, "slaRvlsrd", "", status );
   vvd ( (double) slaRvlsrk ( 6.01f, 0.1f ),
                      -5.925180579830265, 1e-5, "slaRvlsrk", "", status );
}
void t_sep ( int *status )

{
   float r1[3], r2[3], ar1, br1, ar2, br2;
   double d1[3], d2[3], ad1, bd1, ad2, bd2;
   int i;

   r1[0] = 1.0f;
   r1[1] = 0.1f;
   r1[2] = 0.2f;
   r2[0] = -3.0f;
   r2[1] = 1e-3f;
   r2[2] = 0.2f;

   for ( i = 0; i < 3; i++ ) {
      d1[i] = (double) r1[i];
      d2[i] = (double) r2[i];
   }

   slaDcc2s ( d1, &ad1, &bd1 );
   slaDcc2s ( d2, &ad2, &bd2 );
   ar1 = (float) ad1;
   br1 = (float) bd1;
   ar2 = (float) ad2;
   br2 = (float) bd2;

   vvd ( slaDsep ( ad1, bd1, ad2, bd2 ), 2.860391914938267, 1e-13,
         "slaDsep", "", status );
   vvd ( (double) slaSep ( ar1, br1, ar2, br2 ), 2.860391855239868, 1e-6,
         "slaSep", "", status );
   vvd ( slaDsepv ( d1, d2 ), 2.860391914938267, 1e-13,
         "slaDsepv", "", status );
   vvd ( (double) slaSepv ( r1, r2 ), 2.860391855239868, 1e-6,
         "slaSepv", "", status );
}
void t_smat ( int *status )

{
   float a[3][3] = {
      { 2.22f,     1.6578f,     1.380522f     },
      { 1.6578f,   1.380522f,   1.22548578f   },
      { 1.380522f, 1.22548578f, 1.1356276122f }
   };
   float v[3] = { 2.28625f, 1.7128825f, 1.429432225f };
   float d;
   int j, iw[3];

   slaSmat ( 3, (float *) a, v, &d, &j, iw );

   vvd ( (double) a[0][0], 18.02550629769198,
                            1e-2, "slaSmat", "a[0][0]", status );
   vvd ( (double) a[0][1], -52.16386644917481,
                            1e-2, "slaSmat", "a[0][1]", status );
   vvd ( (double) a[0][2], 34.37875949717994,
                            1e-2, "slaSmat", "a[0][2]", status );
   vvd ( (double) a[1][0], -52.16386644917477,
                            1e-2, "slaSmat", "a[1][0]", status );
   vvd ( (double) a[1][1], 168.1778099099869,
                            1e-2, "slaSmat", "a[1][1]", status );
   vvd ( (double) a[1][2], -118.0722869694278,
                            1e-2, "slaSmat", "a[1][2]", status );
   vvd ( (double) a[2][0], 34.37875949717988,
                            1e-2, "slaSmat", "a[2][0]", status );
   vvd ( (double) a[2][1], -118.07228696942770,
                            1e-2, "slaSmat", "a[2][1]", status );
   vvd ( (double) a[2][2], 86.50307003740468,
                            1e-2, "slaSmat", "a[2][2]", status );
   vvd ( (double) v[0], 1.002346480763383,
                            1e-4, "slaSmat", "v[0]", status );
   vvd ( (double) v[1], 0.0328559401697292,
                            1e-4, "slaSmat", "v[1]", status );
   vvd ( (double) v[2], 0.004760688414898454,
                            1e-4, "slaSmat", "v[2]", status );
   vvd ( (double) d, 0.003658344147359863,
                            1e-4, "slaSmat", "d", status );
   viv ( j, 0, "slaSmat", "j", status );
}
void t_supgal ( int *status )

{
   double dl, db;

   slaSupgal ( 6.1, -1.4, &dl, &db );

   vvd ( dl, 3.798775860769474, 1e-12, "slaSupgal", "dl", status );
   vvd ( db, -0.1397070490669407, 1e-12, "slaSupgal", "db", status );
}
void t_svd ( int *status )

{
#define MP (10)
#define NP (6)
#define NC (7)

   double a[MP][NP], w[NP], v[NP][NP], work[NP], b[MP], x[NP], c[NC][NC];
   int m = 5, n = 4;
   int i, j;
   double val;

   for ( i = 0; i < m; i++ ) {
      val = (double) ( i + 1 ) / 2.0;
      b[i] = 23.0 - 3.0 * val - 11.0 * sin ( val ) + 13.0 * cos ( val );
      a[i][0] = 1.0;
      a[i][1] = val;
      a[i][2] = sin ( val );
      a[i][3] = cos ( val );
   }

   slaSvd ( m, n, MP, NP, (double *) a, w, (double *) v, work, &j );

   if ( a[0][0] > 0.0 ) {       
      for ( i = 0; i < m; i++ ) {
         for ( j = 0; j < n; j++ ) {
            a[i][j] = - a[i][j];
            v[i][j] = - v[i][j];
         }
      }
   }

   vvd ( a[0][0], -0.21532492989299, 1e-12, "slaSvd", "a[0][0]", status );
   vvd ( a[0][1],  0.67675050651267, 1e-12, "slaSvd", "a[0][1]", status );
   vvd ( a[0][2], -0.37267876361644, 1e-12, "slaSvd", "a[0][2]", status );
   vvd ( a[0][3],  0.58330405917160, 1e-12, "slaSvd", "a[0][3]", status );
   vvd ( a[1][0], -0.33693420368121, 1e-12, "slaSvd", "a[1][0]", status );
   vvd ( a[1][1],  0.48011695963936, 1e-12, "slaSvd", "a[1][1]", status );
   vvd ( a[1][2],  0.62656568539705, 1e-12, "slaSvd", "a[1][2]", status );
   vvd ( a[1][3], -0.17479918328198, 1e-12, "slaSvd", "a[1][3]", status );
   vvd ( a[2][0], -0.44396825906047, 1e-12, "slaSvd", "a[2][0]", status );
   vvd ( a[2][1],  0.18255923809825, 1e-12, "slaSvd", "a[2][1]", status );
   vvd ( a[2][2],  0.02228154115994, 1e-12, "slaSvd", "a[2][2]", status );
   vvd ( a[2][3], -0.51743308030238, 1e-12, "slaSvd", "a[2][3]", status );
   vvd ( a[3][0], -0.53172583816951, 1e-12, "slaSvd", "a[3][0]", status );
   vvd ( a[3][1], -0.16537863535943, 1e-12, "slaSvd", "a[3][1]", status );
   vvd ( a[3][2], -0.61134201569990, 1e-12, "slaSvd", "a[3][2]", status );
   vvd ( a[3][3], -0.28871221824912, 1e-12, "slaSvd", "a[3][3]", status );
   vvd ( a[4][0], -0.60022523682867, 1e-12, "slaSvd", "a[4][0]", status );
   vvd ( a[4][1], -0.50081781972404, 1e-12, "slaSvd", "a[4][1]", status );
   vvd ( a[4][2],  0.30706750690326, 1e-12, "slaSvd", "a[4][2]", status );
   vvd ( a[4][3],  0.52736124480318, 1e-12, "slaSvd", "a[4][3]", status );

   vvd ( w[0], 4.57362714220621, 1e-12, "slaSvd", "w[0]", status );
   vvd ( w[1], 1.64056393111226, 1e-12, "slaSvd", "w[1]", status );
   vvd ( w[2], 0.03999179717447, 1e-12, "slaSvd", "w[2]", status );
   vvd ( w[3], 0.37267332634218, 1e-12, "slaSvd", "w[3]", status );

   vvd ( v[0][0], -0.46531525230679, 1e-12, "slaSvd", "v[0][0]", status );
   vvd ( v[0][1],  0.41036514115630, 1e-12, "slaSvd", "v[0][1]", status );
   vvd ( v[0][2], -0.70279526907678, 1e-12, "slaSvd", "v[0][2]", status );
   vvd ( v[0][3],  0.34808185338758, 1e-12, "slaSvd", "v[0][3]", status );
   vvd ( v[1][0], -0.80342444002914, 1e-12, "slaSvd", "v[1][0]", status );
   vvd ( v[1][1], -0.29896472833787, 1e-12, "slaSvd", "v[1][1]", status );
   vvd ( v[1][2],  0.46592932810178, 1e-12, "slaSvd", "v[1][2]", status );
   vvd ( v[1][3],  0.21917828721921, 1e-12, "slaSvd", "v[1][3]", status );
   vvd ( v[2][0], -0.36564497020801, 1e-12, "slaSvd", "v[2][0]", status );
   vvd ( v[2][1],  0.28066812941896, 1e-12, "slaSvd", "v[2][1]", status );
   vvd ( v[2][2], -0.03324480702665, 1e-12, "slaSvd", "v[2][2]", status );
   vvd ( v[2][3], -0.88680546891402, 1e-12, "slaSvd", "v[2][3]", status );
   vvd ( v[3][0],  0.06553350971918, 1e-12, "slaSvd", "v[3][0]", status );
   vvd ( v[3][1],  0.81452191085452, 1e-12, "slaSvd", "v[3][1]", status );
   vvd ( v[3][2],  0.53654771808636, 1e-12, "slaSvd", "v[3][2]", status );
   vvd ( v[3][3],  0.21065602782287, 1e-12, "slaSvd", "v[3][3]", status );

   slaSvdsol ( m, n, MP, NP, b, (double *) a, w, (double *) v, work, x );

   vvd ( x[0],  23.0, 1e-12, "slaSvdsol", "x[0]", status );
   vvd ( x[1],  -3.0, 1e-12, "slaSvdsol", "x[1]", status );
   vvd ( x[2], -11.0, 1e-12, "slaSvdsol", "x[2]", status );
   vvd ( x[3],  13.0, 1e-12, "slaSvdsol", "x[3]", status );

   slaSvdcov ( n, NP, NC, w, (double *) v, work, (double *) c );

   vvd ( c[0][0],  309.77269378273270, 1e-10,
                         "slaSvdcov", "c[0][0]", status );
   vvd ( c[0][1], -204.22043941662150, 1e-10,
                         "slaSvdcov", "c[0][1]", status );
   vvd ( c[0][2],   12.43704316907477, 1e-10,
                         "slaSvdcov", "c[0][2]", status );
   vvd ( c[0][3], -235.12299986206710, 1e-10,
                         "slaSvdcov", "c[0][3]", status );
   vvd ( c[1][0], -204.22043941662150, 1e-10,
                         "slaSvdcov", "c[1][0]", status );
   vvd ( c[1][1],  136.14695961108110, 1e-10,
                         "slaSvdcov", "c[1][1]", status );
   vvd ( c[1][2],  -11.10167446246327, 1e-10,
                         "slaSvdcov", "c[1][2]", status );
   vvd ( c[1][3],  156.54937371198730, 1e-10,
                         "slaSvdcov", "c[1][3]", status );
   vvd ( c[2][0],   12.43704316907477, 1e-10,
                         "slaSvdcov", "c[2][0]", status );
   vvd ( c[2][1],  -11.10167446246327, 1e-10,
                         "slaSvdcov", "c[2][1]", status );
   vvd ( c[2][2],    6.38909830090602, 1e-10,
                         "slaSvdcov", "c[2][2]", status );
   vvd ( c[2][3],  -12.41424302586736, 1e-10,
                         "slaSvdcov", "c[2][3]", status );
   vvd ( c[3][0], -235.12299986206710, 1e-10,
                         "slaSvdcov", "c[3][0]", status );
   vvd ( c[3][1],  156.54937371198730, 1e-10,
                         "slaSvdcov", "c[3][1]", status );
   vvd ( c[3][2],  -12.41424302586736, 1e-10,
                         "slaSvdcov", "c[3][2]", status );
   vvd ( c[3][3],  180.56719842359560, 1e-10,
                         "slaSvdcov", "c[3][3]", status );
}
void t_tp ( int *status )

{
   float r0, d0, r1, d1, x, y, r2, d2, r01, d01, r02, d02;
   double dr0, dd0, dr1, dd1, dx, dy, dr2, dd2, dr01, dd01, dr02, dd02;
   int j;

   r0 = 3.1f;
   d0 = -0.9f;
   r1 = r0 + 0.2f;
   d1 = d0 - 0.1f;
   slaS2tp ( r1, d1, r0, d0, &x, &y, &j );
   vvd ( (double) x, 0.1086112301590404, 1e-6, "slaS2tp", "x", status );
   vvd ( (double) y, -0.1095506200711452, 1e-6, "slaS2tp", "y", status );
   viv ( j, 0, "slaS2tp", "j", status );
   slaTp2s ( x, y, r0, d0, &r2, &d2 );
   vvd ( (double) ( r2 - r1 ), 0.0, 1e-6, "slaTp2s", "r", status );
   vvd ( (double) ( d2 - d1 ), 0.0, 1e-6, "slaTp2s", "d", status );
   slaTps2c ( x, y, r2, d2, &r01, &d01, &r02, &d02, &j );
   vvd ( (double) r01,  3.1, 1e-6, "slaTps2c", "r1", status );
   vvd ( (double) d01, -0.9, 1e-6, "slaTps2c", "d1", status );
   vvd ( (double) r02, 0.3584073464102072, 1e-6, "slaTps2c", "r2", status );
   vvd ( (double) d02, -2.023361658234722, 1e-6, "slaTps2c", "d2", status );
   viv ( j, 1, "slaTps2c", "n", status );
   dr0 = 3.1;
   dd0 = -0.9;
   dr1 = dr0 + 0.2;
   dd1 = dd0 - 0.1;
   slaDs2tp ( dr1, dd1, dr0, dd0, &dx, &dy, &j );
   vvd ( dx, 0.1086112301590404, 1e-12, "slaDs2tp", "x", status );
   vvd ( dy, -0.1095506200711452, 1e-12, "slaDs2tp", "y", status );
   viv ( j, 0, "slaDs2tp", "j", status );
   slaDtp2s ( dx, dy, dr0, dd0, &dr2, &dd2 );
   vvd ( r2 - r1, 0.0, 1e-12, "slaDtp2s", "r", status );
   vvd ( d2 - d1, 0.0, 1e-12, "slaDtp2s", "d", status );
   slaDtps2c ( dx, dy, dr2, dd2, &dr01, &dd01, &dr02, &dd02, &j );
   vvd ( dr01,  3.1, 1e-12, "slaDtps2c", "r1", status );
   vvd ( dd01, -0.9, 1e-12, "slaDtps2c", "d1", status );
   vvd ( dr02, 0.3584073464102072, 1e-12, "slaDtps2c", "r2", status );
   vvd ( dd02, -2.023361658234722, 1e-12, "slaDtps2c", "d2", status );
   viv ( j, 1, "slaDtps2c", "n", status );
}
void t_tpv ( int *status )

{
   float rxi, reta, rv[3], rv0[3], rtxi, rteta, rtv[3], rtv01[3], rtv02[3];
   double xi, eta, x, y, z, v[3], v0[3], txi, teta, tv[3], tv01[3], tv02[3];
   int j;


   xi = -0.1;
   eta = 0.055;
   rxi = (float) xi;
   reta = (float) eta;

   x = -0.7;
   y = -0.13;
   z = sqrt ( 1.0 - x * x - y * y );
   rv[0] = (float) x;
   rv[1] = (float) y;
   rv[2] = (float) z;
   v[0] = x;
   v[1] = y;
   v[2] = z;

   x = -0.72;
   y = -0.16;
   z = sqrt ( 1.0 - x * x - y * y );
   rv0[0] = (float) x;
   rv0[1] = (float) y;
   rv0[2] = (float) z;
   v0[0] = x;
   v0[1] = y;
   v0[2] = z;

   slaTp2v ( rxi, reta, rv0, rtv );
   vvd ( (double) rtv[0], -0.7008874, 1e-6, "slaTp2v", "v[0]", status );
   vvd ( (double) rtv[1], -0.05397407, 1e-6, "slaTp2v", "v[1]", status );
   vvd ( (double) rtv[2], 0.7112268, 1e-6, "slaTp2v", "v[2]", status );

   slaDtp2v ( xi, eta, v0, tv );
   vvd ( tv[0], -0.7008874281280771, 1e-13, "slaDtp2v", "v[0]", status );
   vvd ( tv[1], -0.05397406827952735, 1e-13, "slaDtp2v", "v[1]", status );
   vvd ( tv[2], 0.7112268365615617, 1e-13, "slaDtp2v", "v[2]", status );

   slaV2tp ( rv, rv0, &rtxi, &rteta, &j);
   vvd ( (double) rtxi, -0.02497228, 1e-6, "slaV2tp", "xi", status );
   vvd ( (double) rteta, 0.03748143, 1e-6, "slaV2tp", "eta", status );
   viv ( j, 0, "slaV2tp", "j", status );

   slaDv2tp ( v, v0, &txi, &teta, &j );
   vvd ( txi, -0.02497229197023852, 1e-13, "slaDv2tp", "xi", status );
   vvd ( teta, 0.03748140764224765, 1e-13, "slaDv2tp", "eta", status );
   viv ( j, 0, "slaDv2tp", "j", status );

   slaTpv2c ( rxi, reta, rv, rtv01, rtv02, &j );
   vvd ( (double) rtv01[0], -0.7074573732537283, 1e-6,
                                          "slaTpv2c", "v01[0]", status );
   vvd ( (double) rtv01[1], -0.2372965765309941, 1e-6,
                                          "slaTpv2c", "v01[1]", status );
   vvd ( (double) rtv01[2], 0.6657284730245545, 1e-6,
                                          "slaTpv2c", "v01[2]", status );
   vvd ( (double) rtv02[0], -0.6680480104758149, 1e-6,
                                          "slaTpv2c", "v02[0]", status );
   vvd ( (double) rtv02[1], -0.02915588494045333, 1e-6,
                                          "slaTpv2c", "v02[1]", status );
   vvd ( (double) rtv02[2], 0.7435467638774610, 1e-6,
                                          "slaTpv2c", "v02[2]", status );
   viv ( j, 1, "slaTpv2c", "n", status );

   slaDtpv2c ( xi, eta, v, tv01, tv02, &j );
   vvd ( tv01[0], -0.7074573732537283, 1e-13, "slaDtpv2c", "v01[0]", status );
   vvd ( tv01[1], -0.2372965765309941, 1e-13, "slaDtpv2c", "v01[1]", status );
   vvd ( tv01[2], 0.6657284730245545, 1e-13, "slaDtpv2c", "v01[2]", status );
   vvd ( tv02[0], -0.6680480104758149, 1e-13, "slaDtpv2c", "v02[0]", status );
   vvd ( tv02[1], -0.02915588494045333, 1e-13, "slaDtpv2c", "v02[1]", status )
;
   vvd ( tv02[2], 0.7435467638774610, 1e-13, "slaDtpv2c", "v02[2]", status );
   viv ( j, 1, "slaDtpv2c", "n", status );
}
void t_vecmat ( int *status )

{
   int i;
   float av[3], rm1[3][3], rm2[3][3], rm[3][3], v1[3], v2[3],
         v3[3], v4[3], v5[3], vm, v6[3], v7[3];
   double dav[3], drm1[3][3], drm2[3][3], drm[3][3], dv1[3], dv2[3],
          dv3[3], dv4[3], dv5[3], dvm, dv6[3], dv7[3];


   av[0] = -0.123f;
   av[1] = 0.0987f;
   av[2] = 0.0654f;
   slaAv2m ( av, rm1 );
   vvd ( (double) rm1[0][0], 0.9930075842721269,
                                 1e-6, "slaAv2m", "00", status );
   vvd ( (double) rm1[0][1], 0.05902743090199868,
                                 1e-6, "slaAv2m", "01", status );
   vvd ( (double) rm1[0][2], -0.1022335560329612,
                                 1e-6, "slaAv2m", "02", status );
   vvd ( (double) rm1[1][0], -0.07113807138648245,
                                 1e-6, "slaAv2m", "10", status );
   vvd ( (double) rm1[1][1], 0.9903204657727545,
                                 1e-6, "slaAv2m", "11", status );
   vvd ( (double) rm1[1][2], -0.1191836812279541,
                                 1e-6, "slaAv2m", "12", status );
   vvd ( (double) rm1[2][0], 0.09420887631983825,
                                 1e-6, "slaAv2m", "20", status );
   vvd ( (double) rm1[2][1], 0.1256229973879967,
                                 1e-6, "slaAv2m", "21", status );
   vvd ( (double) rm1[2][2], 0.9875948309655174,
                                 1e-6, "slaAv2m", "22", status );


   slaEuler ( "YZY", 2.345f, -0.333f, 2.222f, rm2 );
   vvd ( (double) rm2[0][0], -0.1681574770810878,
                                 1e-6, "slaEuler", "00", status );
   vvd ( (double) rm2[0][1], 0.1981362273264315,
                                 1e-6, "slaEuler", "01", status );
   vvd ( (double) rm2[0][2], 0.9656423242187410,
                                 1e-6, "slaEuler", "02", status );
   vvd ( (double) rm2[1][0], -0.2285369373983370,
                                 1e-6, "slaEuler", "10", status );
   vvd ( (double) rm2[1][1], 0.9450659587140423,
                                 1e-6, "slaEuler", "11", status );
   vvd ( (double) rm2[1][2], -0.2337117924378156,
                                 1e-6, "slaEuler", "12", status );
   vvd ( (double) rm2[2][0], -0.9589024617479674,
                                 1e-6, "slaEuler", "20", status );
   vvd ( (double) rm2[2][1], -0.2599853247796050,
                                 1e-6, "slaEuler", "21", status );
   vvd ( (double) rm2[2][2], -0.1136384607117296,
                                 1e-6, "slaEuler", "22", status );


   slaMxm ( rm2, rm1, rm );
   vvd ( (double) rm[0][0], -0.09010460088585805,
                                 1e-6, "slaMxm", "00", status );
   vvd ( (double) rm[0][1], 0.3075993402463796,
                                 1e-6, "slaMxm", "01", status );
   vvd ( (double) rm[0][2], 0.9472400998581048,
                                 1e-6, "slaMxm", "02", status );
   vvd ( (double) rm[1][0], -0.3161868071070688,
                                 1e-6, "slaMxm", "10", status );
   vvd ( (double) rm[1][1], 0.8930686362478707,
                                 1e-6, "slaMxm", "11", status );
   vvd ( (double) rm[1][2],-0.3200848543149236,
                                 1e-6, "slaMxm", "12", status );
   vvd ( (double) rm[2][0],-0.9444083141897035,
                                 1e-6, "slaMxm", "20", status );
   vvd ( (double) rm[2][1],-0.3283459407855694,
                                 1e-6, "slaMxm", "21", status );
   vvd ( (double) rm[2][2], 0.01678926022795169,
                                 1e-6, "slaMxm", "22", status );


   slaCs2c ( 3.0123f, -0.999f, v1 );
   vvd ( (double) v1[0], -0.5366267667260525,
                                 1e-6, "slaCs2c", "x", status );
   vvd ( (double) v1[1], 0.06977111097651444,
                                 1e-6, "slaCs2c", "y", status );
   vvd ( (double) v1[2], -0.8409302618566215,
                                 1e-6, "slaCs2c", "z", status );


   slaMxv ( rm1, v1, v2 );
   slaMxv ( rm2, v2, v3 );
   vvd ( (double) v3[0], -0.7267487768696160,
                                 1e-6, "slaMxv", "x", status );
   vvd ( (double) v3[1], 0.5011537352639822,
                                 1e-6, "slaMxv", "y", status );
   vvd ( (double) v3[2], 0.4697671220397141,
                                 1e-6, "slaMxv", "z", status );


   slaImxv ( rm, v3, v4 );
   vvd ( (double) v4[0], -0.5366267667260526,
                                 1e-6, "slaImxv", "x", status );
   vvd ( (double) v4[1], 0.06977111097651445,
                                 1e-6, "slaImxv", "y", status );
   vvd ( (double) v4[2], -0.8409302618566215,
                                 1e-6, "slaImxv", "z", status );


   slaM2av ( rm, v5 );
   vvd ( (double) v5[0], 0.006889040510209034,
                                 1e-6, "slaM2av", "x", status );
   vvd ( (double) v5[1], -1.577473205461961,
                                 1e-6, "slaM2av", "y", status );
   vvd ( (double) v5[2], 0.5201843672856759,
                                 1e-6, "slaM2av", "z", status );


   for ( i = 0; i < 3; i++ ) {
      v5[i] *= 1000.0f;
   }
   slaVn ( v5, v6, &vm );
   vvd ( (double) v6[0], 0.004147420704640065,
                                 1e-6, "slaVn", "x", status );
   vvd ( (double) v6[1], -0.9496888606842218,
                                 1e-6, "slaVn", "y", status );
   vvd ( (double) v6[2], 0.3131674740355448,
                                 1e-6, "slaVn", "z", status );
   vvd ( (double) vm, 1661.042127339937,
                                 1e-3, "slaVn", "m", status );


   vvd ( (double) slaVdv ( v6, v1 ), -0.3318384698006295,
                                 1e-6, "slaVn", "", status );


   slaVxv (v6, v1, v7 );
   vvd ( (double) v7[0], 0.7767720597123304,
                                 1e-6, "slaVxv", "x", status );
   vvd ( (double) v7[1], -0.1645663574562769,
                                 1e-6, "slaVxv", "y", status );
   vvd ( (double) v7[2], -0.5093390925544726,
                                 1e-6, "slaVxv", "z", status );



   dav[0] = -0.123;
   dav[1] = 0.0987;
   dav[2] = 0.0654;
   slaDav2m ( dav, drm1 );
   vvd ( drm1[0][0], 0.9930075842721269, 1e-12, "slaDav2m", "00", status );
   vvd ( drm1[0][1], 0.05902743090199868, 1e-12, "slaDav2m", "01", status );
   vvd ( drm1[0][2], -0.1022335560329612, 1e-12, "slaDav2m", "02", status );
   vvd ( drm1[1][0], -0.07113807138648245, 1e-12, "slaDav2m", "10", status );
   vvd ( drm1[1][1], 0.9903204657727545, 1e-12, "slaDav2m", "11", status );
   vvd ( drm1[1][2], -0.1191836812279541, 1e-12, "slaDav2m", "12", status );
   vvd ( drm1[2][0], 0.09420887631983825, 1e-12, "slaDav2m", "20", status );
   vvd ( drm1[2][1], 0.1256229973879967, 1e-12, "slaDav2m", "21", status );
   vvd ( drm1[2][2], 0.9875948309655174, 1e-12, "slaDav2m", "22", status );

   slaDeuler ( "YZY", 2.345, -0.333, 2.222, drm2 );
   vvd ( drm2[0][0], -0.1681574770810878, 1e-12, "slaDeuler", "00", status );
   vvd ( drm2[0][1], 0.1981362273264315, 1e-12, "slaDeuler", "01", status );
   vvd ( drm2[0][2], 0.9656423242187410, 1e-12, "slaDeuler", "02", status );
   vvd ( drm2[1][0], -0.2285369373983370, 1e-12, "slaDeuler", "10", status );
   vvd ( drm2[1][1], 0.9450659587140423, 1e-12, "slaDeuler", "11", status );
   vvd ( drm2[1][2], -0.2337117924378156, 1e-12, "slaDeuler", "12", status );
   vvd ( drm2[2][0], -0.9589024617479674, 1e-12, "slaDeuler", "20", status );
   vvd ( drm2[2][1], -0.2599853247796050, 1e-12, "slaDeuler", "21", status );
   vvd ( drm2[2][2], -0.1136384607117296, 1e-12, "slaDeuler", "22", status );

   slaDmxm ( drm2, drm1, drm );
   vvd ( drm[0][0], -0.09010460088585805 , 1e-12, "slaDmxm", "00", status );
   vvd ( drm[0][1], 0.3075993402463796, 1e-12, "slaDmxm", "01", status );
   vvd ( drm[0][2], 0.9472400998581048, 1e-12, "slaDmxm", "02", status );
   vvd ( drm[1][0], -0.3161868071070688, 1e-12, "slaDmxm", "10", status );
   vvd ( drm[1][1], 0.8930686362478707, 1e-12, "slaDmxm", "11", status );
   vvd ( drm[1][2], -0.3200848543149236, 1e-12, "slaDmxm", "12", status );
   vvd ( drm[2][0], -0.9444083141897035, 1e-12, "slaDmxm", "20", status );
   vvd ( drm[2][1], -0.3283459407855694, 1e-12, "slaDmxm", "21", status );
   vvd ( drm[2][2], 0.01678926022795169, 1e-12, "slaDmxm", "22", status );

   slaDcs2c ( 3.0123, -0.999, dv1 );
   vvd ( dv1[0], -0.5366267667260525, 1e-12, "slaDcs2c", "x", status );
   vvd ( dv1[1], 0.06977111097651444, 1e-12, "slaDcs2c", "y", status );
   vvd ( dv1[2], -0.8409302618566215, 1e-12, "slaDcs2c", "z", status );

   slaDmxv ( drm1, dv1, dv2 );
   slaDmxv ( drm2, dv2, dv3 );
   vvd ( dv3[0], -0.7267487768696160, 1e-12, "slaDmxv", "x", status );
   vvd ( dv3[1], 0.5011537352639822, 1e-12, "slaDmxv", "y", status );
   vvd ( dv3[2], 0.4697671220397141, 1e-12, "slaDmxv", "z", status );

   slaDimxv ( drm, dv3, dv4 );
   vvd ( dv4[0], -0.5366267667260526 , 1e-12, "slaDimxv", "x", status );
   vvd ( dv4[1], 0.06977111097651445, 1e-12, "slaDimxv", "y", status );
   vvd ( dv4[2], -0.8409302618566215 , 1e-12, "slaDimxv", "z", status );

   slaDm2av ( drm, dv5 );
   vvd ( dv5[0], 0.006889040510209034, 1e-12, "slaDm2av", "x", status );
   vvd ( dv5[1], -1.577473205461961, 1e-12, "slaDm2av", "y", status );
   vvd ( dv5[2], 0.5201843672856759, 1e-12, "slaDm2av", "z", status );

   for ( i = 0; i < 3; i++ ) {
      dv5[i] *= 1000.0;
   }
   slaDvn ( dv5, dv6, &dvm );
   vvd ( dv6[0], 0.004147420704640065, 1e-12, "slaDvn", "x", status );
   vvd ( dv6[1], -0.9496888606842218, 1e-12, "slaDvn", "y", status );
   vvd ( dv6[2], 0.3131674740355448, 1e-12, "slaDvn", "z", status );
   vvd ( dvm, 1661.042127339937, 1e-9, "slaDvn", "m", status );

   vvd ( slaDvdv ( dv6, dv1 ), -0.3318384698006295,
                                        1e-12, "slaDvn", "", status );

   slaDvxv (dv6, dv1, dv7 );
   vvd ( dv7[0], 0.7767720597123304, 1e-12, "slaDvxv", "x", status );
   vvd ( dv7[1], -0.1645663574562769, 1e-12, "slaDvxv", "y", status );
   vvd ( dv7[2], -0.5093390925544726, 1e-12, "slaDvxv", "z", status );
}
void t_zd ( int *status )

{
   vvd ( slaZd ( -1.023, -0.876, -0.432 ),
               0.8963914139430839, 1e-12, "slaZd", "", status );
}
