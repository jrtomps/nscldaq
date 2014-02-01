/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/**
 *  This file contains a template device driver for the CC-USB 
 * readout framework.  While these comments are no substitute
 * for the documentation available online at http://docs.nscl.msu.edu
 * The idea is that you will have to modify several sections of this code
 * Each section you need to modify is bracketed with comments of the form
 *  MODIFY ME HERE and
 *  END MODIFICATIONS
 *
 *  Comments above eac modification describe what you are doing and to some extent why.
 *
 * The final objective of this exercise is to produce a Tcl loadable package that you
 * can incorporate into the CC-USB readout framework via a load /some/path/to/yourpackage.so
 * command in your configuration file.
 *
 * As such each driver consists of two segments:
 * The main chunk is device driver code.  Device driver code 
 *
 * - Establishes configuration parameters and their constraints.
 * - Uses the values of those configuration parameters to initialize an instance of the device.
 * - Uses the values of the configuration parameters to contribute the CCUSB instructions required
 *   to read the device to the CC-USB readout list to which the module was assigned.
 *
 *  The second chunk is package initialization code. In that chunk, we need to make your driver
 *  known to the configuration subsystem, assigning it a Tcl command ensemble that can
 *  create, configure, and  query the configuration of instances of the driver.
 */


/*
 *  This template builds a driver class named CTemplateDriver  The first thing you should do
 *  is do a global search and replace fo CTemplateDriver to a name that matches the
 *  device you are trying to support.

 * MODIFY ME HERE
 *
 *  END MODIFICATIONS>
 */
#include <CReadoutHardware.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CUserCommand.h>
#include <tcl.h>
#include <iostream>


#include <cstdlib>
#include <string>
#include <sstream>
#include <stdint.h>
#include <unistd.h>

#include "CXLMFERA.h"

// VME Interface addresses
static const uint32_t Interrupt   (0x000004); // Interrupt/reset register.
static const uint32_t BootSrc     (0x000008);
static const uint32_t ForceOffBus (0x00000c);

static const uint32_t BusAOwner   (0x010000);
static const uint32_t BusBOwner   (0x010004);
static const uint32_t BusXOwner   (0x010008);

// Boot source values
static const uint32_t BootSRAMA   (0x010000);


static const uint32_t FPGADAQEnable (0x000008);
static const uint32_t FPGAWaitLoops (0x00000c);

static const uint8_t  registerAmod     (CVMUSBReadoutList::a32UserData);
static const uint8_t  blockTransferAmod(CVMUSBReadoutList::a32UserBlock);

static const float    vmusbClockTick(12.5);
static const float    busDelay(200/vmusbClockTick); // delay to allow for arbitration

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and other 'canonical' methods

/**
 * Construct an instance of the device.  Note that in this framework this will
 * typically only be used to make a 'template' instance which will be cloned to
 * create instances that are bound to configurations and actual hardware.
 */
CXLMFERA::CXLMFERA() : CXLM()
{
    // This will point to an instance's config base class protected).
    //  m_pConfiguration = 0;	

}

/**
 * Copy construction.  This cannot be virtual by the rules of C++ the clone()
 * method normally creates a new object from an existing template object.
 * 
 * @param rhs  - Template device that is being copied to create the  new device.
 */
CXLMFERA::CXLMFERA(const CXLMFERA& rhs) : CXLM(rhs)
{

}
/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CXLMFERA::~CXLMFERA() 
{
    // will leak memory if we copy constructed

}
///////////////////////////////////////////////////////////////////////////////////////
// Interfaces the driver provides to the framework.

/**
 * This function is called when an instance of the driver has been associated with
 * its configuration database.  The template code stores that in m_pConfiguration
 * The configuration is a CReadoutModule which in turn is derived from
 * CConfigurableObject which encapsulates the configuration database.
 *
 *  You need to invoke methods from CConfigurableObject to create configuration parameters.
 *  by convention a configuration parameter starts with a -.  To illustrate this,
 *  template code will create a -base parameter that captures the base address of the module.
 *  In addition we'll create an -id parameter which will be the value of a marker that will
 *  be put in the event.  The marker value will be constrainted to be 16 bits wide.
 *
 * @parm configuration - Reference to the configuration object for this instance of the driver.
 */
