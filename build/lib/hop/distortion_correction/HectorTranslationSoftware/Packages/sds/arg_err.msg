!+  ARG_ERR.MSG
!
! @(#) $Id: ACMM:sds/arg_err.msg,v 3.94 09-Dec-2020 17:15:24+11 ks $
!
!  Error codes for the argument library
!
!  History:
!    Jeremy Bailey, AAO, 08th Apr 1992  -  Initial version
!

.TITLE		ARG_ERR	ARG Error Codes
.FACILITY	ARG,1825/PREFIX=ARG__
.IDENT		"Version 1.0"


.SEVERITY	ERROR
.BASE		100

CNVERR           "Type Conversion Error"
NOTSCALAR        "Object is not a scalar"
NOTSTRING        "Object is not a string"
MALLOCERR	 "Malloc failed"
! Remember to update sdsutil.c/SdsErrorCodeString().

.END
