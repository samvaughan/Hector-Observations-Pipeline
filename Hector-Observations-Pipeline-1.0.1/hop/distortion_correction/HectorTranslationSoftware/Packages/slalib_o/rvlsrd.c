/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
float slaRvlsrd(float qfoo,float qbar){static float qbaz[3]=
{0.63823f,14.58542f,-7.80116f};float Q0[3];slaCs2c(qfoo,qbar
,Q0);return slaVdv(qbaz,Q0);}