void
CXLMFERA::onAttach(CReadoutModule& configuration)
{
    // superclass adds the -base and -firmware flags
    CXLM::onAttach(configuration);

  // The -id parameter is the value of the marker inserted in the event.
  // This sets bounds on the limits of the configuration id read at FPGA address
  uint32_t max = 2147483647; // 2^31 - 1
  m_pConfiguration->addIntegerParameter("-configurationID",
                                        0,
                                        max,
                                        0x54000041);

    // Allow user to add an identifier specify this data as a packet
    // if it is negative, then ignore it
  m_pConfiguration->addIntegerParameter("-id",-1);
  

}
/**
 * This method is called when a driver instance is being asked to initialize the hardware
 * associated with it. Usually this involves querying the configuration of the device
 * and using VMUSB controller functions and possibily building and executing
 * CVMUSBReadoutList objects to initialize the device to the configuration requested.
 * 
 * @param controller - Refers to a CCUSB controller object connected to the CAMAC crate
 *                     being managed by this framework.
 *
 */
void
CXLMFERA::Initialize(CVMUSB& controller)
{
    using std::hex;
    using std::dec;

    // ensure that red turns on BERR
    // We'll almost always need the module base address.  This line gets it from the
    // configuration database for you:
    uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
    int id = m_pConfiguration->getIntegerParameter("-id");

    std::cout << hex << "\n*** CXLMFERA @ base=0x" << base << ") " << dec;
    std::cout << "initialization ... " << std::endl;

    if (id<0) {
        std::cout << "User specified no id for this data.";
        std::cout << "None will be outputted";
        std::cout << std::endl;
    } 
    try {

        std::string firmwareFname = m_pConfiguration->cget("-firmware"); 

        // Was previously the AXLM72V_CES::Configure

        // Load firmware file and also boot the XLM
        // loadFirmware(controller,firmwareFname);
        myloadFirmware(controller,firmwareFname);


        // Begins the AXFERA::Initialize()
        checkConfiguration(controller);

        initializeFPGA(controller);

        // Adds some delay to test if the bad sramA[0] value is 
        // due to incomplete initialization
        CVMUSBReadoutList list;
//  Added just now.
        list.addDelay(100);
//        addClear(list);
        list.addDelay(100);
        uint32_t dummy;
        size_t dsize;

        int status = controller.executeList(list,(void*)&dummy,sizeof(dummy),&dsize);

        if (status<0) {
            std::ostringstream errmsg;
            errmsg << "CXLMFERA::Initialize(CVMUSB&) ";
            errmsg << "failed to execute list on delay with error ";
            errmsg << "(status=" << status << ")";
            throw errmsg.str(); 
        } 

    }
    catch (std::string& what) {
        std::cerr << "!!! " << what << std::endl;
    }

  /* END MODIFICATIONS */

}

