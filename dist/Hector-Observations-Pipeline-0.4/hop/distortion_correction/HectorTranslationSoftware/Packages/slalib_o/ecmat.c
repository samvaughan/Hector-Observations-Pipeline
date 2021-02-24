/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaEcmat(double qfoo,double qbar[3][3]){double qbaz,Q0;
qbaz=(qfoo-51544.5)/36525.0;Q0=DAS2R*(84381.448+(-46.8150+(-
0.00059+0.001813*qbaz)*qbaz)*qbaz);slaDeuler("\130",Q0,0.0,
0.0,qbar);}
