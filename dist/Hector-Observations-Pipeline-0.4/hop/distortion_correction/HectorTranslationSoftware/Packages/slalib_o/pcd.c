/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaPcd(double qfoo,double*qbar,double*qbaz){double Q0;
Q0=1.0+qfoo*(*qbar**qbar+*qbaz**qbaz);*qbar*=Q0;*qbaz*=Q0;}
