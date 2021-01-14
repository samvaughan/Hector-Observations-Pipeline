#-----------------------------------------------------------------------
#
# Description:  make file for the ANSI-C version of SLALIB.  This
# makefile creates a Unix .a library.  Designed for Linux/gcc but
# can be adapted for other platforms or run in an appropriate way -
# for example on Sun/Solaris type "make CCOMPC=cc CFLAGC=-c".
#
# Acknowledgements:
#    Martin Shepherd, Caltech
#    Jeremy Bailey, AAO
#    Peter Bunclark, RGO
#
# Usage:
#
#    To make the library and install the library and include files, type:
#
#      make
#
#    To delete all object files and the local copy of the library, type:
#
#      make clean
#
#    To build and execute the testbed, type:
#
#      make test
#
# Last revision:   23 May 2009
#
# Copyright P.T.Wallace.  All rights reserved.
#
#-----------------------------------------------------------------------

#----YOU MAY HAVE TO MODIFY THE NEXT FEW DEFINITIONS-----

# Specify the installation home directory.

INSTALL_DIR = $(HOME)

# Specify the installation directory for the library.

SLA_LIB_DIR = $(INSTALL_DIR)/lib/

# Specify the installation directory for the include files.

SLA_INC_DIR = $(INSTALL_DIR)/include/

# This suite of functions is only compilable by ANSI C compilers -
# give the name of your preferred C compiler (CCOMPC) and compilation
# flags (CFLAGC for functions and CFLAGX for executables) here.

CCOMPC = gcc
CFLAGC = -c -pedantic -Wall -W -O
CFLAGX = -pedantic -Wall -W -O

#----YOU SHOULDN'T HAVE TO MODIFY ANYTHING BELOW THIS LINE---------

# The list of installation directories.

INSTALL_DIRS = $(SLA_LIB_DIR) $(SLA_INC_DIR)

# Name the slalib library in its source location.

SLA_LIB_NAME = libsla.a

# Name the slalib library in its target location.

SLA_LIB = $(SLA_LIB_DIR)$(SLA_LIB_NAME)

# Name the slalib includes in their target location.

SLA_INC = $(SLA_INC_DIR)slalib.h $(SLA_INC_DIR)slamac.h

# Name the slalib testbed in its source and target locations

SLA_TEST_NAME = slatest.c
SLA_TEST = slatest

# The list of slalib library object files.

