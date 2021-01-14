$!	ERS_DRAMASTART, called to set up symbols and logical
$!	names for the ERS library
$!
$!
$!  Copyright (c) Anglo-Australian Telescope Board, 1995.
$!  Not to be used for commercial purposes without AATB permission.
$!
$!     @(#) $Id: ACMM:DramaErs/ers_dramastart.com,v 3.19 09-Dec-2020 17:17:02+11 ks $
$!
$!   History:
$!	20-Oct-1992 - TJF - Original version.
$!      21-OCT-92 - TJF - Goto version T1
$!      21-OCT-92 - TJF - Goto version T1
$!      22-Dec-1992 - TJF - Move from ADAM/WHAM to DRAMA
$!      19-FEB-93 - TJF - Goto version D0.0
$!	22-Jun-93 - TJF - Goto style required by imake stuff.
$!      14-Apr-94 - TJF - Support alpha.
$!   End history: 	This line required by the automatic release editor. 
$!
$! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
$!
$ RELEASE = "111"
$ TARGET = f$getsys("ARCH_NAME") + "VMS"
$ DEV = TARGET
$ DEFINE ERS_DIR DRAMADISK:[RELEASE.ERS.'RELEASE'],DRAMADISK:[RELEASE.ERS.'RELEASE'.'TARGET']
$ DEFINE ERS_LIB DRAMADISK:[RELEASE.ERS.'RELEASE'.'TARGET']
$ DEFINE/JOB ERS_DEV DRAMADISK:[RELEASE.ERS.'RELEASE'.'DEV']
$!
