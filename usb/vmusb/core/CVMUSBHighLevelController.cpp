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
# @file   CVMUSBHighLevelController.cpp
# @brief  High level functionality for a VMUSB controller.
# @author <fox@nscl.msu.edu>
*/

#include "CVMUSBHighLevelController.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CConfiguration.h>
#include <CStack.h>
#include <Globals.h>
#include <TclServer.h>
#include <CReadoutModule.h>

#include <stdexcept>


static const unsigned MAX_TOTAL_STACKSIZE=1*1024;   // Size of stack memory.

/**
 * constructor
 *    @param controller - the low level we work through.
 */
CVMUSBHighLevelController::CVMUSBHighLevelController(CVMUSB& controller) :
   m_pController(&controller),
   m_pConfiguration(0),
   m_haveScalerStack(false)
{

}
/**
 * destructor
 */
CVMUSBHighLevelController::~CVMUSBHighLevelController()
{
    // If there's no active configuration, m_pConfiguration is 0 so this is ok:
    
    delete m_pConfiguration;
}
/**
 * readConfiguration
 *    Establish an active DAQ configuration from the specified file path.
 *    Any existing configuration is discarded.
 *
 * @param pFilename - name of the file that contains the configuration (daqconfig.tcl e.g.).
 */
void
CVMUSBHighLevelController::readConfiguration(const char* pFilename)
{
    delete m_pConfiguration;
    m_pConfiguration = 0;   // In case there's an exception reading the new config.
    
    m_pConfiguration = new CConfiguration;
    m_pConfiguration->processConfiguration(pFilename);
    
}
/**
 * initializeModules
 *    Iterate over the stacks defined in the current configuration and
 *    invoke their initialization methods.  This will in turn initialize all
 *    active modules in the configuration.
 *
 *  @throw std::logic_error if a configuration has not bee established.
 */
void
CVMUSBHighLevelController::initializeModules()
{
    if(!m_pConfiguration) {
        throw std::logic_error(
            "CVMUSBHighLevelController::initializeModules - no configuration has been established."
        );
    }
    // Get the stacks and iterate over them:
    
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    
    for (auto p = stacks.begin(); p != stacks.end(); p++) {
        CStack* pStack = dynamic_cast<CStack*>((*p)->getHardwarePointer());
        pStack->Initialize(*m_pController);
    }
}
/**
 *  initializeController
 *     Setup the default configuration of the VMUSB controller.  This should
 *     normally be called prior to initializeModules as there can be modules
 *     in stacks that modify the controller configuration.
 */
void
CVMUSBHighLevelController::initializeController()
{
  m_pController->writeActionRegister(0);

  // Set up the buffer size and mode:

  m_pController->writeBulkXferSetup(0 << CVMUSB::TransferSetupRegister::timeoutShift); // don't want multibuffering...1sec timeout is fine.

  /* The global mode:
     13k buffer
     Bus request level 4.
     Flush scalers on a single event.
  */
  
  m_pController->writeGlobalMode((4 << CVMUSB::GlobalModeRegister::busReqLevelShift) | 
                                (CVMUSB::GlobalModeRegister::bufferLen13K << 
                                  CVMUSB::GlobalModeRegister::bufferLenShift));
    
}
/**
 *  loadStacks
 *      Loads all stacks that have been defined in the configuration to the
 *      controller.    Stacks are not enabled for data taking yet.
 *
 * @throw std::logic_error - if no configuration has been established.
 */
void
CVMUSBHighLevelController::loadStacks()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CVMUSBHighLevelController::loadStacks - Configuration is not yet established"
        );
    }
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    CStack::resetStackOffset();
    m_haveScalerStack = false;
    
    for (auto p = stacks.begin(); p != stacks.end(); p++) {
        CStack* pStack = dynamic_cast<CStack*>((*p)->getHardwarePointer());
        pStack->loadStack(*m_pController);
        if (pStack->getTriggerType() == CStack::Scaler) {
            m_haveScalerStack = true;
        }
    }
    // Could also be a monitor list present:
    
    TclServer* pServer = ::Globals::pTclServer;
    CVMUSBReadoutList list = pServer->getMonitorList();
    if (list.size() > 0) {
        m_pController->loadList(7, list, CStack::getOffset());
    }
}
/**
 * enableStacks
 *    Iterates through the defined stacks and enables each of them.
 *    Note that once this is done, the devices managed by those stacks can
 *    acquire data.
 *
 * @throw std::logic_error - if no configuration has been established.
 */

