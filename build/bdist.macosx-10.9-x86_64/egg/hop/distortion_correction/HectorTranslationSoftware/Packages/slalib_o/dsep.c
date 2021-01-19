/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaDsep(double qfoo,double qbar,double qbaz,double Q0
){double qfobar[3],q1[3];slaDcs2c(qfoo,qbar,qfobar);slaDcs2c
(qbaz,Q0,q1);return slaDsepv(qfobar,q1);}