void CXLMFERA::myloadFirmware(CVMUSB& controller, std::string fname)
{
    using namespace std;

    const size_t AXLM72V_CES_CONFIG_LENGTH = 220000;
	FILE *file;
	int i, j;
	uint32_t word, llow, lhigh, check;
	unsigned char blow, bhigh;
	unsigned char bytes[AXLM72V_CES_CONFIG_LENGTH];
	file = fopen(fname.c_str(), "r");
	if (!file) {
		cout << "Failed to open configuration file " << fname << endl;
		exit(1);
	}
	int length = fread(bytes, 1, AXLM72V_CES_CONFIG_LENGTH, file);
	if (!feof(file)) {
		cout << "Failed to reach EOF of configuration file " << fname << endl;
		cout << "Length read is: " << length << endl;
		exit(1);
	}
	fclose(file);
	// Detect first 0xFF in file
	for (i=0; i<length; i++) {
		if (bytes[i] == 0xFF) break;
	}

    // Busses to request 
    const uint32_t busses = REQ_A | REQ_X;

    // Acquire the bus:
    // 1. inhibit FPGA and DSP
    // 2. Request busses A and X
    CXLMBusController lock(controller,*this,busses,1,0);

    CVMUSBReadoutList list;
    list.clear();

	// Translate and write configuration to SRAMA
	for (j=i; j<length; j++) {
		blow = bytes[j]&0xF;
		bhigh = (bytes[j]&0xF0)>>4;
		llow = ((blow&0x7)<<2) + ((blow&0x8)<<7);
		lhigh = ((bhigh&0x7)<<2) + ((bhigh&0x8)<<7);
		word = llow + (lhigh<<16);

    list.addWrite32(sramA()+(j-i)*sizeof(uint32_t),
        CVMUSBReadoutList::a32UserData,
        word);       
    list.addRead32(sramA()+(j-i)*sizeof(uint32_t),
        CVMUSBReadoutList::a32UserData);       
    if ((j-i)%64 == 0) {

      uint32_t data[2048];
      size_t nbytes;
      int status = controller.executeList(list,
          &data,
          sizeof(data), &nbytes);
      if (status < 0) {
        string msg   = "000 CXLM::loadSRAMA - list execution failed to load the SRAM: ";
        msg         += -100;
        throw msg;
      }
      list.clear();
    }
  }

    // set boot src
    uint32_t bootSrcAddr = Interface() + BootSrc; 
    list.addWrite32(bootSrcAddr, registerAmod, BootSRAMA);
    list.addRead32(bootSrcAddr,CVMUSBReadoutList::a32UserData);       

    uint32_t data[2048];
    size_t nbytes;
    int status = controller.executeList(list,
            &data,
            sizeof(data), &nbytes);
    if (status < 0) {
        string msg   = "1111 CXLM::loadSRAMA - list execution failed to load the SRAM: ";
        msg         += -100;//error;
        throw msg;
    }
    list.clear();

    bootFPGA(controller);

    // bus releases automagically.
}


void CXLMFERA::bootFPGA(CVMUSB& controller)
{
    uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
    
    // Acquire busses A and X
//    uint32_t busses = REQ_A | REQ_X;
//    CXLMBusController lock(controller,*this,busses,1,busDelay);
//    std::cout << "   Booting FPGA ..." << std::flush;
//
    CVMUSBReadoutList list;

//    // set boot src
//    uint32_t bootSrcAddr = Interface() + BootSrc; 
//    list.addWrite32(bootSrcAddr, registerAmod, BootSRAMA);

    // reset fpga
    uint32_t resetAddr = Interface() + Interrupt; 
    list.addWrite32(resetAddr, registerAmod, uint32_t(1));
    list.addWrite32(resetAddr, registerAmod, uint32_t(0));

    // execute list
    uint32_t data = 0;
    size_t nbytes = 0;
    int status = controller.executeList(list,(void*)&data,sizeof(data),&nbytes);

    // Sleep for a while before any more io operations
    // This is time for the fpga to boot
    ::usleep(1000000); 

    if (status < 0) {

        throw std::string("CXLMFERA::bootFPGA(CVMUSB&) failed to boot FPGA");
    }

}

void CXLMFERA::initializeFPGA(CVMUSB& controller)
{
    // Begin XFERA::Initialize
    const uint32_t busRequestAddr = Interface();
    const uint32_t busses = REQ_A | REQ_X;

    std::cout << "--- Initializing FPGA ..." << std::flush;

    // use a scoped bus locking scheme for exception safety
    CXLMBusController busLock(controller,*this,busses,1,busDelay); 
    
    // at this point the VMEbus owns the A and X buses... 
    // start sending commands
    const uint32_t daqEnableAddr  = FPGA() + FPGADAQEnable;
    const uint32_t nWaitLoopsAddr = FPGA() + FPGAWaitLoops;

    uint32_t enable = 1;
    uint32_t nLoopsToWait = 0;
    uint32_t data;

    int status = controller.vmeWrite32(daqEnableAddr,registerAmod,enable);
    if (status<0) {
        std::cout << "after write daqenable...error " << status << std::endl;
    }

    status = controller.vmeWrite32(nWaitLoopsAddr,registerAmod,nLoopsToWait);
    if (status<0) {
        std::cout << "after write nloops...error " << status << std::endl;
    }

    status = controller.vmeRead32(nWaitLoopsAddr,registerAmod,&data);
    if (status<0) {
        std::cout << "after read nloops...error " << status << std::endl;
    }
   
    if ((data&0xff)!=nLoopsToWait) {
        std::cout << " FAILURE" << std::endl;
        std::ostringstream errmsg;
        errmsg << "CXLMFERA::initialize(CVMUSB&)";
        errmsg << " Wrote FPGA wait loops = " << nLoopsToWait;
        errmsg << " but read back = " << data;
        throw errmsg.str();
    }

    std::cout << " COMPLETED" << std::endl;
}

