# Makefile used to create the ERS Html documentation.  Relies on 
# ../makefile.release_files to define the files which should be documented.
#
#
#     @(#) $Id: ACMM:DramaErs/HtmlSupport/Makefile,v 3.19 09-Dec-2020 17:17:02+11 ks $
#
#
include ../makefile.release_files

REL_DIR=..
HTML_DIR=html
DRAMASRC_HTML=../../DramaHtml

html : dramahtml html3

# Ensure the DRAMA standard html stuff is there.  Note, we need to pass
# the value of $(HTML_DIR) to another make file, but need to ensure
# that if relative, we prefix the current default directory
dramahtml :
	if [ ! -d $(HTML_DIR) ] ; then  mkdir $(HTML_DIR); fi;
	( DIR=$(HTML_DIR) ; FIRSTCHAR=`echo $(HTML_DIR) | cut -c 1` ; \
	 if [ "$$FIRSTCHAR" != "/" ]; then DIR=`pwd`/$(HTML_DIR); fi; \
	 cd $(DRAMASRC_HTML) ; $(MAKE) HTML_DIR=$$DIR)

#
# Contstruct the routine pages.  This is a mess at times, due particulary to
# directory management.
#
html3 : ers.wflman.opts sumprefix.html.incl
	if [ $(REL_DIR) != "." ] ; then  /bin/cp -f sumprefix.html.incl $(REL_DIR) ; fi
	(a=`pwd` ; cd $(REL_DIR) ; $(WFLMAN) -ners -ohtml -Ooptfile=$$a/ers.wflman.opts $(ERS_SOURCE))
	if [ $(REL_DIR) != "." ] ; then  /bin/rm -f $(REL_DIR)/sumprefix.html.incl; fi
	if [ ! -d $(HTML_DIR)/routines ] ; \
			     then  mkdir $(HTML_DIR)/routines; fi
	for I in $(REL_DIR)/*.html ; do \
	    mv $$I $(HTML_DIR)/routines 	;\
	done
	mv $(HTML_DIR)/routines/*.summary.html $(HTML_DIR)
	mv $(HTML_DIR)/routines/*.frames.html $(HTML_DIR)
	/bin/rm -f $(HTML_DIR)/routines/*.intro.html


