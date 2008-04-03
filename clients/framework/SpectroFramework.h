/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __SPECTROFRAMEWORK_H
#define __SPECTROFRAMEWORK_H
#include <histotypes.h>

#include <CApplicationRegistry.h>
#include <CApplicationSerializer.h>
#include <CBufferEvent.h>
#include <CBufferMonitor.h>
#include <CBufferReactor.h>
#include <CClassifiedObjectRegistry.h>
#include <CDAQTCLProcessor.h>
#include <CDuplicateNameException.h>
#include <CDuplicateSingleton.h>
#include <ErrnoException.h>
#include <CEvent.h>
#include <CEventLoop.h>
#include <CEventMonitor.h>
#include <Exception.h>
#include <CFdMonitor.h>
#include <CFdReactor.h>
#include <CFileEvent.h>
#include <CIncompatibleMonitor.h>
#include <CInterpreterStartup.h>
#include <CLinkFailedException.h>
#include <CNamedObject.h>
#include <CNoSuchLinkException.h>
#include <CNoSuchObjectException.h>
#include <CObjectRegistry.h>
#include <RangeError.h>
#include <CReactor.h>
#include <CRefptr.h>
#include <CRegisteredObject.h>
#include <CServerConnectionEvent.h>
#include <CServerInstance.h>
#include <CSocket.h>
#include <StreamIOError.h>
#include <CTCLInterpreterStartup.h>
#include <CTCLSynchronizeCommand.h>
#include <CTCPBadSocketState.h>
#include <CTCPConnectionFailed.h>
#include <CTCPConnectionLost.h>
#include <CTCPNoSuchHost.h>
#include <CTCPNoSuchService.h>
#include <CTKInterpreterStartup.h>
#include <CThreadRecursiveMutex.h>
#include <CTimerEvent.h>
#include <CTimerMonitor.h>
#include <TCLApplication.h>
#include <TCLCommandPackage.h>
#include <TCLException.h>
#include <TCLFileHandler.h>
#include <TCLHashTable.h>
#include <TCLHashTableItem.h>
#include <TCLHashTableIterator.h>
#include <TCLIdleProcess.h>
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <TCLList.h>
#include <TCLObject.h>
#include <TCLPackagedCommand.h>
#include <TCLProcessor.h>
#include <TCLResult.h>
#include <TCLString.h>
#include <TCLTimer.h>
#include <TCLVariable.h>


#include <CTypeFreeBinding.h>
#include <CBinding.h>
#include <CVariableBinding.h>
#include <CArrayBinding.h>
#include <CAssocArrayBinding.h>
#include <CConfigurationManager.h>


#endif
