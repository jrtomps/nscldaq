SUBDIRS = btdriver \
	Headers CopyrightTools Exception TCL Security Ftp \
	VMEApi cvt SpecTclOnline Readout diskrun EventLog Scaler contrib \
	vmetcl  \
	bufdump TclServer bcnaf TcpHoister Stager Scripts framework \
	AlarmDisplay LogDisplay ProductionReadout 

EXTRA_DIST =  config/install-sh config/config.sub config/config.guess \
	config/ltmain.sh config/missing config/mkinstalldirs  CHANGELOG

install-data-local:
	rm -f $(prefix)/Bin
	rm -f $(prefix)/Lib
	rm -f $(prefix)/Include
	rm -f $(prefix)/Etc
	$(LN_S) $(prefix)/bin $(prefix)/Bin
	$(LN_S) $(prefix)/lib $(prefix)/Lib
	$(LN_S) $(prefix)/include $(prefix)/Include
	$(LN_S) $(prefix)/etc     $(prefix)/Etc
