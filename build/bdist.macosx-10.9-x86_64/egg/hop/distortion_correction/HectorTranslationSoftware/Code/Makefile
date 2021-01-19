#
#   This is a makefile for the HectorConfigUtil program.
#
#   It assumes that this is being run from the directory containing the
#   main source files for HectorConfigUtil - there aren't many - and
#   that there is a Packages directory alongside that with copies of
#   the SDS, ERS and SLALIB source exactly as released by AAO, together
#   with a miscellaneous directory that holds some more specialised
#   source files collected from various other projects (like the AAO TCS
#   and 2dF projects) where only the odd routine is needed.
#
#   This makefile will build the whole HectorConfigUtil program from
#   these source files as needed.
#
#   The main targets are:
#
#   HectorConfigUtil (the default target) - The program itself.
#   clean - cleans the built files in the current directory, leaving the
#           files for the various packages untouched.
#   all_clean - reduces everything in the current directory and the
#               packages to just the original source files.
#
#   With the exception of SLALIB, all the packages in the Packages directory
#   have their own makefile called Makefile.standalone, and this makefile
#   simply runs make in the various package directories as needed using
#   those standalone makefiles.
#
#   The exception is SLABLIB, where the standard makefile provided always
#   builds the library and insists on releasing this and the include files
#   into a release directory structure which defaults to being in the
#   user's top level directory. To avoid this, this makefile forces the
#   release files to go into a structure in the Packages directory called
#   'slalib'. (The source files are in slalib_o, since this is using the
#   obfuscated version of the files for licencing reasons.) This complicates
#   slightly the way the slalib rules are set up here.
#
#   Author:
#      Keith Shortridge, (KS), K&V.
#
#   History:
#      10th Dec 2020. Original version. KS.

#   Directory layout - note the SLALIB release directories for the library
#   and the include files. DRAMA_DIR holds copies of some standard files
#   (status.h and drama.h) used by various AAO software, and the SDS
#   release has copies of these in a Standalone sub-sirectory.

PACKAGES_DIR = ../Packages
MISC_DIR = $(PACKAGES_DIR)/Misc
SDS_DIR = $(PACKAGES_DIR)/sds
DRAMA_DIR = $(SDS_DIR)/Standalone
ERS_DIR = $(PACKAGES_DIR)/DramaErs
SLALIB_SRC_DIR = $(PACKAGES_DIR)/slalib_o
SLALIB_INC_DIR = $(PACKAGES_DIR)/slalib/include
SLALIB_LIB_DIR = $(PACKAGES_DIR)/slalib/lib

#  The main released slalib include file

SLALIB_INCL = $(SLALIB_INC_DIR)/slalib.h

#  Compilation flags

INC = -I $(MISC_DIR) -I $(SDS_DIR) -I $(SLALIB_INC_DIR) -I $(DRAMA_DIR)
CCC = g++
CCFLAGS = -O $(INC) -std=c++11 -Wall -pedantic

#  The individual object modules used directly from the miscellaneous
#  directory, and the various library files used.

MISC_OBJ = $(MISC_DIR)/ArrayManager.o $(MISC_DIR)/gen_qfmed.o \
                   $(MISC_DIR)/TcsUtil.o $(MISC_DIR)/tdfxy.o

LIBS = $(SDS_DIR)/libsds.a $(ERS_DIR)/libers.a $(SLALIB_LIB_DIR)/libsla.a

#  Local object files specific to HectorConfigUtil

OBJ = HectorConfigUtil.o HectorRaDecXY.o

#  Default target - the executable program

All: HectorConfigUtil

#  Compilation and build rules for the files in the HectorConfigUtil directory

HectorConfigUtil : $(OBJ) $(MISC_OBJ) $(LIBS)
	$(CCC) $(CCFLAGS) -o HectorConfigUtil $(OBJ) $(MISC_OBJ) $(LIBS) -lpthread

HectorConfigUtil.o : HectorConfigUtil.cpp HectorStructures.h HectorRaDecXY.h $(SLALIB_INCL)
	$(CCC) $(CCFLAGS) -c HectorConfigUtil.cpp

HectorRaDecXY.o : HectorRaDecXY.cpp HectorRaDecXY.h $(SLALIB_INCL)
	$(CCC) $(CCFLAGS) -c HectorRaDecXY.cpp

#  Building the various packages from source using their own makefiles

$(SLALIB_INCL) $(SLALIB_LIB_DIR)/libsla.a :
	$(MAKE) -C $(SLALIB_SRC_DIR) INSTALL_DIR=../slalib

$(SDS_DIR)/libsds.a :
	$(MAKE) -C $(SDS_DIR) -f Makefile.standalone libsds.a

$(ERS_DIR)/libers.a :
	$(MAKE) -C $(ERS_DIR) -f Makefile.standalone libers.a

$(MISC_OBJ) :
	$(MAKE) -C $(MISC_DIR) -f Makefile.standalone

#  Cleaning up. Note that all_clean has to explicitly clean out the
#  locally released versions of SLALIB as well as the intermediate files.

clean ::
	$(RM) HectorConfigUtil *.o

all_clean ::
	$(MAKE) -C $(SDS_DIR) -f Makefile.standalone clean
	$(MAKE) -C $(ERS_DIR) -f Makefile.standalone clean
	$(MAKE) -C $(SLALIB_SRC_DIR) clean
	$(MAKE) -C $(MISC_DIR) -f Makefile.standalone clean
	$(RM) $(SLALIB_LIB_DIR)/libsla.a
	$(RM) $(SLALIB_INC_DIR)/*.h
	$(RM) HectorConfigUtil *.o
