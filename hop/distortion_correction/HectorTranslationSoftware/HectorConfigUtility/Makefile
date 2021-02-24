#
#   This is a makefile for the HectorConfigUtil program.
#
#   It assumes that this is being run from the directory containing the
#   main source files for HectorConfigUtil - there aren't many - and
#   that there is a Packages directory alongside that with copies of
#   the SDS, ERS and SLALIB source exactly as released by AAO, along
#   with copies of the distributions for the CFITSIO and WCSLIB libraries
#   and a miscellaneous directory that holds some more specialised
#   source files collected from various other projects (like the AAO TCS
#   and 2dF projects) where only the odd routine is needed.
#
#   This makefile will build the whole HectorConfigUtil program from
#   these source files as needed.
#
#   The main targets are:
#
#   HectorConfigUtil - The program itself. This is the default target.
#   clean            - Cleans the built files in the current directory,
#                      leaving the files for the various packages untouched.
#   all_clean        - Reduces everything in the current directory and the
#                      packages to just the original source files. (Used just
#                      prior to creating a tar file for release.)
#
#   No files are created outside the directory structure released with the
#   HectorConfigUtil program. It builds local copies of all its required
#   packages, and I believe all it requires are standard programming utilities
#   such as 'make' and a C++11 compiler. By the same token, it does not install
#   any copies of those libraries, so will not interfere with any existing
#   installations that might already be on the target machine.
#
#   With the exception of SLALIB, CFITSIO and WCSLIB, all the packages in
#   the Packages directory have their own makefile called Makefile.standalone,
#   and this current makefile simply runs make in the various package
#   directories as needed using those standalone makefiles.
#
#   An exception is SLALIB, where the standard makefile provided always
#   builds the library and insists on releasing this and the include files
#   into a release directory structure which defaults to being in the
#   user's top level directory. To avoid this, this makefile forces the
#   release files to go into a structure in the Packages directory called
#   'slalib'. (The source files are in slalib_o, since this is using the
#   obfuscated version of the files for licencing reasons.) This complicates
#   slightly the way the slalib rules are set up here.
#
#   The installation procedures for CFITSIO uses a configure script to create
#   the Makefile, so there is a separate rule in this current makefile to run
#   configure first before the 'make' step, which uses the default Makefile
#   created by configure.
#
#   For WCSLIB, the process is similar to cfitsio, needing a configure
#   step, but this is a bit messier because of wcslib's preference for
#   gmake, meaning the test for configure having been run depends on the
#   existence of the makedefs file it creates, rather than a Makefile.
#   Telling wcslib's configure to ignore pgplot and cfitsio stops it trying
#   to build its test programs, because it's hard to point that part of the
#   build to the local cfitsio directory - it doesn't like relative file paths.
#
#   Author:
#      Keith Shortridge, (KS), K&V.
#
#   History:
#      10th Dec 2020. Original version. KS.
#      12TH JAN 2021. Extended to include the use of Profit Mask files to
#                     check sky contamination, which means including the
#                     CFITSIO and WCSLIB libraries. KS.

#   Directory layout - note the separate SLALIB release directories for the
#   library and the include files. DRAMA_DIR holds copies of some standard
#   files (status.h and drama.h) used by various AAO software, and the SDS
#   release has copies of these in a Standalone sub-sirectory. Since we don't
#   install the WCSLIB include files and library, they are used directly from
#   the C sub-directory in which they are built.

PACKAGES_DIR = ../Packages
MISC_DIR = $(PACKAGES_DIR)/Misc
SDS_DIR = $(PACKAGES_DIR)/sds
DRAMA_DIR = $(SDS_DIR)/Standalone
ERS_DIR = $(PACKAGES_DIR)/DramaErs
CFITSIO_DIR = $(PACKAGES_DIR)/cfitsio
WCSLIB_DIR = $(PACKAGES_DIR)/wcslib-5.16
WCSLIB_LIB_DIR = $(WCSLIB_DIR)/C
WCSLIB_INC_DIR = $(WCSLIB_DIR)/C
SLALIB_SRC_DIR = $(PACKAGES_DIR)/slalib_o
SLALIB_INC_DIR = $(PACKAGES_DIR)/slalib/include
SLALIB_LIB_DIR = $(PACKAGES_DIR)/slalib/lib

#  The main released slalib include file

SLALIB_INCL = $(SLALIB_INC_DIR)/slalib.h

#  Compilation flags

INC = -I $(MISC_DIR) -I $(SDS_DIR) -I $(SLALIB_INC_DIR) \
      -I $(CFITSIO_DIR) -I $(WCSLIB_DIR) -I $(WCSLIB_INC_DIR) -I $(DRAMA_DIR)
