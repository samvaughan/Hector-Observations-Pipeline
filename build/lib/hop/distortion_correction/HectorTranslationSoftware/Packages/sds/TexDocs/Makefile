# Simple makefile used to create the SDS Tex documentation.  Relies on 
# ../makefile.release_files to define the files which should be documented.
#
#     @(#) $Id: ACMM:sds/TexDocs/Makefile,v 3.94 09-Dec-2020 17:15:24+11 ks $
#
#

include ../makefile.release_files

DOC_DIR=..
REL_DIR=..


doc : 
	(a=`pwd` ; cd $(DOC_DIR) ; b=`pwd` ; if [ $$a != $$b ] ; then cd $$a ; cp -f sds.tex $(DOC_DIR) ; fi)
