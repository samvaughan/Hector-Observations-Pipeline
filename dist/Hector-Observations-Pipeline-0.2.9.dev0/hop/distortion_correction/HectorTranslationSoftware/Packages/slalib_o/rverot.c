/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
float slaRverot(float phi,float qfoo,float qbar,float qbaz)
#define Q0 0.4655
{return(float)(Q0*cos((double)phi)*sin((double)(qbaz-qfoo))*
cos((double)qbar));}
