/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaDranrm(double qfoo){double qbar;qbar=dmod(qfoo,
D2PI);return(qbar>=0.0)?qbar:qbar+D2PI;}
