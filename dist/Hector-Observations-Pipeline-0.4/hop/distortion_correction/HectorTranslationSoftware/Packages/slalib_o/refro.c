/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
static void qfoo(double,double,double,double,double,double,
double,double,double,double,double,double,double*,double*,
double*);static void qbar(double,double,double,double,double
*,double*);void slaRefro(double qbaz,double hm,double tk,
double Q0,double rh,double qfobar,double phi,double tlr,
double q1,double*q2)
#define qfoobar 16384
#define Q3(q4,qfOBAz) ((qfOBAz)/(q4+qfOBAz));
{const double qfoobaz=1.623156204;const double QQUUX=
1.579522973;const double Q5=299792.458;const double QFRED=
8314.32;const double qdog=28.9644;const double qcat=18.0152;
const double QFISH=6378120.0;const double QgASp=18.36;const 
double Q6=11000.0;const double q7=80000.0;double wl;double 
q8;double QBAD;double qBuG;double qsilly;double QBUGGY;
double QMUM;double qDAd;double q9;double Q10;double Q11,q12,
Q13;double Q14;double qdisk;double Q15;double q16;double q17
;double QEMPTY;int q18,QFULL,qfast,qsmall,QBIG,QOK;double 
QHELLO,QBYE,QMAGIC,q19,q20,qobSCUrE,QSPEED,qIndex,Q21,qbill,
q22,q23,qjoe,qemacs,q24,QVI,qrms,QfbI,Qcia,Q25,Q26,QNASA,
QERR,Q27,Q28,qgoogle,q29,QYahoO,Q30,qtrick,q31,Q32,QHINT,Q33
,Q34,QBLAcK,Q35,q36,q37,q38,q39,qred,QgreEN,QYELLOW,q40,
QBLUE,QMAGENTA,QCyaN,Q41,QWHITE,qclinton,q42,qbUsh,q43,Q44,
Q45;double q46[]={50.474238,50.987749,51.503350,52.021410,
52.542394,53.066907,53.595749,54.130000,54.671159,55.221367,
55.783802,56.264775,56.363389,56.968206,57.612484,58.323877,
58.446590,59.164207,59.590983,60.306061,60.434776,61.150560,
61.800154,62.411215,62.486260,62.997977,63.568518,64.127767,
64.678903,65.224071,65.764772,66.302091,66.836830,67.369598,
67.900867,68.431005,68.960311,118.750343,368.498350,
424.763124,487.249370,715.393150,773.839675,834.145330,
22.235080,67.803960,119.995940,183.310091,321.225644,
325.152919,336.222601,380.197372,390.134508,437.346667,
439.150812,443.018295,448.001075,470.888947,474.689127,
488.491133,503.568532,504.482692,547.676440,552.020960,
556.936002,620.700807,645.866155,658.005280,752.033227,
841.053973,859.962313,899.306675,902.616173,906.207325,
916.171582,923.118427,970.315022,987.926764,1780.00000};
double qJFK[]={0.094e-6,0.246e-6,0.608e-6,1.414e-6,3.102e-6,
6.410e-6,12.470e-6,22.800e-6,39.180e-6,63.160e-6,95.350e-6,
54.890e-6,134.400e-6,176.300e-6,214.100e-6,238.600e-6,
145.700e-6,240.400e-6,211.200e-6,212.400e-6,246.100e-6,
250.400e-6,229.800e-6,193.300e-6,151.700e-6,150.300e-6,
108.700e-6,73.350e-6,46.350e-6,27.480e-6,15.300e-6,8.009e-6,
3.946e-6,1.832e-6,0.801e-6,0.330e-6,0.128e-6,94.500e-6,
6.790e-6,63.800e-6,23.500e-6,9.960e-6,67.100e-6,18.000e-6,
0.1130e-1,0.0012e-1,0.0008e-1,2.4200e-1,0.0483e-1,1.4990e-1,
0.0011e-1,11.5200e-1,0.0046e-1,0.0650e-1,0.9218e-1,0.1976e-1
,10.3200e-1,0.3297e-1,1.2620e-1,0.2520e-1,0.0390e-1,
0.0130e-1,9.7010e-1,14.7700e-1,487.4000e-1,5.0120e-1,
0.0713e-1,0.3022e-1,239.6000e-1,0.0140e-1,0.1472e-1,
0.0605e-1,0.0426e-1,0.1876e-1,8.3410e-1,0.0869e-1,8.9720e-1,
132.1000e-1,22300.0000e-1};double Q47[]={9.694,8.694,7.744,
6.844,6.004,5.224,4.484,3.814,3.194,2.624,2.119,0.015,1.660,
1.260,0.915,0.626,0.084,0.391,0.212,0.212,0.391,0.626,0.915,
1.260,0.083,1.665,2.115,2.620,3.195,3.815,4.485,5.225,6.005,
6.845,7.745,8.695,9.695,0.009,0.049,0.044,0.049,0.145,0.130,
0.147,2.143,8.735,8.356,0.668,6.181,1.540,9.829,1.048,7.350,
5.050,3.596,5.050,1.405,3.599,2.381,2.853,6.733,6.733,0.114,
0.114,0.159,2.200,8.580,7.820,0.396,8.180,7.989,7.917,8.432,
5.111,1.442,10.220,1.920,0.258,0.952};double q48[]={0.890e-3
,0.910e-3,0.940e-3,0.970e-3,0.990e-3,1.020e-3,1.050e-3,
1.070e-3,1.100e-3,1.130e-3,1.170e-3,1.730e-3,1.200e-3,
1.240e-3,1.280e-3,1.330e-3,1.520e-3,1.390e-3,1.430e-3,
1.450e-3,1.360e-3,1.310e-3,1.270e-3,1.230e-3,1.540e-3,
1.200e-3,1.170e-3,1.130e-3,1.100e-3,1.070e-3,1.050e-3,
1.020e-3,0.990e-3,0.970e-3,0.940e-3,0.920e-3,0.900e-3,
1.630e-3,1.920e-3,1.930e-3,1.920e-3,1.810e-3,1.810e-3,
1.820e-3,2.811e-3,2.858e-3,2.948e-3,3.050e-3,2.303e-3,
2.783e-3,2.693e-3,2.873e-3,2.152e-3,1.845e-3,2.100e-3,
1.860e-3,2.632e-3,2.152e-3,2.355e-3,2.602e-3,1.612e-3,
1.612e-3,2.600e-3,2.600e-3,3.210e-3,2.438e-3,1.800e-3,
3.210e-3,3.060e-3,1.590e-3,3.060e-3,2.985e-3,2.865e-3,
2.408e-3,2.670e-3,2.900e-3,2.550e-3,2.985e-3,17.620e-3};
double Q49[]={4.80,4.93,4.78,5.30,4.69,4.85,4.74,5.38,4.81,
4.23,4.29,4.23,4.84,4.57,4.65,5.04,3.98,4.01,4.50,4.50,4.11,
4.68,4.00,4.14,4.09,5.76,4.09,4.53,5.10,4.70,5.00,4.78,4.94,
4.55,30.50};double Q50[]={0.240e-3,0.220e-3,0.197e-3,
0.166e-3,0.136e-3,0.131e-3,0.230e-3,0.335e-3,0.374e-3,
0.258e-3,-0.166e-3,0.390e-3,-0.297e-3,-0.416e-3,-0.613e-3,-
0.205e-3,0.748e-3,-0.722e-3,0.765e-3,-0.705e-3,0.697e-3,
0.104e-3,0.570e-3,0.360e-3,-0.498e-3,0.239e-3,0.108e-3,-
0.311e-3,-0.421e-3,-0.375e-3,-0.267e-3,-0.168e-3,-0.169e-3,-
0.200e-3,-0.228e-3,-0.240e-3,-0.250e-3,-0.036e-3,0.0,0.0,0.0
,0.0,0.0,0.0,0.69,0.69,0.70,0.64,0.67,0.68,0.69,0.54,0.63,
0.60,0.63,0.60,0.66,0.66,0.65,0.69,0.61,0.61,0.70,0.70,0.69,
0.71,0.60,0.69,0.68,0.33,0.68,0.68,0.70,0.70,0.70,0.70,0.64,
0.68,2.00};double q51[]={0.790e-3,0.780e-3,0.774e-3,0.764e-3
,0.751e-3,0.714e-3,0.584e-3,0.431e-3,0.305e-3,0.339e-3,
0.705e-3,-0.113e-3,0.753e-3,0.742e-3,0.697e-3,0.051e-3,-
0.146e-3,0.266e-3,-0.090e-3,0.081e-3,-0.324e-3,-0.067e-3,-
0.761e-3,-0.777e-3,0.097e-3,-0.768e-3,-0.706e-3,-0.332e-3,-
0.298e-3,-0.423e-3,-0.575e-3,-0.700e-3,-0.735e-3,-0.744e-3,-
0.753e-3,-0.760e-3,-0.765e-3,0.009e-3,0.0,0.0,0.0,0.0,0.0,
0.0,1.00,0.82,0.79,0.85,0.54,0.74,0.61,0.89,0.55,0.48,0.52,
0.50,0.67,0.65,0.64,0.72,0.43,0.45,1.00,1.00,1.00,0.68,0.50,
1.00,0.84,0.45,0.84,0.90,0.95,0.53,0.78,0.80,0.67,0.90,5.00}
;QHELLO=(qfobar!=0.0)?qfobar:0.1;if(QHELLO<0.0){q8=-QHELLO;
wl=Q5/q8;}else{wl=QHELLO;q8=Q5/wl;}wl=gmax(wl,0.1);q18=(wl<=
100.0);QBYE=slaDrange(qbaz);QHELLO=fabs(QBYE);QMAGIC=gmin(
QHELLO,q18?qfoobaz:QQUUX);QHELLO=gmax(hm,-1000.0);QBAD=gmin(
QHELLO,q7);QHELLO=gmax(tk,100.0);qBuG=gmin(QHELLO,500.0);
QHELLO=gmax(Q0,0.0);qsilly=gmin(QHELLO,10000.0);QHELLO=gmax(
rh,0.0);QBUGGY=gmin(QHELLO,1.0);QHELLO=fabs(tlr);QHELLO=gmax
(QHELLO,0.001);QMUM=gmin(QHELLO,0.01);QHELLO=fabs(q1);QHELLO
=gmax(QHELLO,1e-12);q19=gmin(QHELLO,0.1)/2.0;q20=qBuG*qBuG;
qobSCUrE=q20*qBuG;if(qBuG>273.16){QSPEED=q20*q20;qIndex=-
2991.2729/q20-6017.0128/qBuG+18.87643854-0.028354721*qBuG+
0.17838301e-4*q20-0.84150417e-9*qobSCUrE+0.44412543e-12*
QSPEED+2.858487*log(qBuG);QHELLO=qBuG-242.55-3.8e-2*qsilly;
QHELLO=4.1e-4+qsilly*(3.48e-6+7.4e-10*QHELLO*QHELLO);}else{
qIndex=-5865.3696/qBuG+22.241033+0.013749042*qBuG-
0.34031775e-4*q20+0.26967687e-7*qobSCUrE+0.6918651*log(qBuG)
;QHELLO=qBuG-249.35-3.1e-2*qsilly;QHELLO=4.8e-4+qsilly*(
3.47e-6+5.9e-10*QHELLO*QHELLO);}qDAd=(QHELLO+1.0)*exp(qIndex
)/100.0;Q10=(qsilly>0.0)?QBUGGY*qDAd/(1.0-(1.0-QBUGGY)*qDAd/
qsilly):0.0;if(q18){QHELLO=wl*wl;Q11=((287.6155+1.62887/
QHELLO+0.01360/(QHELLO*QHELLO))*273.15/1013.25)*1e-6;q12=
11.2684e-6;Q13=0.0;}else{q9=qsilly-Q10;Q21=qbill=0.0;q22=
300.0/qBuG;q23=q22*q22;qjoe=1.0-q22;qemacs=pow(q22,0.8);q24=
q9*q22*q23;QVI=q9*qemacs+1.1*Q10*q22;qrms=qsilly*qemacs;for(
QFULL=0;QFULL<38;QFULL++){QfbI=q46[QFULL];Qcia=q24*qJFK[
QFULL]*exp(Q47[QFULL]*qjoe)/QfbI;Q26=-Qcia*(Q50[QFULL]+q51[
QFULL]*q22)*qrms;QHELLO=q48[QFULL]*QVI;Q25=sqrt(QHELLO*
QHELLO+2.25e-6);QNASA=Q25*Q25;QERR=QfbI-q8;QHELLO=QERR*QERR+
QNASA;Q21+=(Qcia*QERR-Q26*Q25)/QHELLO;qbill+=(Q26*QERR+Qcia*
Q25)/QHELLO;QERR=QfbI+q8;QHELLO=QERR*QERR+QNASA;Q21-=(Qcia*
QERR-Q26*Q25)/QHELLO;qbill+=(Q26*QERR+Qcia*Q25)/QHELLO;}QVI=
q9*pow(q22,0.2)+1.1*Q10*q22;for(QFULL=38;QFULL<44;QFULL++){
QfbI=q46[QFULL];Qcia=q24*qJFK[QFULL]*exp(Q47[QFULL]*qjoe)/
QfbI;QHELLO=q48[QFULL]*QVI;Q25=sqrt(QHELLO*QHELLO+2.25e-6);
QNASA=Q25*Q25;QERR=QfbI-q8;QHELLO=Qcia/(QERR*QERR+QNASA);Q21
+=QHELLO*QERR;qbill+=QHELLO*Q25;QERR=QfbI+q8;QHELLO=Qcia/(
QERR*QERR+QNASA);Q21-=QHELLO*QERR;qbill+=QHELLO*Q25;}if(
qbill<0.0)Q21=0.0;Q27=qsilly*qemacs*0.56e-3;Q21-=(6.14e-5*q9
*q23)*q8/(q8*q8+Q27*Q27);Q28=2.1316e-12/q22;q24=Q10*pow(q22,
3.5);for(QFULL=44;QFULL<79;QFULL++){QfbI=q46[QFULL];Qcia=q24
*qJFK[QFULL]*exp(Q47[QFULL]*qjoe)/QfbI;Q25=q48[QFULL]*(q9*
pow(q22,Q50[QFULL])+Q10*Q49[QFULL-44]*pow(q22,q51[QFULL]));
QHELLO=QfbI;Q25=0.535*Q25+sqrt(0.217*Q25*Q25+Q28*QHELLO*
QHELLO);QNASA=Q25*Q25;QERR=QfbI-q8;Q21+=Qcia*QERR/(QERR*QERR
+QNASA);QERR=QfbI+q8;Q21-=Qcia*QERR/(QERR*QERR+QNASA);}Q11=(
77.6890+q8*Q21)*1e-6;q12=6.3938e-6;Q13=375463e-6;}Q14=9.784*
(1.0-2.6e-3*cos(2.0*phi)-2.8e-7*QBAD);qdisk=Q14*qdog/QFRED;
Q15=qdisk/QMUM;QHELLO=(1.0-qcat/qdog)*Q15/(QgASp-Q15);
qgoogle=Q10/qBuG;q24=Q11*(qsilly+QHELLO*Q10)/qBuG;QVI=(Q11*
QHELLO+q12)*qgoogle;q29=Q13*qgoogle;qrms=QMUM*(Q15-1.0)*q24/
qBuG;QYahoO=QMUM*(QgASp-1.0)*QVI/qBuG;Q30=QMUM*(QgASp-2.0)*
q29/(qBuG*qBuG);q16=QFISH+QBAD;qfoo(q16,qBuG,QMUM,QgASp,Q15,
q24,QVI,q29,qrms,QYahoO,Q30,q16,&qtrick,&q31,&Q32);QHINT=q31
*q16*sin(QMAGIC);Q33=Q3(q31,Q32);q17=QFISH+gmax(Q6,QBAD);
qfoo(q16,qBuG,QMUM,QgASp,Q15,q24,QVI,q29,qrms,QYahoO,Q30,q17
,&qtrick,&q31,&Q32);Q34=asin(QHINT/(q17*q31));QBLAcK=Q3(q31,
Q32);Q35=q31-1.0;q36=qdisk/qtrick;qbar(q17,Q35,q36,q17,&q31,
&Q32);q37=asin(QHINT/(q17*q31));q38=Q3(q31,Q32);QEMPTY=QFISH
+q7;qbar(q17,Q35,q36,QEMPTY,&q31,&Q32);q39=asin(QHINT/(
QEMPTY*q31));qred=Q3(q31,Q32);QgreEN=0.0;for(qfast=1;qfast<=
2;qfast++){QYELLOW=1.0;qsmall=8;if(qfast==1){q40=QMAGIC;
QBLUE=Q34-q40;QMAGENTA=Q33;QCyaN=QBLAcK;Q41=q16;}else{q40=
q37;QBLUE=q39-q40;QMAGENTA=q38;QCyaN=qred;Q41=q17;}QWHITE=
qclinton=0.0;QBIG=1;for(;;){q42=QBLUE/(double)qsmall;qbUsh=
Q41;for(QOK=1;QOK<qsmall;QOK+=QBIG){q43=sin(q40+q42*(double)
QOK);if(q43>1e-20){QHELLO=QHINT/q43;QFULL=0;do{if(qfast==1){
qfoo(q16,qBuG,QMUM,QgASp,Q15,q24,QVI,q29,qrms,QYahoO,Q30,
qbUsh,&qtrick,&q31,&Q32);}else{qbar(q17,Q35,q36,qbUsh,&q31,&
Q32);}Q44=(qbUsh*q31-QHELLO)/(q31+Q32);qbUsh-=Q44;}while(
fabs(Q44)>1.0&&QFULL++<=4);}if(qfast==1){qfoo(q16,qBuG,QMUM,
QgASp,Q15,q24,QVI,q29,qrms,QYahoO,Q30,qbUsh,&qtrick,&q31,&
Q32);}else{qbar(q17,Q35,q36,qbUsh,&q31,&Q32);}qgoogle=Q3(q31
,Q32);if(QBIG==1&&QOK%2==0){qclinton+=qgoogle;}else{QWHITE+=
qgoogle;}}Q45=q42*(QMAGENTA+4.0*QWHITE+2.0*qclinton+QCyaN)/
3.0;if(fabs(Q45-QYELLOW)<=q19||qsmall>=qfoobar){QgreEN+=Q45;
break;}else{QYELLOW=Q45;qsmall+=qsmall;qclinton+=QWHITE;
QWHITE=0.0;QBIG=2;}}}*q2=(QBYE>0.0)?QgreEN:-QgreEN;}static 
void qfoo(double q16,double qBuG,double QMUM,double QgASp,
double Q15,double q24,double QVI,double q29,double qrms,
double QYahoO,double Q30,double qbUsh,double*q52,double*q31,
double*Q32){double qtrick,QHELLO,q53,qSEx;qtrick=qBuG-QMUM*(
qbUsh-q16);QHELLO=qtrick/qBuG;q53=pow(QHELLO,Q15);qSEx=pow(
QHELLO,QgASp);*q52=qtrick;*q31=1.0+(q24*q53-(QVI-q29/qtrick)
*qSEx)/QHELLO;*Q32=-qbUsh*(qrms*q53-QYahoO*qSEx+Q30*qSEx/
QHELLO)/(QHELLO*QHELLO);}static void qbar(double q17,double 
Q35,double q36,double qbUsh,double*q31,double*Q32){double 
QHELLO;QHELLO=Q35*exp(-q36*(qbUsh-q17));*q31=1.0+QHELLO;*Q32
=-qbUsh*q36*QHELLO;}
