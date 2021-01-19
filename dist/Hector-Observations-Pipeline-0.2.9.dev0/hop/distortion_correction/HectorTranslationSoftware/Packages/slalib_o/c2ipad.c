/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaC2ipad(CIpars*qfoo){int qbar,qbaz;for(qbar=0;qbar<3;
qbar++){for(qbaz=0;qbaz<3;qbaz++){qfoo->bpn[qbar][qbaz]=(
qbar==qbaz?1.0:0.0);}}}
