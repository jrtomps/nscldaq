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
# @file   CCCUSBHighLevelController.h
# @brief  Provides high levelc ontroller functions for the CCUSB.
# @author <fox@nscl.msu.edu>
*/
#ifndef CCCUSBHIGHLEVELCONTROLLER_H
#define CCCUSBHIGHLEVELCONTROLLER_H

class CCCUSB;                       // CCUSB Controller.
class CConfiguration;

/**
 * @class CCCUSBHighLevelController
 *    Provides functionality for the CCUSB at a high level of abstraction.
 *    By high level of abstraction what I mean is that this class provides
 *    the top level operations that are normally performed, like loading stacks
 *    from the configuration, Setting up the controller for data taking.
 *    Performing module initialization etc. etc.
 *    This has been pulled out of the individual points that it occurs so that:
 *    -    Duplicate code is factored out from within chunks of the application.
 *    -    In some mythical future, this may derive from a base class that has
 *         both CC and VM usb subclasses allowing us to further unify the
 *         CC and VM usb readout programs (since there is so much more in
 *         common between those programs than there are differences).
 */
class CCCUSBHighLevelController
{
private:
    CCCUSBController*  m_pController;
    CConfiguration*  m_pConfiguration;
    
    // Canonical methods:
    
public:
    CCCUSBHighLevelController(CCCUSBController& controller);
    virtual ~CCCUSBHighLevelController();
    
    // operations:
    
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
    bool readData(void* pBuffer, unsigned maxBytes, unsigned& bytesRead);
};
#endif