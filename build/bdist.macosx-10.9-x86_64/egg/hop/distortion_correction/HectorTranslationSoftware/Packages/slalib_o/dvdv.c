/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaDvdv(double qfoo[3],double qbar[3]){return qfoo[0]
*qbar[0]+qfoo[1]*qbar[1]+qfoo[2]*qbar[2];}
