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
#include <stdexcept>
#include <vector>
#inclue <CReadoutModule.h>


/**
 * constructor
 *    - save the controller object.
 *    - null out the configuration.
 *
 *  @param controller - references the CCUSBController object.
 */
CCCUSBHighLevelController::CCCUSBHighLevelController(CCCUSB& controller) :
    m_pController(&controller),
    m_pConfiguration(0)
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
    
    m_Controller->writeUSBBulkTransferSetup(
        0 << CCUSB::TransferSetupRegsiter::timeoutShift
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
 * @throw std::logic_error - if ther is no configuration.
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
