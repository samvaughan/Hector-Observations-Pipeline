/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
#include <ctype.h>
double slaEpco(char qfoo,char qbar,double qbaz){double Q0;
int qfobar;qfobar=toupper((int)qfoo);if(qfobar==toupper((int
)qbar)){Q0=qbaz;}else{if(qfobar==(int)'B'){Q0=slaEpb(
slaEpj2d(qbaz));}else{Q0=slaEpj(slaEpb2d(qbaz));}}return(Q0)
;}
