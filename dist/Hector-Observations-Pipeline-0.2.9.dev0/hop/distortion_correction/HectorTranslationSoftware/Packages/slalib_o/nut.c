/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaNut(double qfoo,double qbar[3][3]){double qbaz,Q0,
qfobar;slaNutc(qfoo,&qbaz,&Q0,&qfobar);slaDeuler(
"\170\172\170",qfobar,-qbaz,-(qfobar+Q0),qbar);}
