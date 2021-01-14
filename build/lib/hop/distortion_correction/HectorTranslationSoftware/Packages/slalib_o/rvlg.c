/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
float slaRvlg(float qfoo,float qbar){static float qbaz[3]={-
148.23284f,133.44888f,-224.09467f};float Q0[3];slaCs2c(qfoo,
qbar,Q0);return slaVdv(qbaz,Q0);}
