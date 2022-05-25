$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$!+			E R S _ R E L E A S E . C O M
$!
$!  Function:
$!    Perform a release of the ERSs sub-system
$!
$!  Copyright (c) Anglo-Australian Telescope Board, 1995.
$!  Not to be used for commercial purposes without AATB permission.
$!
$!     @(#) $Id: ACMM:DramaErs/ers_release.com,v 3.19 09-Dec-2020 17:17:02+11 ks $
$!
$!  History:
$!    20-Oct-92 - TJF - Original version.
$!    22-Dec-1992 - TJF - Move from ADAM/WHAM to DRAMA
$!    19-Feb-1992 - TJF - USe .release for unix stuff.  use DRAMAUTIL_DIR
$!				instead of INSTLIB_DIR for release.tpu
$!
$! First, get the version number from the user.
$!
$ directory dramadisk:[ers]*.*;0
$ inquire ver_type "Version type (e.g. D or R etc) "
$ inquire ver_main_number "Version main number (e.g. the 0 in D0.1.2) "
$ inquire ver_sub_number  "Version sub  number (e.g. the 1 in D0.1.2) "
$ inquire ver_subsub_number  "Version sub sub number (e.g. the 2 in D0.1.2) "
$
$! Create cms class number (x.n.n.n) and version string (xn_n_n)
$!
$ cms_class = ver_type + ver_main_number
$ if ver_sub_number .nes. "" then cms_class = cms_class + "." + ver_sub_number
$ if ver_subsub_number .nes. "" then cms_class = cms_class + "." + ver_subsub_number
$!
$ ver_string = ver_type + ver_main_number
$ if ver_sub_number .nes. "" then ver_string = ver_string + "_" + ver_sub_number
$ if ver_subsub_number .nes. "" then ver_string = ver_string + "_" + ver_subsub_number
$!
$!
$!  Create the release directory
$!
$ define vmsrel_dir dramadisk:[ers.'ver_string']
$ define unixrel_dir dramadisk:[ers.release]
$ create/directory vmsrel_dir
$ create/directory unixrel_dir
$!
$ inquire flag "Skip Cms management([N],Y)"
$ if flag .eqs. "Y" .or flag .eqs. "y" THEN goto NOCMS
$!
$! Create remark for cms operations
$!
$ remark = """Release of version "+ver_string+""""
$!
$ write sys$output ""
$ write sys$output "CMS class ", cms_Class, " will be created for release ", ver_string
$ write sys$output ""
$!
$!	Get the reason for the release
$!
$ write sys$output ""
$ inquire reason "Reason for release (class creation remark)"
$ reason = """"+reason+""""
$!
$ envfile = f$search("ERS.ENV")
$ if envfile .nes. "" 
$   then
$	write sys$output "An environment file, ",envfile, " exists!"
$	write sys$output "Is this the one you want associated with class - ",cms_class
$	inquire response "([Y]/N) "
$	if response .eqs. "N" then envfile = ""
$   else 
$       write sys$output "The environment file ERS.ENV is not in the default search path"
$ endif
$ if envfile .eqs. ""		
$   then
$       write sys$output "Shall I - "
$	write sys$output "          A. Abort the release so you can create an environment file"
$	write sys$output "          C. Create an environment file using current envonment"
$	write sys$output "          N. Ignore the environment file"
$	inquire response "(A/[C]/N) "
$	if response .eqs. "A" then exit
$       write sys$output ""
$       write sys$output "I am Starting the hard work. This will take some time"
$       write sys$output "Take a break, have a cup of tea, or something!"
$       write sys$output ""
$	write sys$output "Creating environment save file"
$	if response .nes. "N" 
$	  then 
$	    a = f$verify(0)
$	    DRAMA_ENVSAVE 'reason' ERS.ENV
$	    a = f$verify(a)
$	    envfile = "ERS.ENV"
$	endif
$ else
$     write sys$output ""
$     write sys$output "I am Starting the hard work. This will take some time"
$     write sys$output "Take a break, have a cup of tea, or something!"
$     write sys$output ""
$ endif
$!
$!  Get the user name
$!
$ user = f$getjpi("","USERNAME")
$ user = f$element(0," ",user)
$!
$!	Write the version string and user name to a file so that it 
$!      can be used by the editor
$!
$ open/write file ERS_VER.TMP 
$ write file ver_string
$ write file user
$ close file
$!
$!
$!
$ write sys$output  "Reserving/fetching cms elements to be modified."
$!
$!
$!
$! Set the cms library.
$!
$ save_cmslib:= 'f$logical("cms$lib")'
$ CMS set library/nolog dramadisk:[ERS.source.cms]
$ ON CONTROL_Y THEN GOTO RESTORE_CMS
$!
$! Reserve cms elements to be modified.  
$!
$ cms reserve/nolog ers_dramastart.com,ers_dramastart.csh 'remark'
$ cms reserve/nolog makefile.vw68k,makefile.gnu 'remark'
$ cms reserve/nolog ers.env/output=NL: 'remark'
$!
$! Modify ers_dramastart.com ers_dramastart.csh and makefiles
$!
$ write sys$output  "Modifing files for release ",ver_string
$ define/useR_mode sys$input sys$command
$ editor/tpu/init=ers_dir:ers_release.eve/command=dramautil_dir:release.tpu ers_ver.tmp
$ delete/nolog ers_ver.tmp;0
$!
$ write sys$output  "Replacing modified cms elements."
$!
$! Replace  ers_dramastart.com, ers_dramastart.csh and makefiles
$!
$ cms replace/nolog ers_Dramastart.com,ers_dramastart.csh ""
$ cms replace/nolog makefile.vw68k,makefile.gnu
$!
$! Replace if necessary ers.env
$!
$ if envfile .eqs. "" 
$   then
$     write sys$output "Warning, there is no environment save file for class ",cms_class
$     cms unreserve ers.env "No environment file for this version"
$   else
$     cms replace/nolog ers.env/input='envfile' ""
$ endif
$!
$! Create class and insert files.
$!
$ write sys$output  "Creating class ",cms_class," and inserting elements"
$ cms create class/nolog 'cms_class' 'reason'	
$ cms insert generation/nolog release 'cms_class' 'remark'
$!
$!  Copy release files, Don't stop on error
$!
$ nocms:
$ set noon
$ 
$
$!  Software development tools
$
$ copy/log ers_dir:ers.mms vmsrel_dir:
$ copy/log ers_dir:ers_dramastart.com vmsrel_dir:
$ copy/log ers_dir:ers_devstart.com vmsrel_dir:
$ copy/log ers_dir:ers_release.com vmsrel_dir:
$ copy/log ers_dir:ers.olb vmsrel_dir:
$ copy/log ers_dir:ers.olb_deb vmsrel_dir:
$ copy/log ers_dir:ers_star.olb vmsrel_dir:
$ copy/log ers_dir:ers_star.olb_deb vmsrel_dir:
$!
$
$!  Include files required by users
$!
$ copy/log ers_dir:ers.h vmsrel_dir:
$ copy/log ers_dir:ers_err.h vmsrel_dir:
$ copy/log ers_dir:ers_err_msgt.h vmsrel_dir:
$!
$! Error object file
$!
$ copy/log ers_dir:ers_err.obj vmsrel_dir:
$!
$!
$! Unix source, so ers can be build on a unix machine or for vxworks.
$!
$ copy/log ers_dir:ers.c unixrel_dir:
$ copy/log ers_dir:erssprintf.c unixrel_dir:
$ copy/log ers_dir:ersvsprintf.c unixrel_dir:
$ copy/log ers_dir:ers_star.c unixrel_dir:
$ copy/log ers_dir:ers.h unixrel_dir:
$ copy/log ers_dir:ers_err.msg unixrel_dir:
$ copy/log ers_dir:makefile.gnu unixrel_dir:
$ copy/log ers_dir:makefile.vw68k unixrel_dir:
$ copy/log ers_dir:ers_dramastart.csh unixrel_dir:
$ copy/log ers_dir:ers_star_link.unix unixrel_dir:
$ copy/log ers_dir:ers_moveunix. unixrel_dir:
$ 
$!  Restore the cms library
$RESTORE_CMS:
$ cmslib:= 'f$logical("cms$lib")'
$ if (save_cmslib .nes. "").and.(save_cmslib .nes. cmslib) 
$   then 
$	cms set library/nolog 'save_cmslib'
$ endif
$
$!  Purge the release directory and deassign logical names
$
$ purge/log vmsrel_dir:*.*
$ purge/log unixrel_dir:*.*
$ deassign vmsrel_dir
