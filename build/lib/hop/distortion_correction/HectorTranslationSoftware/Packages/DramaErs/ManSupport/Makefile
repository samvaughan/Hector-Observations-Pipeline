# Makefile used to create the ERS Man pages.  Relies on 
# ../makefile.release_files to define the files which should be documented.
#
#     @(#) $Id: ACMM:DramaErs/ManSupport/Makefile,v 3.19 09-Dec-2020 17:17:02+11 ks $
#
#
include ../makefile.release_files

MAN_DIR=man
REL_DIR=..


man : man3 

# Routine man pages
man3 :
	(DATE=`date +"%d-%h-19%y"` 				;\
	cd $(REL_DIR) 						;\
	$(WFLMAN) -oman -ners  -O"3 $$DATE DRAMA DRAMA" $(ERS_SOURCE)	;\
	if [ ! -d $(MAN_DIR) ] ; then  mkdir $(MAN_DIR); fi             ;\
	if [ ! -d $(MAN_DIR)/man3 ] ; then  mkdir $(MAN_DIR)/man3; fi   ;\
	for I in *.man ; do                                              \
	    a=`echo $$I | sed 's/\.man/\.3/'` 	                        ;\
	    tbl $$I > $(MAN_DIR)/man3/$$a 	                        ;\
	    /bin/rm -f $$I			   	                ;\
	done                                                            ;\
	mv ers.man_summary $(MAN_DIR)/man3/ErsIntro.3)

catman :
	(cd $(REL_DIR) ; catman -w -M $(MAN_DIR))

