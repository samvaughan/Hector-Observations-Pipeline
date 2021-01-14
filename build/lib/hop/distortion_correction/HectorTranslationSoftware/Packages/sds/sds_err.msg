!+ SDS_ERR.MSG
!
! @(#) $Id: ACMM:sds/sds_err.msg,v 3.94 09-Dec-2020 17:15:24+11 ks $
!
!  Error codes for SDS
!
!  History:
!    Jeremy Bailey, AAO, 29th Jan 1992  -  Initial version
!    Jeremy Bailey, AAO, 31st Jan 1992  -  Add codes for sdsutil package
!    Jeremy Bailey, AAO, 11th Feb 1992  -  Add SDS type
!    Tony  Farrell, AAO, 03rd Mar 1995  -  Add codes for Sds Compiler.
!    Jeremy Bailey, AAO. 30th Apr 1997  -  Add TIMEOUT
!

.TITLE		SDS_ERR	SDS Error Codes
.FACILITY	SDS,1824/PREFIX=SDS__
.IDENT		"Version 1.0"


.SEVERITY	ERROR
.BASE		100

BADID           "Illegal Identifier"
NOMEM           "Insufficient Memory"
NOTSTRUCT       "Object is not a structure"
NOITEM          "No such item"
NOTPRIM         "Object is not a primitive"
TOOLONG         "Data too long"
UNDEFINED       "Data value is undefined"
LONGNAME        "Object name is too long"
EXTRA           "Too much extra data"
INVCODE         "Invalid type code"
INVDIMS         "Invalid dimensions"
NOTARRAY        "Not a structure array"
INDEXERR        "Indices invalid"
ILLDEL          "Object cannot be deleted"
NOTSDS          "Not a valid SDS object"
NOTTOP          "Not a top level object"
EXTERN          "Illegal operation on external object"
NOTARR          "Object is not an array"
VERSION         "Object created by an incompatible SDS version"
FOPEN           "Error opening file"
FWRITE          "Error writing to a file"
FREAD           "Error reading a file"
TYPE            "Object has incorrect type"
TIMEOUT         "Timeout on semaphore access"


INVPRIMTYPE 	"Invalid primitive type code"
INVSTRUCTDEF    "Invalid structure definition"
SYNTAX 		"Parsing syntax error"
INVSTRUCTURE 	"Invalid structure"
INVTYPETYPE 	"Invalid typedef type"
STRUCTMULTDEF   "Multipled defined structure"
INVINT		"Invalid integer"
INVTYPE		"Invalid type"
INVINPUT	"Invalid input"
STRUCTARRAY 	"Array of structures"
MAXDIMS 	"Maximum number of dimensions"
NOINPUT		"Empty input file"

TCL_ARGERR	"Error in command arguments"
TCL_CMDERR	"Unspecified error in Tcl command"
TCL_CMD		"Error in Tcl Command Action"

TESTERR         "SDS Test failure"

INVITEM         "Sds Item definition does not match the definition expected by the appliaction code"
CHK_LEAK        "SdsCheck() indicates a SDS data leak"
CHK_RELEASED    "SdsCheck() indicates an unexpected release of an SDS id"
CHK_FREE        "SdsFreeIdAndCheck() indicates SDS misuse (missing SdsDelete() or SdsReadFree() call)"

SEM_INIT_ERR    "SDS Semaphore/Mutex initialisation error"
SEM_TAKE_ERR    "Failed to take SDS Semaphore/Mutex"

AMBIGIOUS_TYPE  "Ambigous type name - when compling C to SDS structure"
! Remember to update sdsutil.c/SdsErrorCodeString().
.END
