
H_FILES = mainexterns.h DAQMain.h maintypes.h daqtypes.h maindefs.h \
SyncGuard.h Synchronizable.h Printable.h DAQObject.h \
DAQObjectArray.h HashTable.h PrintStreamIface.h OstreamCompat.h \
PrintStream.h DAQVector.h DAQIntArray.h DAQCharArray.h DAQLogger.h \
DAQString.h DAQCommInfo.h DAQCommMsg.h DAQCommMsgIface.h \
DAQCommunicator.h DAQCommunicatorOut.h DAQConfigParser.h \
DAQConfigParser_H.h DAQCParserOut.h DAQDisplayHTML.h DAQDisplayHTML_H.h \
DAQDisplayProto.h DAQDisplayText.h DAQDisplayText_H.h DAQDisplayXML.h \
DAQOstreamOut.h DAQServConfig.h DAQURL.h LinkedList.h Queue.h \
daqcstr.h genmacros.h MutexIface.h Block.h BufferAllocator.h \
DAQPageBlock.h DAQBufferPageHeaderEncoder.h DAQBufferPage.h \
Bag.h Buffer.h Pointer.h DAQBufferAllocator.h DAQBuffer.h \
DAQByteBufferPtr.h DAQWordBufferPtr.h DAQDWordBufferPtr.h \
DAQByteBuffer.h DAQWordBuffer.h DAQDWordBuffer.h \
DAQPriorityBuffer.h DAQPriorityBufferPtr.h \
DAQBufferPager.h DAQMemory.h DAQAlarm.h DAQAlarmMsg.h  DAQTrap.h \
DAQSelectable.h DAQSelectObj.h DAQStatus.h DAQLink.h DAQException.h \
DAQExceptionFactory.h DAQExceptionType.h DAQRefCounter.h DAQRefObject.h \
DAQContext.h DAQNodeContext.h DAQPageCache.h


FILES = DAQMain spectrodaq_main main \
Synchronizable SyncGuard DAQObjectArray OstreamCompat \
PrintStream HashTable DAQVector DAQIntArray DAQCharArray DAQLogger \
DAQString DAQCommInfo DAQCommMsg DAQCommMsgIface DAQCommunicator \
DAQCommunicatorOut DAQConfigParser DAQConfigParser_H DAQCParserOut \
DAQDisplayHTML DAQDisplayHTML_H DAQDisplayText DAQDisplayText_H \
DAQDisplayXML DAQOstreamOut DAQServConfig DAQURL LinkedList Queue \
DAQPageBlock DAQBufferPageHeaderEncoder DAQBufferPage Bag Pointer \
DAQBufferAllocator DAQBuffer \
DAQByteBufferPtr DAQWordBufferPtr DAQDWordBufferPtr \
DAQByteBuffer DAQWordBuffer DAQDWordBuffer \
DAQPriorityBuffer DAQPriorityBufferPtr \
DAQBufferPager DAQMemory DAQAlarm DAQAlarmMsg DAQTrap DAQStatus \
DAQLink DAQException DAQExceptionFactory DAQRefCounter \
DAQContext DAQNodeContext DAQPageCache


include ../../config.make

all: include lib prog

lib: $(O_FILES)
	$(AR) $(DAQLIB) $(O_FILES)

include::
	$(CP) $(H_FILES) $(DAQINCDIR)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend
