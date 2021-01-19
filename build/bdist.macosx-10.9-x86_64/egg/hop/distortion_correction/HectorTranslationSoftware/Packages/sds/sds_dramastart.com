$! @(#) $Id: ACMM:sds/sds_dramastart.com,v 3.94 09-Dec-2020 17:15:24+11 ks $
$RELEASE="1"
$ TARGET = f$getsys("ARCH_NAME") + "VMS"
$ DEV = TARGET
$!
$define sds_dir dramadisk:[release.sds.'release'],dramadisk:[release.sds.'release'.'target']
$define sds_lib dramadisk:[release.sds.'release'.'target']
$define/job sds_dev dramadisk:[release.sds.'release'.'dev']
$define gcc_Dir utask_dir		! For SDSC C preprocessor
$sdsc:==$sds_dev:sdsc
$hds2sds:==$sds_dev:hds2sds
$sds2hds:==$sds_dev:sds2hds
$sdslist:==$sds_dev:sdslist
