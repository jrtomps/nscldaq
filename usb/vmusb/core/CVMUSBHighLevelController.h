/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CVMUSBHighLevelController.h
# @brief  High level functionality for the VMUSB controller.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVMUSBHIGHLEVELCONTROLLER_H
#define CVMUSBHIGHLEVELCONTROLLER_H

#include <stddef.h>
class CVMUSB;
class CConfiguration;

/**
 * @class CVMUSBHighLevelController
 *    Provides the high level interfaces fro the VMUSB.    The level of abstraction
 *    this class provides may allow us, in the future to subclass this and
 *    CCCUSBHighLevelController from a common base class and merge the VMSUBReadout
 *    and CCUSBReadout programs into a common XXUSBReadout program.
 *
 */
class CVMUSBHighLevelController {
private:
    CVMUSB*                 m_pController;             // Low level controller.
    CConfiguration*         m_pConfiguration;          // processed daqconfig.tcl
    bool                    m_haveScalerStack;
    
    // canonicals

public:
    CVMUSBHighLevelController(CVMUSB& controller);
    virtual ~CVMSUBHighLevelController();
    
    // Controlle high level operations:
public:
    void readConfiguration(const char* pFilename);
    void initializeModules();
    void initializeController();
    void loadStacks();
    void enableStacks();
    void performStartOperations();
    void performStopOperations();
    void startAcquisition();
    void stopAcquisition();
    void flushBuffers();
    void reconnect();
    bool checkStackSize();
    bool readData(void* pBuffer, size_t maxBytes, size_t& bytesRead, int timeout);    
};

#endif