#
#  Makefile for the production readout system.
#
#  Targets:
#    documents   - Doxygen documentation.
#    system      - Readout software/skeletons.
#    install    - Installations.
#    clean       - remove objects etc.
#
# Key variables:
#	OS      - operating system identifier.
#   	INSTDIR - Install root.
#       DOCDIR  - Documentation installation root.
#
# Change Log:
#   $Log$
#   Revision 1.1  2003/03/12 04:17:41  ron-fox
#   Initial revision
#
#   Revision 1.7  2003/03/12 03:50:56  fox
#   Bring up to date prior to export that gets the files that are loaded into sourceforge.
#
#   Revision 1.6  2002/11/21 14:51:17  fox
#   Added skeleton and odified installation... support multiple VME crates correctly
#
#   Revision 1.5  2002/11/08 19:19:54  fox
#   Add documentation to the build.
#
# Revision 1.4  2002/10/16  16:23:41  fox
# Add copyright, author notices.
#
# Revision 1.3  2002/10/16  14:22:22  fox
# Change some runvars into consts:
# - state -- the run state is now a const maintained by the program.
# - starttime - the time at which the run started.
#
# Link in libraries from . before $(INSTDIR)
#
# Revision 1.2  2002/10/15  12:48:32  fox
# 1. Initial testing
# 2. Add const variable and make tkloaded const.
# 3. Add TclServer functionality.
#


