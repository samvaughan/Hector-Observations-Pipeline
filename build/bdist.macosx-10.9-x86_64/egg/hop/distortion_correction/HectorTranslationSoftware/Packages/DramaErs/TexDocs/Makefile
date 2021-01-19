# Simple makefile used to create the ERS Tex documentation.  Relies on 
# ../makefile.release_files to define the files which should be documented.
#
#     @(#) $Id: ACMM:DramaErs/TexDocs/Makefile,v 3.19 09-Dec-2020 17:17:02+11 ks $
#
#

include ../makefile.release_files

DOC_DIR=.
REL_DIR=..


doc : 
	(cd $(REL_DIR) ; $(WFLMAN) $(ERS_SOURCE))
	(cd $(REL_DIR) ; texfilter TexDocs/ers_spec_4.texi> $(DOC_DIR)/ers_spec_4.tex)
	- (cd $(REL_DIR) ; /bin/rm -f *.tex_descr *.tex_intro *.tex_summary)

