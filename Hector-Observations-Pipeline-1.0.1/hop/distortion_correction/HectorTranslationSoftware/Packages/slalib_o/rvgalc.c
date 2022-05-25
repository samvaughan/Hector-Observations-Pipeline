/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
float slaRvgalc(float qfoo,float qbar){static float qbaz[3]=
{-108.70408f,97.86251f,-164.33610f};float Q0[3];slaCs2c(qfoo
,qbar,Q0);return slaVdv(qbaz,Q0);}
