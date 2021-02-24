/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
float slaRvlsrk(float qfoo,float qbar){static float qbaz[3]=
{-0.29000f,17.31726f,-10.00141f};float Q0[3];slaCs2c(qfoo,
qbar,Q0);return slaVdv(qbaz,Q0);}
