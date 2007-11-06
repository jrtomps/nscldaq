SUBDIRS=lib include
all-local:
	echo	./DriverBuild.sh  $(KERNEL_SRC)
	./DriverBuild.sh  $(KERNEL_SRC)

install-data-local:
	$(INSTALL_DATA)   dd/btp.ko /lib/modules
	$(INSTALL_SCRIPT) sys/mkbtp  /lib/modules



EXTRA_DIST=DriverBuild.sh