CCC = g++
CCFLAGS = -O $(INC) -std=c++11 -Wall

#  The individual object modules used directly from the miscellaneous
#  directory, and the various library files used.

MISC_OBJ = $(MISC_DIR)/ArrayManager.o $(MISC_DIR)/gen_qfmed.o \
      $(MISC_DIR)/TcsUtil.o $(MISC_DIR)/tdfxy.o $(MISC_DIR)/CommandHandler.o \
      $(MISC_DIR)/ReadFilename.o $(MISC_DIR)/Wildcard.o

LIBS = $(SDS_DIR)/libsds.a $(ERS_DIR)/libers.a $(SLALIB_LIB_DIR)/libsla.a \
         $(CFITSIO_DIR)/libcfitsio.a $(WCSLIB_LIB_DIR)/libwcs-5.16.a

#  Local object files specific to HectorConfigUtil

OBJ = HectorConfigUtil.o HectorRaDecXY.o ProfitSkyCheck.o

#  Default target - the executable program

All: HectorConfigUtil

#  Compilation and build rules for the files in the HectorConfigUtil directory

HectorConfigUtil : $(LIBS) $(OBJ) $(MISC_OBJ)
	$(CCC) $(CCFLAGS) -o HectorConfigUtil $(OBJ) $(MISC_OBJ) $(LIBS) -lpthread

HectorConfigUtil.o : HectorConfigUtil.cpp HectorStructures.h HectorRaDecXY.h $(SLALIB_INCL)
	$(CCC) $(CCFLAGS) -c HectorConfigUtil.cpp

HectorRaDecXY.o : HectorRaDecXY.cpp HectorRaDecXY.h $(SLALIB_INCL)
	$(CCC) $(CCFLAGS) -c HectorRaDecXY.cpp

ProfitSkyCheck.o : ProfitSkyCheck.cpp ProfitSkyCheck.h $(WCSLIB_INCL)
	$(CCC) $(CCFLAGS) -c ProfitSkyCheck.cpp

#  Building the various packages from source using their own makefiles
#  and or ./configure systems.

$(SLALIB_INCL) $(SLALIB_LIB_DIR)/libsla.a :
	$(MAKE) -C $(SLALIB_SRC_DIR) INSTALL_DIR=../slalib

$(SDS_DIR)/libsds.a :
	$(MAKE) -C $(SDS_DIR) -f Makefile.standalone libsds.a

$(ERS_DIR)/libers.a :
	$(MAKE) -C $(ERS_DIR) -f Makefile.standalone libers.a

$(CFITSIO_DIR)/libcfitsio.a : $(CFITSIO_DIR)/Makefile
	$(MAKE) -C $(CFITSIO_DIR) libcfitsio.a

$(CFITSIO_DIR)/Makefile :
	cd $(CFITSIO_DIR); ./configure

$(WCSLIB_LIB_DIR)/libwcs-5.16.a : $(WCSLIB_DIR)/makedefs
	$(MAKE) -C $(WCSLIB_DIR)

$(WCSLIB_DIR)/makedefs :
	cd $(WCSLIB_DIR); ./configure --without-cfitsio --without-pgplot

$(MISC_OBJ) :
	$(MAKE) -C $(MISC_DIR) -f Makefile.standalone

#  Cleaning up. Note that all_clean has to explicitly clean out the
#  locally released versions of SLALIB as well as the intermediate files.
#  The WCSLIB 'cleanest' target gets rid of almost everything bar the original
#  source, but has the annoying side-effect of re-running the configure step
#  in order to do so; it does no harm, but you feel it should be unnecessary.

clean ::
	$(RM) HectorConfigUtil *.o

all_clean ::
	-$(MAKE) -C $(SDS_DIR) -f Makefile.standalone clean
	-$(MAKE) -C $(ERS_DIR) -f Makefile.standalone clean
	-$(MAKE) -C $(SLALIB_SRC_DIR) clean
	-$(MAKE) -C $(MISC_DIR) -f Makefile.standalone clean
	-$(MAKE) -C $(CFITSIO_DIR) clean
	-$(MAKE) -C $(WCSLIB_DIR) cleanest
	-$(RM) $(CFITSIO_DIR)/Makefile
	-$(RM) $(SLALIB_LIB_DIR)/libsla.a
	-$(RM) $(SLALIB_INC_DIR)/*.h
	-$(RM) $(WCSLIB_LIB_DIR)/libwcs*
	-$(RM) HectorConfigUtil *.o
