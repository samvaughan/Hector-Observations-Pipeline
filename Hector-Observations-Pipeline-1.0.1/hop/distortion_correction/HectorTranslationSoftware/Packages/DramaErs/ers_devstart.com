$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$!+                     E R S  _ D E V S T A R T . C O M
$!
$!  Function:
$!    Set up for developing the ERS sub-system
$!
$!  Description:
$!    This command file sets up from scratch for working on the ers sub-system.
$!
$!  Copyright (c) Anglo-Australian Telescope Board, 1995.
$!  Not to be used for commercial purposes without AATB permission.
$!
$!     @(#) $Id: ACMM:DramaErs/ers_devstart.com,v 3.19 09-Dec-2020 17:17:02+11 ks $
$!
$!  History:
$!    20-Oct-92 - TJF - Original version.
$!    22-Dec-92 - TJF - Change from ADAM/WHAM to DRAMA.  Only startup ADAM
$!			if P1 is defined to be ADAM.
$
$ say = "write sys$output"
$!
$ p1 = f$edit(p1,"UPCASE")
$ if P1 .eqs. "ADAM"
$  then
$!  Get ready for ADAM system development. This looks complicated but aims
$!  to avoid unnecessary repetition of what has already been done. It is
$!  possible that this might cease to work with new versions of ADAM.
$
$    if f$type(adamstart) .eqs. ""
$    then
$        say "Executing Starlink login..."
$        @ssc:login
$        if f$type(adamdev) .nes. "" then delete/global/symbol adamdev
$    endif
$    if f$type(adamdev) .eqs. ""
$    then
$        say "Executing ADAM startup..."
$        adamstart
$        if f$type(compifl) .nes. "" then delete/global/symbol compifl
$    endif
$    if f$type(compifl) .eqs. ""
$    then
$        say "Setting up for ADAM development..."
$        adamdev
$        if f$trnlnm("uface_dir") .nes. "" then deassign/process uface_dir
$    endif
$    if f$trnlnm("uface_dir") .eqs. ""
$    then
$        say "Setting up for ADAM system development..."
$        @adam_sys:sysdev
$        deassign com_dir
$        deassign nbs_dir
$        define out_log 'f$trnlnm("in_log")'
$        if f$trnlnm("gct_dir") .nes. "" then deassign/job gct_dir
$    endif
$    if f$trnlnm("gct_dir") .eqs. ""		! Use gcT_dir since ers_dir
$    then						! is in the system table
$        say "Executing AAO local ADAM startup..."
$        ladamstart
$    endif
$ else
$! Just execute the DRAMA startup
$     if f$trnlnm("DRAMASTART") .eqs. ""
$     then
$       say "Executing DRAMA startup..."
$	dramastart
$     endif
$ endif
$
$!  Set up for using the sub-system.
$
$ say "Defining ers_DIR..."
$ @dramadisk:[ers]ers_dramastart
$ olddir = ""
$ index  = 0
$ next:
$    equiv = f$trnlnm ("ers_dir",,index)
$    if equiv .nes. ""
$    then
$       olddir = olddir + "," + equiv
$       index  = index + 1
$       goto next
$    endif
$ define/job ers_dir 'f$trnlnm("sys$disk")''f$directory()',-
		     dramadisk:[ers.source.ref]'olddir'

$ cms set library dramadisk:[ers.source.cms]
$
$!  This logical name is used by TJF's version of LSE to find the
$!  include files
$define/job lse_c_include ers_dir
$
$ say "Ready to develop the ers sub-system."
