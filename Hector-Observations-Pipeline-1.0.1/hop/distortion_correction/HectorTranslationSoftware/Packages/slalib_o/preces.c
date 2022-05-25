/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
#include <ctype.h>
void slaPreces(const char qfoo[3],double qbar,double qbaz,
double*Q0,double*qfobar){double q1[3][3],q2[3],qfoobar[3];if
((toupper((int)qfoo[0])!='F')||(toupper((int)qfoo[1])!='K')
||((int)qfoo[2]!='4'&&(int)qfoo[2]!='5')){*Q0=-99.0;*qfobar=
-99.0;}else{if((int)qfoo[2]=='4')slaPrebn(qbar,qbaz,q1);else
 slaPrec(qbar,qbaz,q1);slaDcs2c(*Q0,*qfobar,q2);slaDmxv(q1,
q2,qfoobar);slaDcc2s(qfoobar,Q0,qfobar);*Q0=slaDranrm(*Q0);}
}