SLA_OBS = slaAddet.o slaAfin.o slaAirmas.o slaAltaz.o slaAmp.o \
slaAmpqk.o slaAop.o slaAoppa.o slaAoppat.o slaAopqk.o slaAtmdsp.o \
slaAv2m.o slaBear.o slaC2i.o slaC2ipa.o slaC2ipad.o slaC2ipas.o \
slaC2iqk.o slaC2iqkz.o slaCaf2r.o slaCaldj.o slaCalyd.o \
slaCc2s.o slaCc62s.o slaCd2tf.o slaCldj.o slaClyd.o slaCombn.o \
slaCr2af.o slaCr2tf.o slaCs2c.o slaCs2c6.o slaCtf2d.o slaCtf2r.o \
slaDaf2r.o slaDafin.o slaDat.o slaDav2m.o slaDbear.o slaDbjin.o \
slaDc62s.o slaDcc2s.o slaDcmpf.o slaDcs2c.o slaDd2tf.o slaDe2h.o \
slaDeuler.o slaDfltin.o slaDh2e.o slaDimxv.o slaDjcal.o slaDjcl.o \
slaDm2av.o slaDmat.o slaDmoon.o slaDmxm.o slaDmxv.o slaDpav.o \
slaDr2af.o slaDr2tf.o slaDrange.o slaDranrm.o slaDs2c6.o \
slaDs2tp.o slaDsep.o slaDsepv.o slaDt.o slaDtf2d.o slaDtf2r.o \
slaDtp2s.o slaDtp2v.o slaDtps2c.o slaDtpv2c.o slaDtt.o slaDv2tp.o \
slaDvdv.o slaDvn.o slaDvxv.o slaE2h.o slaEarth.o slaEcleq.o \
slaEcmat.o slaEcor.o slaEg50.o slaEl2ue.o slaEo.o slaEors.o \
slaEpb.o slaEpb2d.o slaEpco.o slaEpj.o slaEpj2d.o slaEpv.o \
slaEqecl.o slaEqeqx.o slaEqgal.o slaEra.o slaEtrms.o slaEuler.o \
slaEvp.o slaFitxy.o slaFk425.o slaFk45z.o slaFk524.o slaFk52h.o \
slaFk54z.o slaFk5hz.o slaFlotin.o slaFw2m.o slaFw2xy.o \
slaG2ixys.o slaGaleq.o slaGalsup.o slaGe50.o slaGeoc.o slaGmst.o \
slaGmsta.o slaGst.o slaH2e.o slaH2fk5.o slaHfk5z.o slaI2c.o \
slaI2cqk.o slaI2o.o slaI2opa.o slaI2opad.o slaI2opat.o slaI2oqk.o \
slaImxv.o slaInt2in.o slaIntin.o slaInvf.o slaKbj.o slaM2av.o \
slaMap.o slaMappa.o slaMapqk.o slaMapqkz.o slaMoon.o slaMxm.o \
slaMxv.o slaNu.o slaNu00a.o slaNut.o slaNutc.o slaNutc80.o \
slaO2i.o slaO2iqk.o slaOap.o slaOapqk.o slaObs.o \
slaPa.o slaPav.o slaPcd.o slaPda2h.o slaPdq2h.o slaPermut.o \
slaPertel.o slaPertue.o slaPfw.o slaPlanel.o slaPlanet.o \
slaPlante.o slaPlantu.o slaPm.o slaPncio.o slaPneqx.o slaPolmo.o \
slaPomom.o slaPrebn.o slaPrec.o slaPrecl.o slaPreces.o slaPrenut.o \
slaPvobs.o slaPv2el.o slaPv2ue.o slaPxy.o slaRange.o slaRanorm.o \
slaRcc.o slaRdplan.o slaRefco.o slaRefcoq.o slaRefro.o slaRefv.o \
slaRefz.o slaRverot.o slaRvgalc.o slaRvlg.o slaRvlsrd.o \
slaRvlsrk.o slaS.o slaS2tp.o slaSep.o slaSepv.o slaSmat.o slaSp.o \
slaSubet.o slaSupgal.o slaSvd.o slaSvdcov.o slaSvdsol.o slaTp2s.o \
slaTp2v.o slaTps2c.o slaTpv2c.o slaUe2el.o slaUe2pv.o slaUnpcd.o \
slaV2tp.o slaVdv.o slaVn.o slaVxv.o slaXy2xy.o slaZd.o

#-----------------------------------------------------------------------

default: $(INSTALL_DIRS) $(SLA_INC) $(SLA_LIB)

# Make the installation directories if necessary.

$(INSTALL_DIRS):
	mkdir -p $@

# Make and install the library.

$(SLA_LIB): $(SLA_OBS)
	ar ru $(SLA_LIB_NAME) $?
	cp $(SLA_LIB_NAME) $@

# Keep the installed include files up to date.

$(SLA_INC): slalib.h slamac.h
	cp slalib.h $(SLA_INC_DIR).
	cp slamac.h $(SLA_INC_DIR).

# Run the test.

test: $(SLA_TEST_NAME) $(SLA_LIB)
	$(CCOMPC) $(CFLAGX) $(SLA_TEST_NAME) \
           -I$(SLA_INC_DIR) -L$(SLA_LIB_DIR) -lm -lsla -o $(SLA_TEST)
	./$(SLA_TEST)
	rm -f $(SLA_TEST)

clean:
	rm -f $(SLA_OBS) $(SLA_LIB_NAME)

#-----------------------------------------------------------------------
# The list of object file dependencies

