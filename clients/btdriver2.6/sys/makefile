#!/bin/ksh -f
#
#	Filename:	makefile
#
#	Purpose:	Creates options include file make.config, builds
#                       the driver if needed and finally installs the
#                       and creates the device files.
#
#	Copyright (c) 1999 by SBS Technologies, Inc.
#	All rights reserved.
#
#	$Revision$
#

##############################################################################
#
#	User configuration
#
##############################################################################
#    Commenting variables on the same line adds white space to the 
#    end of the value. 
#       MOD_BASE     location of driver module and corresponding files
#       MODULE       Module name
#       MAKECFG      configurations file included by dd/makefile
#
MOD_BASE=/lib/modules
MODULE=btp
MAKECFG=config.make
##############################################################################
#
#	End configuration information
#
##############################################################################

default:	install 

rebuild:	mrproper install

config:
	@if [ ! -e ${MAKECFG} ] ; then \
                echo "   Creating makefile configuration file ${MAKECFG}" ; \
		cp /dev/null ${MAKECFG}; \
		echo "# ${MAKECFG} created by 'make config'." \
                    >> ${MAKECFG} ; \
		if grep -q smp_ /proc/ksyms ; then \
			echo "SMP_FLAG=-D__SMP__" >> ${MAKECFG} ; \
		else \
			echo "SMP_FLAG=" >> ${MAKECFG} ; \
		fi ; \
		echo 'ARCH_FLAGS=$$(I386_FLAGS)' >> ${MAKECFG} ; \
		LINUX_SOURCE=/usr/src/kernel-headers-`uname -r` ; \
		echo "LINUX_SRC=$$LINUX_SOURCE" >> ${MAKECFG} ; \
		echo "#CC=kgcc" >> ${MAKECFG} ; \
		echo "   location of linux source $$LINUX_SOURCE" ; \
	fi ;
	@if [ ! -e ${MODULE}.conf ] ; then \
                echo "   Creating driver configuration file ${MODULE}.conf" ; \
		cp /dev/null ${MODULE}.conf ; \
	fi ;

check_env:
	@if [ `whoami` != root ] ; then \
	    echo Error, only root can [un]install or [un]load the driver ; \
	    exit 1 ; \
	fi ;
	if [ ! -d ${MOD_BASE} ] ; then \
		mkdir -p ${MOD_BASE} ; \
	fi ;

install:	btp.o lib check_env uninstall
	@if [ ! -e ${MODULE}.o  ] ; then \
	    echo Error, ${MODULE}.o wasn't built - can't install it. ; \
	    exit 1 ; \
	fi ;
	cp -p ${MODULE}.o ${MOD_BASE} ; \
	cp -p ${MODULE}.o ${MOD_BASE}/$(MODULE).ko ; \
	cp -p ${MODULE}.conf ${MOD_BASE} ; \
	cp -p mkbtp ${MOD_BASE} ; \
	${MOD_BASE}/mkbtp ;
	@echo -e \
	"\nTo reload the device driver with new parameters use \"make load\"." ;

uninstall:	check_env unload
	if [ -e ${MOD_BASE}/${MODULE}.conf ] ; then \
	if ! diff -q ${MODULE}.conf ${MOD_BASE} ; then \
		cp -f ${MOD_BASE}/${MODULE}.conf ${MODULE}.conf.prev ; \
	fi ; \
	fi ;
	rm -f ${MOD_BASE}/${MODULE}.o ; \
	rm -f ${MOD_BASE}/${MODULE}.ko ; \
	rm -f ${MOD_BASE}/${MODULE}.conf ; \
	rm -f ${MOD_BASE}/mkbtp ;

clean:
	rm -f *.o config.make ;

mrproper:
	cd ../dd; rm -f *.d; make clean;
	make clean;

cleanall:
	make config;
	cd ../lib; make clean;
	cd ../src; make clean;
	cd ../diag; make clean;
	make mrproper;

unload:		check_env
	if grep -q ${MODULE} /proc/modules ; then \
		/sbin/rmmod ${MODULE} ; \
	fi ;

load:		check_env unload
	@if [ ! -e ${MOD_BASE}/${MODULE}.conf ] ; then \
		echo Error, driver not installed - use \"make install\". ; \
		exit 1 ; \
	fi ;
	if ! diff -q ${MODULE}.conf ${MOD_BASE} ; then \
		cp -f ${MOD_BASE}/${MODULE}.conf ${MODULE}.conf.prev ; \
		cp -f ${MODULE}.conf ${MOD_BASE} ; \
	fi ; \
	/sbin/insmod ${MODULE} `cat ${MOD_BASE}/${MODULE}.conf` ;

btp btp.o:  config
	cd ../dd; make ;

lib:
	@if [ ! -e ../lib/libbtp.a  ] ; then \
		cd ../lib; \
		make;  \
	fi ;

.FAILED:
	@echo "" ;
