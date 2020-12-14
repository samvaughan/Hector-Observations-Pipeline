/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaDrange(double qfoo){double qbar;qbar=dmod(qfoo,
D2PI);return(fabs(qbar)<DPI)?qbar:qbar-dsign(D2PI,qfoo);}