SPECTRO_FLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --libs)
VME=$(shell $(INSTDIR)/Bin/VmeDevice)
OS=$(shell uname | sed s/-//)
LD=g++
LDFLAGS=-g

HYPERDEST=$(DOCDIR)/Manual
DOXYDEST=$(DOCDIR)/Doxygen

NSCLBIT3LIB=-lVmeApi
SBSBIT3LIB  = -lVmeApi -lbtp

LIBFLAGS=-L. -L$(INSTDIR)/Lib  -L/usr/X11R6/lib -lPReadout \
	-lEventFramework -lTclPlus -lException $($(VME)LIB)  -lException  \
	$(SPECTRO_LDFLAGS) -ltk -ltcl -lXm -lXt -lX11 -lm -lLicense

COMPILEFLAGS= $(SPECTRO_FLAGS) -c -ggdb -I. -I$(INSTDIR)/Include  \
	-D$(OS) -I/usr/include/tcl 

CPP=g++ $(COMPILEFLAGS)
CC=gcc $(COMPILEFLAGS)

.cpp.o:
	$(CPP) $<

.c.o:
	$(CC) $<

OBJECTS = CTimedEvent.o CTimer.o CCamac.o CRunState.o CStatusModule.o \
      CCamacModule.o CCamacNimout.o CBD8210.o CCrateController.o    \
      CStatusModule.o CCAMACStatusModule.o CVMEStatusModule.o \
      CTrigger.o CCAMACTrigger.o CVMETrigger.o \
      CScaler.o CScalerBank.o CCAMACScalerLRS2551.o \
      CTraditionalScalerReadout.o CCAMACScalerLRS4434.o \
      CVMEScalerLRS1151.o \
      CTraditionalEventSegment.o CCompoundEventSegment.o \
      Nimout.o CaenIO.o VmeModule.o \
      CNSCLOutputBuffer.o CNSCLScalerBuffer.o CNSCLControlBuffer.o \
      CNSCLPhysicsBuffer.o CNSCLStringListBuffer.o CNSCLBeginBuffer.o \
      CNSCLEndBuffer.o CNSCLPauseBuffer.o CNSCLResumeBuffer.o  \
      CStateVariableBuffer.o CRunVariableBuffer.o \
      CNSCLDocumentationBuffer.o CExperiment.o CScalerTrigger.o \
      cmdline.o CReadoutMain.o CInterpreterCore.o CInterpreterShell.o \
      CTclInterpreterShell.o  CTkInterpreterShell.o \
      CRunVariable.o CRunVariableCommand.o \
      CStateVariable.o CStateVariableCommand.o  CExitCommand.o \
      CTagBaseCommand.o CStateTransitionCommand.o  \
      CBeginCommand.o CEndCommand.o CPauseCommand.o \
      CResumeCommand.o CReadoutException.o CBadTransitionException.o \
      CInvalidPacketStateException.o \
      CDocumentedPacket.o CDocumentedPacketManager.o \
      CRunVariableDumpTrigger.o MmapError.o CConstVariable.o \
      CConstVariable.o CConstVariableCommand.o CReaper.o \
      CTCLServer.o CTCLListener.o \
	AccessListCheck.o AuthTokenCheck.o DAQAuthTokenInteractor.o \
	FdInteractor.o HostListCheck.o IOInteractor.o Interactor.o \
	PasswordCheck.o StringInteractor.o TclAccessListCheck.o \
	UnixUserCheck.o TclAuthorizer.o 
      
system: libPReadout.a TestReadout

TestReadout:	libPReadout.a MyExperiment.o 
	$(LD) -o TestReadout $(LDFLAGS) MyExperiment.o  $(LIBFLAGS)

libPReadout.a: $(OBJECTS)
	ar crs libPReadout.a $(OBJECTS)


documents:
	doxygen
	(cd hyperlatex; make)

docinstall:
	install -d -m 02775 $(DOCDIR)/Manual
	install -d -m 02775 $(DOCDIR)/Doxygen
	install -m 0774 hyperlatex/html/* $(DOCDIR)/Manual
	install -m 0774 docs/html/* $(DOCDIR)/Doxygen

install: system
	install -d -m 02775 $(INSTDIR)/Lib
	install -d -m 02775 $(INSTDIR)/Include
	install -d -m 02775 $(INSTDIR)/pReadoutSkeleton
	install *.a -m 02775   $(INSTDIR)/Lib
	install *.h -m 02775  $(INSTDIR)/Include
	install -m 0664 MyExperiment.cpp $(INSTDIR)/pReadoutSkeleton/Skeleton.cpp
	echo    INSTDIR=$(INSTDIR) > Makefile.user
	echo    OS=$(OS)          >> Makefile.user
	cat     Makefile.skel     >> Makefile.user
	install -m 0664 Makefile.user $(INSTDIR)/pReadoutSkeleton/Makefile


clean:
	rm -f $(OBJECTS)
	rm -f libPReadout.a
	rm -f TestReadout
	rm -rf docs

depend:
	makedepend *.cpp


# DO NOT DELETE

AccessListCheck.o: AccessListCheck.h Authenticator.h Interactor.h
AuthTokenCheck.o: AuthTokenCheck.h AccessListCheck.h Authenticator.h
AuthTokenCheck.o: Interactor.h
CBD8210.o: CBD8210.h CCamacModule.h
CBadTransitionException.o: CBadTransitionException.h CReadoutException.h
CBeginCommand.o: CBeginCommand.h CStateTransitionCommand.h CExperiment.h
CBeginCommand.o: CStatusModule.h CTrigger.h CScalerBank.h CScaler.h
CBeginCommand.o: CCompoundEventSegment.h CEventSegment.h CReadoutMain.h
CBeginCommand.o: CRunState.h CDocumentedPacketManager.h
CBeginCommand.o: CBadTransitionException.h CReadoutException.h
CCAMACScalerLRS2551.o: CCAMACScalerLRS2551.h CScaler.h CCamacModule.h
CCAMACScalerLRS4434.o: CCAMACScalerLRS4434.h CScaler.h CCamacModule.h
CCAMACStatusModule.o: CCAMACStatusModule.h CStatusModule.h
CCAMACTrigger.o: CCAMACTrigger.h CTrigger.h CBD8210.h CCamacModule.h
CCamac.o: CCamac.h /usr/include/unistd.h /usr/include/features.h
CCamac.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CCamac.o: /usr/include/bits/posix_opt.h /usr/include/bits/types.h
CCamac.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CCamac.o: /usr/include/bits/confname.h /usr/include/assert.h
CCamac.o: /usr/include/sys/types.h /usr/include/time.h
CCamac.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
CCamac.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
CCamacModule.o: CCamacModule.h CCamac.h /usr/include/assert.h
CCamacModule.o: /usr/include/features.h /usr/include/sys/cdefs.h
CCamacModule.o: /usr/include/gnu/stubs.h
CCamacNimout.o: CCamacNimout.h CCamacModule.h
CCompoundEventSegment.o: CCompoundEventSegment.h CEventSegment.h
CConstVariable.o: CConstVariable.h
CConstVariableCommand.o: CConstVariableCommand.h CConstVariable.h
CCrateController.o: CCrateController.h CBD8210.h CCamacModule.h
CDocumentedPacket.o: CDocumentedPacket.h CDocumentedPacketManager.h
CDocumentedPacket.o: CInvalidPacketStateException.h CReadoutException.h
CDocumentedPacket.o: /usr/include/stdio.h /usr/include/features.h
CDocumentedPacket.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CDocumentedPacket.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CDocumentedPacket.o: /usr/include/bits/types.h /usr/include/libio.h
CDocumentedPacket.o: /usr/include/_G_config.h /usr/include/wchar.h
CDocumentedPacket.o: /usr/include/bits/wchar.h /usr/include/gconv.h
CDocumentedPacket.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
CDocumentedPacket.o: /usr/include/bits/stdio_lim.h /usr/include/time.h
CDocumentedPacketManager.o: CDocumentedPacketManager.h CDocumentedPacket.h
CEndCommand.o: CEndCommand.h CStateTransitionCommand.h CReadoutMain.h
CEndCommand.o: CRunState.h CDocumentedPacketManager.h
CEventSegment.o: CEventSegment.h
CExitCommand.o: CExitCommand.h CRunState.h CReadoutMain.h
CExitCommand.o: CDocumentedPacketManager.h /usr/include/unistd.h
CExitCommand.o: /usr/include/features.h /usr/include/sys/cdefs.h
CExitCommand.o: /usr/include/gnu/stubs.h /usr/include/bits/posix_opt.h
CExitCommand.o: /usr/include/bits/types.h
CExitCommand.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CExitCommand.o: /usr/include/bits/confname.h
CExperiment.o: CExperiment.h CStatusModule.h CTrigger.h CScalerBank.h
CExperiment.o: CScaler.h CCompoundEventSegment.h CEventSegment.h
CExperiment.o: CStateTransitionCommand.h CBeginCommand.h CEndCommand.h
CExperiment.o: CReadoutMain.h CRunState.h CDocumentedPacketManager.h
CExperiment.o: CNSCLPhysicsBuffer.h CNSCLOutputBuffer.h CNSCLScalerBuffer.h
CExperiment.o: CNSCLControlBuffer.h CRunVariableBuffer.h
CExperiment.o: CNSCLStringListBuffer.h CStateVariableBuffer.h
CExperiment.o: CNSCLDocumentationBuffer.h CDocumentedPacket.h
CExperiment.o: CInterpreterCore.h CPauseCommand.h CResumeCommand.h
CExperiment.o: CRunVariableCommand.h CRunVariable.h CStateVariableCommand.h
CExperiment.o: CTagBaseCommand.h CExitCommand.h CConstVariableCommand.h
CExperiment.o: CConstVariable.h CInterpreterShell.h CTimer.h CScalerTrigger.h
CExperiment.o: CTimedEvent.h CStateVariable.h buftypes.h buffer.h
CExperiment.o: /usr/include/sys/time.h /usr/include/features.h
CExperiment.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CExperiment.o: /usr/include/bits/types.h
CExperiment.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CExperiment.o: /usr/include/time.h /usr/include/bits/time.h
CExperiment.o: /usr/include/sys/select.h /usr/include/bits/select.h
CExperiment.o: /usr/include/bits/sigset.h /usr/include/unistd.h
CExperiment.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
CExperiment.o: /usr/include/stdlib.h
CExperiment.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/typeinfo
CExperiment.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
CInterpreterCore.o: TclAuthorizer.h CInterpreterCore.h CPauseCommand.h
CInterpreterCore.o: CResumeCommand.h CRunVariableCommand.h CRunVariable.h
CInterpreterCore.o: CStateVariableCommand.h CTagBaseCommand.h CExitCommand.h
CInterpreterCore.o: CConstVariableCommand.h CConstVariable.h CReadoutMain.h
CInterpreterCore.o: CRunState.h CDocumentedPacketManager.h CStateVariable.h
CInterpreterCore.o: CTCLListener.h
CInterpreterShell.o: CInterpreterShell.h CInterpreterCore.h CPauseCommand.h
CInterpreterShell.o: CResumeCommand.h CRunVariableCommand.h CRunVariable.h
CInterpreterShell.o: CStateVariableCommand.h CTagBaseCommand.h CExitCommand.h
CInterpreterShell.o: CConstVariableCommand.h CConstVariable.h
CInvalidPacketStateException.o: CInvalidPacketStateException.h
CInvalidPacketStateException.o: CReadoutException.h
CNSCLBeginBuffer.o: CNSCLBeginBuffer.h CNSCLControlBuffer.h buftypes.h
CNSCLControlBuffer.o: CNSCLControlBuffer.h /usr/include/time.h
CNSCLControlBuffer.o: /usr/include/bits/types.h /usr/include/features.h
CNSCLControlBuffer.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CNSCLControlBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CNSCLDocumentationBuffer.o: CNSCLDocumentationBuffer.h buftypes.h
CNSCLEndBuffer.o: CNSCLEndBuffer.h CNSCLControlBuffer.h buftypes.h
CNSCLOutputBuffer.o: CNSCLOutputBuffer.h /usr/include/unistd.h
CNSCLOutputBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
CNSCLOutputBuffer.o: /usr/include/gnu/stubs.h /usr/include/bits/posix_opt.h
CNSCLOutputBuffer.o: /usr/include/bits/types.h
CNSCLOutputBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CNSCLOutputBuffer.o: /usr/include/bits/confname.h
CNSCLPauseBuffer.o: CNSCLPauseBuffer.h CNSCLControlBuffer.h buftypes.h
CNSCLPhysicsBuffer.o: CNSCLPhysicsBuffer.h CNSCLOutputBuffer.h buftypes.h
CNSCLPhysicsBuffer.o: /usr/include/unistd.h /usr/include/features.h
CNSCLPhysicsBuffer.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CNSCLPhysicsBuffer.o: /usr/include/bits/posix_opt.h /usr/include/bits/types.h
CNSCLPhysicsBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CNSCLPhysicsBuffer.o: /usr/include/bits/confname.h
CNSCLResumeBuffer.o: CNSCLResumeBuffer.h CNSCLControlBuffer.h buftypes.h
CNSCLScalerBuffer.o: CNSCLScalerBuffer.h buftypes.h
CNSCLStringListBuffer.o: CNSCLStringListBuffer.h
CPauseCommand.o: CPauseCommand.h CReadoutMain.h CRunState.h
CPauseCommand.o: CDocumentedPacketManager.h
CReadoutException.o: CReadoutException.h
CReadoutMain.o: CReadoutMain.h CRunState.h CDocumentedPacketManager.h
CReadoutMain.o: cmdline.h CTclInterpreterShell.h CInterpreterShell.h
CReadoutMain.o: CTkInterpreterShell.h CInterpreterCore.h CPauseCommand.h
CReadoutMain.o: CResumeCommand.h CRunVariableCommand.h CRunVariable.h
CReadoutMain.o: CStateVariableCommand.h CTagBaseCommand.h CExitCommand.h
CReadoutMain.o: CConstVariableCommand.h CConstVariable.h CStateVariable.h
CReadoutMain.o: CVMETrigger.h CTrigger.h CCAMACTrigger.h CBD8210.h
CReadoutMain.o: CCamacModule.h CVMEStatusModule.h CStatusModule.h
CReadoutMain.o: CCAMACStatusModule.h CReaper.h CTCLListener.h
CReadoutMain.o: /usr/include/stdio.h /usr/include/features.h
CReadoutMain.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CReadoutMain.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CReadoutMain.o: /usr/include/bits/types.h /usr/include/libio.h
CReadoutMain.o: /usr/include/_G_config.h /usr/include/wchar.h
CReadoutMain.o: /usr/include/bits/wchar.h /usr/include/gconv.h
CReadoutMain.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
CReadoutMain.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
CReadoutMain.o: /usr/include/string.h /usr/include/unistd.h
CReadoutMain.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
CReaper.o: CReaper.h
CResumeCommand.o: CResumeCommand.h CReadoutMain.h CRunState.h
CResumeCommand.o: CDocumentedPacketManager.h
CRunState.o: CRunState.h CBadTransitionException.h CReadoutException.h
CRunState.o: CReadoutMain.h CDocumentedPacketManager.h CInterpreterShell.h
CRunState.o: CInterpreterCore.h CPauseCommand.h CResumeCommand.h
CRunState.o: CRunVariableCommand.h CRunVariable.h CStateVariableCommand.h
CRunState.o: CTagBaseCommand.h CExitCommand.h CConstVariableCommand.h
CRunState.o: CConstVariable.h CStateVariable.h
CRunVariable.o: CRunVariable.h
CRunVariableBuffer.o: CRunVariableBuffer.h CNSCLStringListBuffer.h buftypes.h
CRunVariableCommand.o: CRunVariableCommand.h CRunVariable.h
CRunVariableCommand.o: /usr/include/assert.h /usr/include/features.h
CRunVariableCommand.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CRunVariableDumpTrigger.o: CRunVariableDumpTrigger.h CTimedEvent.h
CRunVariableDumpTrigger.o: CExperiment.h CStatusModule.h CTrigger.h
CRunVariableDumpTrigger.o: CScalerBank.h CScaler.h CCompoundEventSegment.h
CRunVariableDumpTrigger.o: CEventSegment.h
CScaler.o: CScaler.h
CScalerBank.o: CScalerBank.h CScaler.h
CScalerTrigger.o: CScalerTrigger.h CTimedEvent.h CExperiment.h
CScalerTrigger.o: CStatusModule.h CTrigger.h CScalerBank.h CScaler.h
CScalerTrigger.o: CCompoundEventSegment.h CEventSegment.h
CStateTransitionCommand.o: CStateTransitionCommand.h CReadoutMain.h
CStateTransitionCommand.o: CRunState.h CDocumentedPacketManager.h
CStateVariable.o: CStateVariable.h CReadoutMain.h CRunState.h
CStateVariable.o: CDocumentedPacketManager.h /usr/include/assert.h
CStateVariable.o: /usr/include/features.h /usr/include/sys/cdefs.h
CStateVariable.o: /usr/include/gnu/stubs.h
CStateVariableBuffer.o: CStateVariableBuffer.h buftypes.h
CStateVariableCommand.o: CStateVariableCommand.h CStateVariable.h
CStatusModule.o: CStatusModule.h
CTCLListener.o: CTCLListener.h CTCLServer.h TclAuthorizer.h CReadoutMain.h
CTCLListener.o: CRunState.h CDocumentedPacketManager.h CInterpreterShell.h
CTCLListener.o: CInterpreterCore.h CPauseCommand.h CResumeCommand.h
CTCLListener.o: CRunVariableCommand.h CRunVariable.h CStateVariableCommand.h
CTCLListener.o: CTagBaseCommand.h CExitCommand.h CConstVariableCommand.h
CTCLListener.o: CConstVariable.h /usr/include/stdio.h /usr/include/features.h
CTCLListener.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CTCLListener.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CTCLListener.o: /usr/include/bits/types.h /usr/include/libio.h
CTCLListener.o: /usr/include/_G_config.h /usr/include/wchar.h
CTCLListener.o: /usr/include/bits/wchar.h /usr/include/gconv.h
CTCLListener.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
CTCLListener.o: /usr/include/bits/stdio_lim.h
CTCLServer.o: CTCLServer.h CReaper.h CReadoutMain.h CRunState.h
CTCLServer.o: CDocumentedPacketManager.h CInterpreterShell.h
CTCLServer.o: CInterpreterCore.h CPauseCommand.h CResumeCommand.h
CTCLServer.o: CRunVariableCommand.h CRunVariable.h CStateVariableCommand.h
CTCLServer.o: CTagBaseCommand.h CExitCommand.h CConstVariableCommand.h
CTCLServer.o: CConstVariable.h /usr/include/assert.h /usr/include/features.h
CTCLServer.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
CTagBaseCommand.o: CTagBaseCommand.h /usr/include/stdio.h
CTagBaseCommand.o: /usr/include/features.h /usr/include/sys/cdefs.h
CTagBaseCommand.o: /usr/include/gnu/stubs.h
CTagBaseCommand.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
CTagBaseCommand.o: /usr/include/bits/types.h /usr/include/libio.h
CTagBaseCommand.o: /usr/include/_G_config.h /usr/include/wchar.h
CTagBaseCommand.o: /usr/include/bits/wchar.h /usr/include/gconv.h
CTagBaseCommand.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
CTagBaseCommand.o: /usr/include/bits/stdio_lim.h
CTclInterpeterShell.o: CTclInterpeterShell.h CInterpreterShell.h
CTclInterpreterShell.o: CTclInterpreterShell.h CInterpreterShell.h
CTclInterpreterShell.o: CInterpreterCore.h CPauseCommand.h CResumeCommand.h
CTclInterpreterShell.o: CRunVariableCommand.h CRunVariable.h
CTclInterpreterShell.o: CStateVariableCommand.h CTagBaseCommand.h
CTclInterpreterShell.o: CExitCommand.h CConstVariableCommand.h
CTclInterpreterShell.o: CConstVariable.h
CTimedEvent.o: CTimedEvent.h
CTimer.o: CTimer.h CTimedEvent.h
CTkInterpreterShell.o: CTkInterpreterShell.h CInterpreterCore.h
CTkInterpreterShell.o: CPauseCommand.h CResumeCommand.h CRunVariableCommand.h
CTkInterpreterShell.o: CRunVariable.h CStateVariableCommand.h
CTkInterpreterShell.o: CTagBaseCommand.h CExitCommand.h
CTkInterpreterShell.o: CConstVariableCommand.h CConstVariable.h
CTraditionalEventSegment.o: CTraditionalEventSegment.h CEventSegment.h
CTraditionalScalerReadout.o: CTraditionalScalerReadout.h CScaler.h
CTraditionalScalerReadout.o: /usr/include/assert.h /usr/include/features.h
CTraditionalScalerReadout.o: /usr/include/sys/cdefs.h
CTraditionalScalerReadout.o: /usr/include/gnu/stubs.h
CTrigger.o: CTrigger.h
CVMEScalerCAENV830.o: CVMEScalerCAENV830.h CScaler.h
CVMEScalerLRS1151.o: CVMEScalerLRS1151.h CScaler.h
CVMEStatusModule.o: CVMEStatusModule.h CStatusModule.h
CVMETrigger.o: CVMETrigger.h CTrigger.h
DAQAuthTokenInteractor.o: DAQAuthTokenInteractor.h StringInteractor.h
DAQAuthTokenInteractor.o: Interactor.h
FdInteractor.o: FdInteractor.h Interactor.h /usr/include/unistd.h
FdInteractor.o: /usr/include/features.h /usr/include/sys/cdefs.h
FdInteractor.o: /usr/include/gnu/stubs.h /usr/include/bits/posix_opt.h
FdInteractor.o: /usr/include/bits/types.h
FdInteractor.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
FdInteractor.o: /usr/include/bits/confname.h
HostListCheck.o: HostListCheck.h AccessListCheck.h Authenticator.h
HostListCheck.o: Interactor.h StringInteractor.h /usr/include/netinet/in.h
HostListCheck.o: /usr/include/features.h /usr/include/sys/cdefs.h
HostListCheck.o: /usr/include/gnu/stubs.h /usr/include/stdint.h
HostListCheck.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
HostListCheck.o: /usr/include/bits/types.h
HostListCheck.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
HostListCheck.o: /usr/include/bits/socket.h /usr/include/limits.h
HostListCheck.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/limits.h
HostListCheck.o: /usr/include/sys/types.h /usr/include/time.h
HostListCheck.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
HostListCheck.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
HostListCheck.o: /usr/include/endian.h /usr/include/bits/endian.h
HostListCheck.o: /usr/include/bits/byteswap.h /usr/include/netdb.h
HostListCheck.o: /usr/include/bits/netdb.h /usr/include/stdio.h
HostListCheck.o: /usr/include/libio.h /usr/include/_G_config.h
HostListCheck.o: /usr/include/wchar.h /usr/include/gconv.h
HostListCheck.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
HostListCheck.o: /usr/include/bits/stdio_lim.h
IOInteractor.o: IOInteractor.h Interactor.h
Interactor.o: Interactor.h
MmapError.o: MmapError.h /usr/include/stdio.h /usr/include/features.h
MmapError.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
MmapError.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
MmapError.o: /usr/include/bits/types.h /usr/include/libio.h
MmapError.o: /usr/include/_G_config.h /usr/include/wchar.h
MmapError.o: /usr/include/bits/wchar.h /usr/include/gconv.h
MmapError.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
MmapError.o: /usr/include/bits/stdio_lim.h
MyExperiment.o: CReadoutMain.h CRunState.h CDocumentedPacketManager.h
MyExperiment.o: CExperiment.h CStatusModule.h CTrigger.h CScalerBank.h
MyExperiment.o: CScaler.h CCompoundEventSegment.h CEventSegment.h
MyExperiment.o: CInterpreterCore.h CPauseCommand.h CResumeCommand.h
MyExperiment.o: CRunVariableCommand.h CRunVariable.h CStateVariableCommand.h
MyExperiment.o: CTagBaseCommand.h CExitCommand.h CConstVariableCommand.h
MyExperiment.o: CConstVariable.h CStateVariable.h CVMEScalerLRS1151.h
MyExperiment.o: CTraditionalEventSegment.h
Nimout.o: Nimout.h
PasswordCheck.o: PasswordCheck.h Authenticator.h Interactor.h
StringInteractor.o: StringInteractor.h Interactor.h
TclAccessListCheck.o: TclAccessListCheck.h Authenticator.h Interactor.h
TclAccessListCheck.o: AccessListCheck.h
TclAuthorizer.o: TclAuthorizer.h /usr/include/string.h
TclAuthorizer.o: /usr/include/features.h /usr/include/sys/cdefs.h
TclAuthorizer.o: /usr/include/gnu/stubs.h
TclAuthorizer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
TclAuthorizer.o: /usr/include/assert.h /usr/include/netdb.h
TclAuthorizer.o: /usr/include/netinet/in.h /usr/include/stdint.h
TclAuthorizer.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
TclAuthorizer.o: /usr/include/bits/types.h /usr/include/bits/socket.h
TclAuthorizer.o: /usr/include/limits.h
TclAuthorizer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/limits.h
TclAuthorizer.o: /usr/include/sys/types.h /usr/include/time.h
TclAuthorizer.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
TclAuthorizer.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
TclAuthorizer.o: /usr/include/endian.h /usr/include/bits/endian.h
TclAuthorizer.o: /usr/include/bits/byteswap.h /usr/include/bits/netdb.h
TclAuthorizer.o: /usr/include/arpa/inet.h /usr/include/stdio.h
TclAuthorizer.o: /usr/include/libio.h /usr/include/_G_config.h
TclAuthorizer.o: /usr/include/wchar.h /usr/include/gconv.h
TclAuthorizer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
TclAuthorizer.o: /usr/include/bits/stdio_lim.h
UnixUserCheck.o: UnixUserCheck.h Authenticator.h Interactor.h
UnixUserCheck.o: /usr/include/pwd.h /usr/include/features.h
UnixUserCheck.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
UnixUserCheck.o: /usr/include/bits/types.h
UnixUserCheck.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
UnixUserCheck.o: /usr/include/crypt.h
VmeModule.o: VmeModule.h