void CXLMFERA::Clear(CVMUSB& controller)
{
    const uint32_t busRequestAddr = Interface();
    const uint32_t busses = REQ_A | REQ_B;
    CXLMBusController(controller,*this,busses,1,busDelay);

    // clear first word of each sram block. 
    controller.vmeWrite32(sramA(),registerAmod,uint32_t(0x0));
    controller.vmeWrite32(sramB(),registerAmod,uint32_t(0x0));

    // bus releases automatically
}

void CXLMFERA::addClear(CVMUSBReadoutList& list)
{
    const uint32_t busRequestAddr = Interface();
    const uint32_t busses = REQ_A | REQ_B;

    addBusAccess(list,busses,0);

    // clear first word of each sram block. 
    list.addWrite32(sramA(),registerAmod,uint32_t(0x0));
    list.addWrite32(sramB(),registerAmod,uint32_t(0x0));

    addBusAccess(list,uint32_t(0),0);
}

/** 
  * Read back the value stored in
*/
void CXLMFERA::checkConfiguration(CVMUSB& controller)
{

    std::cout << "--- Validating FPGA configuration ..." << std::flush;

    uint32_t configID 
        = m_pConfiguration->getUnsignedParameter("-configurationID");

    uint32_t inhibitAddr= Interface() + ForceOffBus;

    CVMUSBReadoutList cmdList;

    cmdList.addWrite32(inhibitAddr, registerAmod,uint32_t(1));
    addBusAccess(cmdList,CXLM::REQ_X,::busDelay);    

    cmdList.addRead32(FPGA(),registerAmod);

    cmdList.addWrite32(inhibitAddr, registerAmod,uint32_t(0));
    addBusAccess(cmdList,0,0); // release bus

    uint32_t data[2];
    size_t nBytesRead=0;

    controller.executeList(cmdList,&data,sizeof(data),&nBytesRead);

    if (data[0] != configID) {
        std::cout << " FAIL";
        std::cout << std::endl;

        std::ostringstream errmsg;
        errmsg << "CXLMFERA::checkConfiguration(CVMUSB&) ";
        errmsg << "FPGA configuration (" << data[0] << ")";
        errmsg << " does not match user specified configurationID ";
        errmsg << "(" << configID << ")"; 
        errmsg << " read " << nBytesRead;
        throw errmsg.str(); 
    } 
    std::cout << " SUCCESS";
    std::cout << std::endl;

}

/**
 * This method is called to ask a driver instance to contribute to the readout list (stack)
 * in which the module has been placed.  Normally you'll need to get some of the configuration
 * parameters and use them to add elements to the readout list using CCUSBReadoutList methods.
 *
 * @param list - A CCUSBReadoutList reference to the list that will be loaded into the
 *               CCUSB.
 */
void CXLMFERA::addReadoutList(CVMUSBReadoutList& list)
{

    int id = m_pConfiguration->getIntegerParameter("-id");
    if (id>=0) list.addMarker(static_cast<uint16_t>(id));

//    list.addDelay(150);
//    for (int i=0; i<10; ++i) {
//        list.addRead32(Interface()+BusAOwner,registerAmod);
//        list.addRead32(Interface()+BusBOwner,registerAmod);
//    }

    addSramAReadout(list);

    if (id>=0) list.addMarker(static_cast<uint16_t>(id));

    addClear(list);
}