void CVMUSBHighLevelController::enableStacks()
{
if (!m_pConfiguration) {
        throw std::logic_error(
            "CVMUSBHighLevelController::enableStacks - Configuration is not yet established"
        );
    }
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    
    for (auto p = stacks.begin(); p != stacks.end(); p++) {
        CStack* pStack = dynamic_cast<CStack*>((*p)->getHardwarePointer());
        pStack->enableStack(*m_pController);
    }
}
/**
 * performStartOperations
 *    This interface allows the addition of special start actions.   At present
 *    these are not implemented.  Normally performStartActions are invoked
 *    just prior to turning on data taking.
 */
void
CVMUSBHighLevelController::performStartOperations()
{}
/**
 *  peformStopOperations
 *     Invoked to perform special end of run operations bound to each stack.
 *
 *  @throw std::logic_error if no configuration is established (and that logic
 *         error must be a doozy because either this is being invoked when data
 *         taking was never started or somehow the configuration got wiped out
 *         between starting the run and ending it).
 */

void
CVMUSBHighLevelController::performStopOperations()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CVMUSBHighLevelController::performStopOperations - Configuration is not yet established"
        );
    }
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    for (auto p = stacks.begin(); p != stacks.end(); p++) {
        CStack* pStack = dynamic_cast<CStack*>((*p)->getHardwarePointer());
        pStack->onEndRun(*m_pController);
    }
}
/**
 * startAcquisition
 *    Set the controller into autonomous data acquisition mode.
 *    Prior to calling this:
 *    -   loadStacks must be called.
 *    -   enableStacks must be called.
 *    if not behavior is not well defined.
 */
void
CVMUSBHighLevelController::startAcquisition()
{
        m_pController->writeActionRegister(CVMUSB::ActionRegister::startDAQ);
}
/**
 * stopAcquisition
 *    Turns off data acquisition
 *    -  If there's a scaler stack, a dump scalers is forced.
 */
void
CVMUSBHighLevelController::stopAcquisition()
{
    // Force scaler dump if needed 
    if (m_haveScalerStack) {
        m_pController->writeActionRegister(CVMUSB::ActionRegister::scalerDump);
    }
    // Turn off daq
    
    m_pController->writeActionRegister(0);
}
/**
 *  flushBuffers
 *
 *    Ensure that the VMUSB has no data hanging around in its output
 *    fifo buffers.  We're just going to issue reads with a relatively short timeout
 *    until they complete.  This should be performed with data acquisition disabled
 *    and the data drained from the controller.
 */
void
CVMUSBHighLevelController::flushBuffers()
{
    int status;
    do {
        char buffer[13*1024*sizeof(uint16_t)];  // 13Kword buffer is the biggest.
        size_t bytesRead;
        status = m_pController->usbRead(
            buffer, sizeof(buffer), &bytesRead, 1000
        );
    } while (status == 0);
    
    // Check for errno != ETIMEDOUT -- which is what we want to see here?
}
/**
 * reconnect
 *    Reconnect with the controller.  The current controller handle
 *    is closed and the Controller is located and reconnected.
 *    this can be called when there is a possibility the power has
 *    been cycled in the VME crate (e.g. when the run is off).
 */
void
CVMUSBHighLevelController::reconnect()
{
    m_pController->reconnect();
}
/**
 * checkStackSize
 *    Ensure the stacks will fit in the VMUSB stack memory
 *    - Obtain the vector of stack objects.
 *    - Sum the size of each vetor
 *    - Ensure the summed value is less than the maximum stack memory
 *      of the VMUSB
 *
 * @return bool - True if the stacks will fit.
 */
bool CVMUSBHighLevelController::checkStackSize()
{
    std::vector<CReadoutModule*> stackModules =
        m_pConfiguration->getStacks();
    CVMUSBReadoutList combinedStack;
    for (auto p = stackModules.begin(); p != stackModules.end(); p++) {
        CStack* s = reinterpret_cast<CStack*>((*p)->getHardwarePointer());
        s->addReadoutList(combinedStack);
    }
    // There could also be a monitor stack:
    
    TclServer* pServer = ::Globals::pTclServer;
    CVMUSBReadoutList monList = pServer->getMonitorList();
    
    
    return (combinedStack.size() + monList.size()) < MAX_TOTAL_STACKSIZE;
}
/**
 *  readData
 *     Read dats from the input buffer.
 * @param pBuffer - points to a buffer into which the data will be read.
 * @param maxBytes - Maximum number of bytes that can be stored in the
 *                   buffer pointed to by pBuffer.
 * @param bytesRead - Reference to a size_t into which will be written
 *                    the actual byte transfer count.
 * @param timeout   - Timeout in milliseconds for the transfer.
 * @return bool     - true on success, false on failure with errno having
 *                     a failure reason.
 *
 */
bool
CVMUSBHighLevelController::readData(
    void* pBuffer, size_t maxBytes, size_t& bytesRead, int timeout
)
{   
    int status = m_pController->usbRead(pBuffer, maxBytes, &bytesRead, timeout);
    return status == 0;
}