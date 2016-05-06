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
# @file   CCCUSBHighLevelController.cpp
# @brief  Implementation of high level controller operations for CCUSBs.
# @author <fox@nscl.msu.edu>
*/
#include "CCCUSBHighLevelController.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>
#include <CConfiguration.h>
#include <CStack.h>
#include <stdexcept>
#include <vector>
#include <CReadoutModule.h>

static const size_t MAX_STACK_STORAGE(1024);


/**
 * constructor
 *    - save the controller object.
 *    - null out the configuration.
 *
 *  @param controller - references the CCUSBController object.
 */
CCCUSBHighLevelController::CCCUSBHighLevelController(CCCUSB& controller) :
    m_pController(&controller),
    m_pConfiguration(0),
    m_haveScalerStack(false)
{}
/**
 * destructor
 *      If there's still a configuration object hiding around we need to
 *      delete it.  Note that delete 0 is a no-op so:  Note also that the
 *      controller object is owned by our client so we won't kill it off.
 */
CCCUSBHighLevelController::~CCCUSBHighLevelController()
{
    delete m_pConfiguration;
}

/**
 * readConfiguration
 *    Read a new configuration file.
 *    -  Delete any old CConfiguration
 *    -  Load in any new configuration.
 *
 *    @note exceptions must be handled by the caller.
 *
 * @param pFilename - pointer to null terminated filename path.
 */
void
CCCUSBHighLevelController::readConfiguration(const char* pFilename)
{
    delete m_pConfiguration;
    m_pConfiguration = 0;
    m_pConfiguration = new CConfiguration;
    m_pConfiguration->processConfiguration(pFilename);
}
/**
 *  initializeModules
 *     For each module in defined stacks in the configuration,
 *     invoke the initialization method.
 *
 *  @throw - std::logic_error - if the configuration is not defined.
 *  @throw - other from the actual initialization if there is a detectable Error.
 */
void
CCCUSBHighLevelController::initializeModules()
{
    // Throw logic error if there's no configuration yet:
    
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CCCUSBHighLevelController::initializeModules: The configuration has not yet been read!"
        );
    }
    // Get the stacks, iterate through them and initialize:
    
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    for (int i =0; i < stacks.size(); i++) {
        CStack* pStack = dynamic_cast<CStack*>(stacks[i]->getHardwarePointer());
        pStack->Initialize(*m_pController);
    }
}
/**
 * initializeController
 *    Initialize the controller internal registers.  Note that these register
 *    values could be modified by initializeModule if the configuration
 *    contains a module that plays with those registers.  It's therefore
 *    wise to invoke this method prior to initializeModules().
 */
void
CCCUSBHighLevelController::initializeController()
{
    // Turn off data taking and flush the controller -- sometimes juni is left
    
    m_pController->writeActionRegister(0);           // Ensure daq is stopped.
    char junk[100000];
    size_t junkRead;
    m_pController->usbRead(junk, sizeof(junk), &junkRead, 1*1000);
    
    // Bulk transfer register:   just set 1 second usb timeout:
    
    m_pController->writeUSBBulkTransferSetup(
        0 << CCCUSB::TransferSetupRegister::timeoutShift
    );
    
    // Default values for global mode reg:
    //  - 4k buffer.
    //  - single event separator word.
    //  - single header word:
    
    m_pController->writeGlobalMode(
        (CCCUSB::GlobalModeRegister::bufferLen4K <<
            CCCUSB::GlobalModeRegister::bufferLenShift)
    );
    // Set up default output device sources:
    //  O1 - Bisy
    //  O2 - Acquire.
    //  O3 - End of busy pulse.
    
    m_pController->writeOutputSelector(
        CCCUSB::OutputSourceRegister::nimO1Busy |
        CCCUSB::OutputSourceRegister::nimO2Acquire |
        CCCUSB::OutputSourceRegister::nimO3BusyEnd
    );
     
    
}
/**
 *  loadStacks
 *     Load all stacks defined in the configuration to the controller.
 *
 *   @throw std::logic_error - if there is no configuration.
 */
void
CCCUSBHighLevelController::loadStacks()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CCCUSBHighLevelController::loadStacks - no configuration yet"
        );
    }
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    for (int i =0; i < stacks.size(); i++) {
        CStack* pStack = dynamic_cast<CStack*>(stacks[i]->getHardwarePointer());
        pStack->loadStack(*m_pController);
    }
}
/**
 * enableStacks
 *    Enable all stacks - the stacks should first have been loaded with
 *    loadStacks - though I'm not convinced the VM-USB requires it...it's best
 *    to do so as once enabled a stack can trigger and, if not loaded probably
 *    make the VMUSB go insane.
 *
 * @throw std::logic_error - if there is no configuration.
 */
void
CCCUSBHighLevelController::enableStacks()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CCCUSBHighLevelController::enableStacks - no configuration yet"
        );
    }
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    for (int i =0; i < stacks.size(); i++) {
        CStack* pStack = dynamic_cast<CStack*>(stacks[i]->getHardwarePointer());
        pStack->enableStack(*m_pController);
    }
}
/**
 * performStartOperations
 *   Placeholder method in case we later want to add the capability for other
 *   scripts to run at start time (after initializeModules e.g.)
 */