void CXLMFERA::addSramAReadout(CVMUSBReadoutList& list)
{
    //  this wil be a bit different than before because we wont use 
    // the XLMBusLock for bus access. This needs to be a self contained
    // set of commands.
    //
    // inhibit other FPGA and DSP from obtaining bus control
    // This arbitrates the bus master to be VMEbus unconditionally
    uint32_t inhibitAddr= Interface() + ForceOffBus;
    uint32_t enableInhibit = 1;
    list.addWrite32(inhibitAddr, registerAmod, enableInhibit);

    // Request VMEbus control of bus 
    const uint32_t busses = REQ_A;
//    addBusAccess(list,busses,busDelay);
    addBusAccess(list,busses,0);


    // Dynamically read out data stored in sramA given the number
    // stored at sramA  
    uint32_t sramAAddr = sramA();
    uint32_t numberMask = 0xFFFF; // specifies the bits to interpret as
                                  // the number of words to read.
    list.addBlockCountRead32(sramAAddr,numberMask,registerAmod);

    // Now add the block transfer whose length is dtermined by the
    // previous command. 
//    list.addMaskedCountBlockRead32(sramAAddr+0x000000,
    list.addMaskedCountBlockRead32(sramAAddr,
                                   blockTransferAmod);

    // Release the bus
//    addBusAccess(list,0,busDelay);
    addBusAccess(list,0,0/*no delay*/);
    list.addWrite32(inhibitAddr, registerAmod,uint32_t(0));

    // the prevous driver never cleared the fpga so I wont as well.

}

/**
 * This method virtualizes copy construction by providing a virtual method that
 * invokes it.  Usually you don't have to modify this code.
 *
 * @return CXLMFERA*
 * @retval Pointer to a dynamically allocated driver instance created by copy construction
 *         from *this
 */
CReadoutHardware*
CXLMFERA::clone() const
{
  return new CXLMFERA(*this);
}

//////////////////////////////////////////////////////////////////////////////////////////


/*
 * This section of code provides the package initialization code.
 * The Tcl interpreter will call it when the package is loaded.
 * You need to make the driver known to the framework and indicate that you were
 * succesful doing so.  Naturally you can do whatever other initialization you might
 * require here.
 * There are a set of requirements that must be met if Tcl is to locate your package:
 *
 *  The name of the initialization function is derived from the library name you build 
 * (see the Makefile that came with this kit).  If you created libtemplatedriver.so 
 * the name of the initialization function must be Templatedriver_Init
 * The name of the Tcl package you must provide is Templatedriver in that case.
 *
 * The initialization function must also specify C language call methods
 *
 * @param pInterp - Pointer to the raw Tcl Interpreter that is loading this object.
 *
 * @return int 
 * @retval TCL_OK - success.
 * @retval TCL_ERROR - failure.
 */
extern "C" {
  /*
   * You should modify the name below to match you Makefile
   * MODIFY ME HERE */
  int Xlmfera_Init(Tcl_Interp* pInterp)
  /* END MODIFICATIONS */
  {

    // Create a Tcl package so that pkg_mkIndex could find us:

    /* Change the name (Tempatedriver) to match the initialization function.
     * as you evolve the driver you may also want to modify the package version (1.0)
     * MODFIY ME HERE */
    Tcl_PkgProvide(pInterp, "XLMFERA", "1.0");

    /* Register the driver command.  In the line below change the first parameter
     * to the correct Tcl command you want the driver bound to:
     *
     * MODIFY ME HERE */

    CUserCommand::addDriver("XLMFERA", new CXLMFERA);

    /* END MODIFICATIONS */
    return TCL_OK;
    
  }
}

CXLMFERA::CXLMBusController::CXLMBusController(CVMUSB& controller, 
                            CXLMFERA& xlm, 
                            uint32_t request, 
                            uint32_t busInhibit, 
                            uint8_t nDelayCycles)// throw (std::string)
