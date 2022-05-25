/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaGst(double qfoo,double qbar,double qbaz){return 
slaDranrm(slaEra(qbar,qbaz)-slaEo(qfoo));}
