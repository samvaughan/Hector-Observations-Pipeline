# Makefile used to create the SDS Man pages.  Relies on 
# ../makefile.release_files to define the files which should be documented.
#
#     @(#) $Id: ACMM:sds/ManSupport/Makefile,v 3.94 09-Dec-2020 17:15:24+11 ks $
#
#
include ../makefile.release_files

MAN_DIR=man
REL_DIR=..


man : man1 man3 


# Routine man pages
man3 :
	(DATE=`date +"%d-%h-19%y"` 				;\
	cd $(REL_DIR) 						;\
	$(WFLMAN) -oman -nsds  -O"3 $$DATE DRAMA DRAMA" $(MAN3_SRCS)	;\
	if [ ! -d $(MAN_DIR) ] ; then  mkdir $(MAN_DIR); fi             ;\
	if [ ! -d $(MAN_DIR)/man3 ] ; then  mkdir $(MAN_DIR)/man3; fi   ;\
	for I in *.man ; do                                              \
	    a=`echo $$I | sed 's/\.man/\.3/'` 	                        ;\
	    tbl $$I > $(MAN_DIR)/man3/$$a 	                        ;\
	    /bin/rm -f $$I			   	                ;\
	done                                                            ;\
	mv sds.man_summary $(MAN_DIR)/man3/SdsIntro.3)

# Program man pages
man1 :
	(DATE=`date +"%d-%h-19%y"` 				        ;\
	cd $(REL_DIR) 						        ;\
	$(WFLMAN) -oman --nsds -O"1  $$DATE DRAMA DRAMA" $(MAN1_SRCS)	;\
	if [ ! -d $(MAN_DIR) ] ;      then  mkdir $(MAN_DIR); fi        ;\
	if [ ! -d $(MAN_DIR)/man1 ] ; then  mkdir $(MAN_DIR)/man1; fi   ;\
	for I in *.man ; do                                              \
	    a=`echo $$I | sed 's/\.man/\.1/'` 	                        ;\
	    tbl $$I > $(MAN_DIR)/man1/$$a 	                        ;\
	    /bin/rm -f $$I			   	                ;\
	done                                                            ;\
	mv sds.man_summary $(MAN_DIR)/man1/SdsIntro.1)

catman :
	(cd $(REL_DIR) ; catman -w -M $(MAN_DIR))