void
CCCUSBHighLevelController::performStartOperations()
{
    
}
/**
 * performStopOperations
 *   Perform the end run operations on all stacks.
 *
 * @throw std::logic_error - if there is no configuration.
 */
void
CCCUSBHighLevelController::performStopOperations()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CCCUSBHighLevelController::performStopOperations - no configuration yet"
        );
    }
    std::vector<CReadoutModule*> Stacks = m_pConfiguration->getStacks();
    for(int i =0; i < Stacks.size(); i++) {
      CStack* pStack = dynamic_cast<CStack*>(Stacks[i]->getHardwarePointer());
      pStack->onEndRun(*m_pController);    // Call onEndRun for daq hardware associated with the stack.
    }
      
    
}
/**
 * startAcquisition
 *    Do what's needed to start data taking:
 *    - Determine if we need to force a scaler dump at end of run.
 *    - Write the action register with the datataking bit.
 *
 * @throw std::logic_error - if there is no configuration.
 */
void
CCCUSBHighLevelController::startAcquisition()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CCCUSBHighLevelController::performStopOperations - no configuration yet"
        );
    }
    std::vector<CReadoutModule*> Stacks = m_pConfiguration->getStacks();
    m_haveScalerStack = false;
    for(int i =0; i < Stacks.size(); i++) {
      CStack* pStack = dynamic_cast<CStack*>(Stacks[i]->getHardwarePointer());
      if (pStack->getTriggerType() == CStack::Scaler) m_haveScalerStack = true;
    }
    
    m_pController->writeActionRegister(CCCUSB::ActionRegister::startDAQ);
}
/**
 * stopAcquisition
 *    turn off the startDAQ bit in the action register and, if appropriate,
 *    simlutaneously set the dump scaler bit.  Appropriate is defined by the
 *    state of the m_haveScalerStack flag.
 */
void
CCCUSBHighLevelController::stopAcquisition()
{
    
    if (m_haveScalerStack) {
        m_pController->writeActionRegister(CCCUSB::ActionRegister::scalerDump);
    }
    m_pController->writeActionRegister(0);
}
/**
 * flushBuffers
 *   Reads buffers from the CCUSB until timeout fires.  This is normally used
 *   to empty the CCUSB of data after data taking was turned off at initialization
 *   time. This is done to ensure the CCCUSB can be programmed even if it was
 *   left in acquisition mode by the last use.
 */
void
CCCUSBHighLevelController::flushBuffers()
{
    int status;
    do {
        char buffer[8192];                // Largest buffer.
        size_t bytesRead;
        status = m_pController->usbRead(buffer, sizeof(buffer), &bytesRead, 1000);
        
    } while (status == 0);
    
    // Should we also check that errno == ETIMEDOUT?
}
/**
 * reconnect
 *     Reconnect the controller object to the hardware.  This is necessary
 *     if the hardware was power cycled since the last use (e.g. when
 *     the run was halted the crate was cycled to change a module).
 */
void
CCCUSBHighLevelController::reconnect()
{
    m_pController->reconnect();
}
/**
 * checkStackSize
 *    Totals up the size requirements of the event and scaler stacks and
 *    determines if that will overflow the stack memory of the module.
 *
 *
 *  @return bool -true if the stack size is ok. false  if not.
 *  @throw std::logic_error if the configuration has not yet been defined.
 */
bool
CCCUSBHighLevelController::checkStackSize()
{
    if (!m_pConfiguration) {
        throw std::logic_error(
            "CCCUSBHighLevelController::checkStackSize: no configuration defined yet"
        );
    }
    CCCUSBReadoutList fullstack;
    std::vector<CReadoutModule*> stacks = m_pConfiguration->getStacks();
    for (int i = 0; i < stacks.size(); i++) {
        stacks[i]->addReadoutList(fullstack);
    }
    return fullstack.size() < MAX_STACK_STORAGE;
}
/**
 * readData
 *    Reads a block of data from the CCUSB
 *
 *  @param pBuffer - points to where to put the data read from the device.
 *  @param maxBytes - Maximum number of bytes to read (pBuffer must point
 *                    to at least this number of bytes of storage).
 *  @param bytesRead - Reference to an unsigned that will receive the number
 *                    of bytes actually read from the device.
 *  @param timeout    - Maximum number of milliseconds to block for the read.
 *
 *  @return bool - true if the read succeeded or false if not.  If the read failed,
 *                     errno has the reason for the failure.  Note that
 *                     ETIMEDOUT is not necessarily an error.  If the timeout is
 *                     short and the data rate is low this may just indicate the
 *                     CCUSB buffer has not filled by the time the timeout expired.
 */
bool
CCCUSBHighLevelController::readData(
    void* pBuffer, size_t maxBytes, size_t& bytesRead, int timeout
)
{
    int status = m_pController->usbRead(pBuffer, maxBytes, &bytesRead, timeout);
    return status == 0;
}
