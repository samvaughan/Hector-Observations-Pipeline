cc -c -O gen_qfmed.c
c++ -c -O TcsUtil.cpp
c++ -o HectorConfigUtil -DM_I86 -std=c++11 -Wall -pedantic -I ../../configure/2dFutil/ -I ../../configure/include/ -I ../../configure/sds -I ../../configure/sds/Standalone/  -I ../../configure/aaoConfigure/drama_src   HectorConfigUtil.cpp HectorRaDecXy.cpp gen_qfmed.o TcsUtil.o ../../configure/2dFutil/tdfxy.o /Users/drama/release/sds/r1_4_3_86/macosx_x86_64/libsds.a /Users/drama/release/ers/r3_16/macosx_x86_64/libers.a /usr/local/lib/libsla.a