: m_controller(controller),
    m_interfaceAddr(xlm.Interface()),
    m_request(request)
{
    CVMUSBReadoutList list;

    if (busInhibit!=0) { // inhibit FPGA and DSP as well...
        list.addWrite32(m_interfaceAddr+ForceOffBus,registerAmod,busInhibit);

#ifdef PRINTBUSLOCK
        std::cout << "### XLM FPGA and DSP inhibit requested"; 
        std::cout << std::endl;
#endif
    }

    list.addWrite32(m_interfaceAddr, registerAmod, m_request);


    if (nDelayCycles>0) {
        list.addDelay(nDelayCycles);
    } 

    // execute the commands
    size_t nbytes = 0;
    uint32_t data=0;
    int status = controller.executeList(list,(void*)&data,sizeof(data),&nbytes);

    if (status<0) {
        std::cerr << "CXLMBusController::CXLMBusController(CVMUSB&,uint32_t,uint32_t,uint32_t,uint8_t) ";
        std::cerr << "readout list failed to execute with error " << status;
        std::cerr << std::endl;
    }
//    if (!successfulArbitration()) {
//        // note that this will not invoke this object's dtor 
//        // b/c we never finished the ctor
//        
//        // Therefore, ensure that we clean up
//        releaseBusses();
//
//        std::string msg("CXLMBusController::CXLMBusController(CVMUB&,CXLM&,uint32_t)");
//        msg += " failed to satisfy request";
//        throw msg;
//    } 

#ifdef PRINTBUSLOCK
    std::cout << "### XLM VMEBus successfully accquired : ";
    if ((m_request&0x000001)!=0) std::cout << "BUS_A ";
    if ((m_request&0x000002)!=0) std::cout << "BUS_B ";
    if ((m_request&0x010000)!=0) std::cout << "BUS_X ";
    std::cout << std::endl;
#endif
    //at this point VMEbus owns the desired bus(ses)

}

/**! Unconditionally release the bus(ses)
 */
CXLMFERA::CXLMBusController::~CXLMBusController() 
{
    releaseBusses();

}

void CXLMFERA::CXLMBusController::releaseBusses()
{

    uint32_t release = 0;
    m_controller.vmeWrite32(m_interfaceAddr,registerAmod,release);

    // unconditionally remove inhibit
    m_controller.vmeWrite32(m_interfaceAddr+ForceOffBus,registerAmod,release);

#ifdef PRINTBUSLOCK
    std::cout << "### XLM VMEBus released               : ";
    if ((m_request&0x000001)!=0) std::cout << "BUS_A ";
    if ((m_request&0x000002)!=0) std::cout << "BUS_B ";
    if ((m_request&0x010000)!=0) std::cout << "BUS_X ";
    std::cout << std::endl;
    std::cout << "### Inhibit removed from XLM FPGA and DSP" << std::endl; 
#endif

}

bool CXLMFERA::CXLMBusController::successfulArbitration()
{

    uint32_t busAOwnerAddr = (m_interfaceAddr | 0x10000);
    uint32_t busBOwnerAddr = (m_interfaceAddr | 0x10004);
    uint32_t busXOwnerAddr = (m_interfaceAddr | 0x10008);

    // Figure  out what busses were requested by the VMEbus       
    bool busREQ_A = false;
    bool busREQ_B = false;
    bool busREQ_X = false;
    if ((m_request&0x000001)!=0) busREQ_A=true; 
    if ((m_request&0x000002)!=0) busREQ_B=true; 
    if ((m_request&0x010000)!=0) busREQ_X=true; 

    //Block until the VMEbus has acquired the requested busses
    uint32_t data=0;   
    unsigned int attempt = 0;
    unsigned int maxPolls = 100;
    bool busAArbitrated= !busREQ_A;
    bool busBArbitrated= !busREQ_B;
    bool busXArbitrated= !busREQ_X;
    bool arbitrationIncomplete=true;
    while ( arbitrationIncomplete && attempt<maxPolls ) {
        // verify VMEbus has ownership of specified buses.

        if (busREQ_A) {
            m_controller.vmeRead32(busAOwnerAddr,
                    registerAmod,
                    &data);
            if (data==1) busAArbitrated=true;
        }
        if (busREQ_B){
            m_controller.vmeRead32(busBOwnerAddr,
                    registerAmod,
                    &data);
            if (data==1) busBArbitrated=true;
        }
        if (busREQ_X){
            m_controller.vmeRead32(busXOwnerAddr,
                    registerAmod,
                    &data);
            if (data==1) busXArbitrated=true;
        }
        arbitrationIncomplete = ! (busAArbitrated 
                && busBArbitrated 
                && busXArbitrated);
        ++attempt; 
    }

    // if attempt == maxPolls, then arbitration failed
    return (attempt != maxPolls);

}