slaAddet.o  : addet.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ addet.c
slaAfin.o   : afin.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ afin.c
slaAirmas.o : airmas.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ airmas.c
slaAltaz.o  : altaz.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ altaz.c
slaAmp.o    : amp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ amp.c
slaAmpqk.o  : ampqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ampqk.c
slaAop.o    : aop.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ aop.c
slaAoppa.o  : aoppa.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ aoppa.c
slaAoppat.o : aoppat.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ aoppat.c
slaAopqk.o  : aopqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ aopqk.c
slaAtmdsp.o : atmdsp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ atmdsp.c
slaAv2m.o   : av2m.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ av2m.c
slaBear.o   : bear.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ bear.c
slaC2i.o    : c2i.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ c2i.c
slaC2ipa.o  : c2ipa.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ c2ipa.c
slaC2ipad.o : c2ipad.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ c2ipad.c
slaC2ipas.o : c2ipas.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ c2ipas.c
slaC2iqk.o  : c2iqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ c2iqk.c
slaC2iqkz.o : c2iqkz.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ c2iqkz.c
slaCaf2r.o  : caf2r.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ caf2r.c
slaCaldj.o  : caldj.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ caldj.c
slaCalyd.o  : calyd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ calyd.c
slaCc2s.o   : cc2s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cc2s.c
slaCc62s.o  : cc62s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cc62s.c
slaCd2tf.o  : cd2tf.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cd2tf.c
slaCldj.o   : cldj.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cldj.c
slaClyd.o   : clyd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ clyd.c
slaCombn.o  : combn.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ combn.c
slaCr2af.o  : cr2af.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cr2af.c
slaCr2tf.o  : cr2tf.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cr2tf.c
slaCs2c.o   : cs2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cs2c.c
slaCs2c6.o  : cs2c6.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ cs2c6.c
slaCtf2d.o  : ctf2d.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ctf2d.c
slaCtf2r.o  : ctf2r.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ctf2r.c
slaDaf2r.o  : daf2r.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ daf2r.c
slaDafin.o  : dafin.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dafin.c
slaDat.o    : dat.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dat.c
slaDav2m.o  : dav2m.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dav2m.c
slaDbear.o  : dbear.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dbear.c
slaDbjin.o  : dbjin.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dbjin.c
slaDc62s.o  : dc62s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dc62s.c
slaDcc2s.o  : dcc2s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dcc2s.c
slaDcmpf.o  : dcmpf.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dcmpf.c
slaDcs2c.o  : dcs2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dcs2c.c
slaDd2tf.o  : dd2tf.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dd2tf.c
slaDe2h.o   : de2h.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ de2h.c
slaDeuler.o : deuler.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ deuler.c
slaDfltin.o : dfltin.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dfltin.c
slaDh2e.o   : dh2e.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dh2e.c
slaDimxv.o  : dimxv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dimxv.c
slaDjcal.o  : djcal.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ djcal.c
slaDjcl.o   : djcl.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ djcl.c
slaDm2av.o  : dm2av.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dm2av.c
slaDmat.o   : dmat.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dmat.c
slaDmoon.o  : dmoon.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dmoon.c
slaDmxm.o   : dmxm.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dmxm.c
slaDmxv.o   : dmxv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dmxv.c
slaDpav.o   : dpav.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dpav.c
slaDr2af.o  : dr2af.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dr2af.c
slaDr2tf.o  : dr2tf.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dr2tf.c
slaDrange.o : drange.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ drange.c
slaDranrm.o : dranrm.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dranrm.c
slaDs2c6.o  : ds2c6.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ds2c6.c
slaDs2tp.o  : ds2tp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ds2tp.c
slaDsep.o   : dsep.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dsep.c
slaDsepv.o   : dsepv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dsepv.c
slaDt.o     : dt.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dt.c
slaDtf2d.o  : dtf2d.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtf2d.c
slaDtf2r.o  : dtf2r.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtf2r.c
slaDtp2s.o  : dtp2s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtp2s.c
slaDtp2v.o  : dtp2v.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtp2v.c
slaDtps2c.o : dtps2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtps2c.c
slaDtpv2c.o : dtpv2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtpv2c.c
slaDtt.o    : dtt.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dtt.c
slaDv2tp.o  : dv2tp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dv2tp.c
slaDvdv.o   : dvdv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dvdv.c
slaDvn.o    : dvn.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dvn.c
slaDvxv.o   : dvxv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ dvxv.c
slaE2h.o    : e2h.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ e2h.c
slaEarth.o  : earth.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ earth.c
slaEcleq.o  : ecleq.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ecleq.c
slaEcmat.o  : ecmat.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ecmat.c
slaEcor.o   : ecor.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ecor.c
slaEg50.o   : eg50.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ eg50.c
slaEl2ue.o  : el2ue.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ el2ue.c
slaEo.o     :  eo.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ eo.c
slaEors.o   :  eors.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ eors.c
slaEpb.o    : epb.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ epb.c
slaEpb2d.o  : epb2d.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ epb2d.c
slaEpco.o   : epco.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ epco.c
slaEpj.o    : epj.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ epj.c
slaEpj2d.o  : epj2d.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ epj2d.c
slaEpv.o    : epv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ epv.c
slaEqecl.o  : eqecl.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ eqecl.c
slaEqeqx.o  : eqeqx.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ eqeqx.c
slaEqgal.o  : eqgal.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ eqgal.c
slaEra.o    :  era.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ era.c
slaEtrms.o  : etrms.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ etrms.c
slaEuler.o  : euler.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ euler.c
slaEvp.o    : evp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ evp.c
slaFitxy.o  : fitxy.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fitxy.c
slaFk425.o  : fk425.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fk425.c
slaFk45z.o  : fk45z.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fk45z.c
slaFk524.o  : fk524.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fk524.c
slaFk52h.o  : fk52h.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fk52h.c
slaFk54z.o  : fk54z.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fk54z.c
slaFk5hz.o  : fk5hz.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fk5hz.c
slaFlotin.o : flotin.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ flotin.c
slaFw2m.o   :  fw2m.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fw2m.c
slaFw2xy.o  :  fw2xy.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ fw2xy.c
slaG2ixys.o :  g2ixys.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ g2ixys.c
slaGaleq.o  : galeq.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ galeq.c
slaGalsup.o : galsup.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ galsup.c
slaGe50.o   : ge50.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ge50.c
slaGeoc.o   : geoc.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ geoc.c
slaGmst.o   : gmst.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ gmst.c
slaGmsta.o  : gmsta.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ gmsta.c
slaGst.o    :  gst.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ gst.c
slaH2e.o    : h2e.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ h2e.c
slaH2fk5.o  : h2fk5.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ h2fk5.c
slaHfk5z.o  : hfk5z.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ hfk5z.c
slaI2c.o    : i2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2c.c
slaI2cqk.o  : i2cqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2cqk.c
slaI2o.o    : i2o.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2o.c
slaI2opa.o  : i2opa.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2opa.c
slaI2opad.o : i2opad.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2opad.c
slaI2opat.o : i2opat.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2opat.c
slaI2oqk.o  : i2oqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ i2oqk.c
slaImxv.o   : imxv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ imxv.c
slaInt2in.o  : int2in.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ int2in.c
slaIntin.o  : intin.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ intin.c
slaInvf.o   : invf.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ invf.c
slaKbj.o    : kbj.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ kbj.c
slaM2av.o   : m2av.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ m2av.c
slaMap.o    : map.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ map.c
slaMappa.o  : mappa.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ mappa.c
slaMapqk.o  : mapqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ mapqk.c
slaMapqkz.o : mapqkz.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ mapqkz.c
slaMoon.o   : moon.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ moon.c
slaMxm.o    : mxm.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ mxm.c
slaMxv.o    : mxv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ mxv.c
slaNu.o     :  nu.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ nu.c
slaNu00a.o  :  nu00a.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ nu00a.c
slaNut.o    : nut.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ nut.c
slaNutc.o   : nutc.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ nutc.c
slaNutc80.o   : nutc80.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ nutc80.c
slaO2i.o    : o2i.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ o2i.c
slaO2iqk.o  : o2iqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ o2iqk.c
slaOap.o    : oap.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ oap.c
slaOapqk.o  : oapqk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ oapqk.c
slaObs.o    : obs.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ obs.c
slaPa.o     : pa.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pa.c
slaPav.o    : pav.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pav.c
slaPcd.o    : pcd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pcd.c
slaPfw.o    :  pfw.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pfw.c
slaPm.o     : pm.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pm.c
slaPda2h.o  : pda2h.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pda2h.c
slaPdq2h.o  : pdq2h.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pdq2h.c
slaPermut.o : permut.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ permut.c
slaPertel.o : pertel.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pertel.c
slaPertue.o : pertue.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pertue.c
slaPlanel.o : planel.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ planel.c
slaPlanet.o : planet.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ planet.c
slaPlante.o : plante.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ plante.c
slaPlantu.o : plantu.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ plantu.c
slaPolmo.o  : polmo.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ polmo.c
slaPomom.o  : pomom.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pomom.c
slaPncio.o  :  pncio.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pncio.c
slaPneqx.o  :  pneqx.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pneqx.c
slaPrebn.o  : prebn.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ prebn.c
slaPrec.o   : prec.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ prec.c
slaPrecl.o  : precl.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ precl.c
slaPreces.o : preces.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ preces.c
slaPrenut.o : prenut.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ prenut.c
slaPvobs.o  : pvobs.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pvobs.c
slaPv2el.o  : pv2el.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pv2el.c
slaPv2ue.o  : pv2ue.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pv2ue.c
slaPxy.o    : pxy.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ pxy.c
slaRange.o  : range.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ range.c
slaRanorm.o : ranorm.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ranorm.c
slaRcc.o    : rcc.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rcc.c
slaRdplan.o : rdplan.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rdplan.c
slaRefco.o  : refco.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ refco.c
slaRefcoq.o : refcoq.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ refcoq.c
slaRefro.o  : refro.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ refro.c
slaRefv.o   : refv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ refv.c
slaRefz.o   : refz.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ refz.c
slaRverot.o : rverot.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rverot.c
slaRvgalc.o : rvgalc.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rvgalc.c
slaRvlg.o   : rvlg.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rvlg.c
slaRvlsrd.o : rvlsrd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rvlsrd.c
slaRvlsrk.o : rvlsrk.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ rvlsrk.c
slaS.o      :  s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ s.c
slaS2tp.o   : s2tp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ s2tp.c
slaSep.o    : sep.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ sep.c
slaSepv.o    : sepv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ sepv.c
slaSmat.o   : smat.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ smat.c
slaSp.o   : sp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ sp.c
slaSubet.o  : subet.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ subet.c
slaSupgal.o : supgal.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ supgal.c
slaSvd.o    : svd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ svd.c
slaSvdcov.o : svdcov.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ svdcov.c
slaSvdsol.o : svdsol.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ svdsol.c
slaTp2s.o   : tp2s.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ tp2s.c
slaTp2v.o   : tp2v.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ tp2v.c
slaTps2c.o  : tps2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ tps2c.c
slaTpv2c.o  : tpv2c.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ tpv2c.c
slaUe2el.o  : ue2el.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ue2el.c
slaUe2pv.o  : ue2pv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ ue2pv.c
slaUnpcd.o  : unpcd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ unpcd.c
slaV2tp.o   : v2tp.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ v2tp.c
slaVdv.o    : vdv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ vdv.c
slaVn.o     : vn.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ vn.c
slaVxv.o    : vxv.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ vxv.c
slaXy2xy.o  : xy2xy.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ xy2xy.c
slaZd.o     : zd.c slalib.h slamac.h
	$(CCOMPC) $(CFLAGC) -o $@ zd.c
