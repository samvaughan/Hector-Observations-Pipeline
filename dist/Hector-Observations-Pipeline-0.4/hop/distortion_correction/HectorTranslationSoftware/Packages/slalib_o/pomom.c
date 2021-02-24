/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaPomom(double qfoo,double qbar,double qbaz,double Q0[
3][3]){slaDeuler("\172\171\170",slaSp(qfoo),-qbar,-qbaz,Q0);
}
