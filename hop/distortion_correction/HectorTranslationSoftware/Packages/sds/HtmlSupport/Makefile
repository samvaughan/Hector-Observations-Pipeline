# Makefile used to create the SDS Html documentation.  Relies on 
# ../makefile.release_files to define the files which should be documented.
#
#
#     @(#) $Id: ACMM:sds/HtmlSupport/Makefile,v 3.94 09-Dec-2020 17:15:24+11 ks $
#
#
include ../makefile.release_files

REL_DIR=..
HTML_DIR=html
DRAMASRC_HTML=../../DramaHtml

html : dramahtml html1 html3

# Ensure the DRAMA standard html stuff is there.  Note, we need to pass
# the value of $(HTML_DIR) to another make file, but need to ensure
# that if relative, we prefix the current default directory
dramahtml :
	if [ ! -d $(HTML_DIR) ] ; then  mkdir $(HTML_DIR); fi;
	( DIR=$(HTML_DIR) ; FIRSTCHAR=`echo $(HTML_DIR) | cut -c 1` ; \
	 if [ "$$FIRSTCHAR" != "/" ]; then DIR=`pwd`/$(HTML_DIR); fi; \
	 cd $(DRAMASRC_HTML) ; $(MAKE) HTML_DIR=$$DIR )

#
# Contstruct the routine pages.  This is a mess at times, due particulary to
# directory management.
#
html3 : sds.rout.wflman.opts arg.rout.wflman.opts routsum_prefix.html.incl
	if [ $(REL_DIR) != "." ] ; then  /bin/cp -f routsum_prefix.html.incl arg_prefix.html.incl $(REL_DIR) ; fi
	(a=`pwd` ; cd $(REL_DIR) ; $(WFLMAN) -nsds_routines -ohtml -Ooptfile=$$a/sds.rout.wflman.opts $(MAN3_SRCS_SDS))
	(a=`pwd` ; cd $(REL_DIR) ; $(WFLMAN) -narg_routines -ohtml -Ooptfile=$$a/arg.rout.wflman.opts $(MAN3_SRCS_ARG))
	if [ $(REL_DIR) != "." ] ; then  /bin/rm -f $(REL_DIR)/routsum_prefix.html.incl $(REL_DIR)/arg_prefix.html.incl ; fi
	if [ ! -d $(HTML_DIR)/routines ] ; \
			     then  mkdir $(HTML_DIR)/routines; fi
	for I in $(REL_DIR)/*.html ; do \
	    mv $$I $(HTML_DIR)/routines 	;\
	done
	mv $(HTML_DIR)/routines/*.summary.html $(HTML_DIR)
	mv $(HTML_DIR)/routines/*.frames.html $(HTML_DIR)
	/bin/rm -f $(HTML_DIR)/routines/*.intro.html


#
# Construct the program pages.
#
html1 : sds.prog.wflman.opts progsum_prefix.html.incl
	if [ $(REL_DIR) != "." ] ; then  /bin/cp -f progsum_prefix.html.incl $(REL_DIR) ; fi
	( a=`pwd` ; cd $(REL_DIR) ; $(WFLMAN) -nsds_programs -ohtml -Ooptfile=$$a/sds.prog.wflman.opts  $(MAN1_SRCS))
	if [ $(REL_DIR) != "." ] ; then  /bin/rm -f $(REL_DIR)/progsum_prefix.html.incl  ; fi
	if [ ! -d $(HTML_DIR)/programs ] ; \
			     then  mkdir $(HTML_DIR)/programs; fi
	for I in $(REL_DIR)/*.html ; do \
	    mv $$I $(HTML_DIR)/programs	;\
	done
	mv $(HTML_DIR)/programs/*.summary.html $(HTML_DIR)
	mv $(HTML_DIR)/programs/*.frames.html $(HTML_DIR)
	/bin/rm -f $(HTML_DIR)/programs/*.intro.html


# Use doxygen to generate C++ documentation in the directory ../cpp_html.
doxygen : ../arg.h ../sds.h doxygen.config
	doxygen doxygen.config
