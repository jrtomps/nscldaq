// this is now a dummy class for the XLM just to permit the GUI to run
#include <config.h>
#include "XLM.h"
#include <errno.h>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdlib.h>
#include "constants.h"              // reg defs etc. for XLM
#include <fcntl.h>
//#include "param.h"

#define XLM_CONFIG_LENGTH 42000


const static int BUSRETRYTIME(5); // usec delay between bus access retry.

uint32_t ID;
static CVMUSBReadoutList VMEList;
static uint32_t XLM_Base;    // VME base address of XLM


//////////////////////////////////////////////////////////////////////////////
/////////////////////  Class level constants  ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//  These constants define bits in the XLM bus request/ownership register:

const uint32_t REQ_A(0x00000001);
const uint32_t REQ_B(0x00000002);
const uint32_t REQ_X(0x00010000);


// The XLM memory layout relative to the base address:

// Address modifiers we are going to use:

static const uint8_t   registerAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   sramaAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   blockTransferAmod(CVMUSBReadoutList::a32UserBlock); 

//  Base addresses of 'unstructured' areas.

//static const uint32_t  SRAMA(0x000000); //  Base address of static ram A
//static const uint32_t  SRAMB(0x200000); //  Base address of static RAM B
static const uint32_t  FPGABase (0x400000); //  Base address of FPGA `register' set.
static const uint32_t  DSP  (0x600000); //  Base address of DSP interface.
static const uint32_t  InterfaceBase(0x800000);

// Interface layout:

static const uint32_t  BusRequest (0x800000); // Register for submitting bus requests.
static const uint32_t  Interrupt  (0x800004); // Interrupt/reset register.
static const uint32_t  FPGABootSrc(0x800008); // Select boot source for FPGA.
static const uint32_t  ForceOffBus(0x80000c); // Register to force FPGA/DSP off bus.
static const uint32_t  BUSAOwner  (0x810000); // Shows who has bus A (SRAM A)
static const uint32_t  BUSBOwner  (0x810004); // Shows who has bus B (SRAM B)
static const uint32_t  BUSXOwner  (0x810008); // Shows who has bus X (FPGA).
static const uint32_t  IRQSerial  (0x820048); // Write for IRQ id reads serial number.
static const uint32_t  POLLISR    (0x820824); // 'mailbox' betweenFPGA and DSP.

// Note that the REQ_A/B/X definitions above define the bits in the bus request register
// therefore we don't define these bits here.

// Bits in Interrupt register; Note these are 'negative logic bits' in that to assert
// the reset, the bit shown below should be set to zero.

static const uint32_t InterruptResetFPGA    (0x00000001);
static const uint32_t InterruptResetDSP     (0x00000002);
// Boot source register contents.  This defines where the FPGA loads its
// microcode from.  There are 4 ROM locations as well as the possibility
// to load the microcode in to SRAM A.  This is coded rather than bits.

static const uint32_t BootSrcRom0 (0x00000000); // Load from sector 0 of PROM.
static const uint32_t BootSrcRom1 (0x00000001); // Load from sector 1 of PROM.
static const uint32_t BootSrcRom2 (0x00000002); // Load from sector 2 of PROM.
static const uint32_t BootSrcRom3 (0x00000003); // Load from sector 3 of PROM.
static const uint32_t BootSrcSRAMA(0x00010000); // Load from SRAM A image.

// The ForceOffBus register only has a single bit.... the LSB.  When set, the FPGA
// and DSP are forced off the bus.  When clear they are allowed to arbitrate for the bus.
// XLM Manual 4.3.1.4 states that even when that bit is set the VME bus host must arbitrate
// for the bus via the BusRequest register.

static const uint32_t ForceOffBusForce(0x00000001); // Force all but VME off bus.

// BUS*Owner values.  These describe who is actually the owner of the specific bus.

static const uint32_t BusOwnerNone(0);  // Bus not owned.
static const uint32_t BusOwnerVME (1);  // VME host owns bus.
static const uint32_t BusOwnerFPGA(2);  // Bus owned by FPGA.
static const uint32_t BusOwnerDSP (3);  // Bus owned by DSP.

// Constructors
/*!
XLM object constructor. Derives from CVMUSB Class and maps unsigned longs
onto the address space of the VME Module class.
*/

XLM::XLM (string name, unsigned int crate, unsigned int slot) :
  AVmeModule(name, crate, slot)
{
  pUSBController = new CVMUSBRemote();

  printf("XLM object created\n");
}

void
XLM::Initialize (string name, unsigned int crate, unsigned int slot)
{
  ID = pUSBController->readFirmwareID();
  printf("FirmwareID is %x\n",ID);
  XLM_Base = slot << 27;
  printf("XLM_Base = %x\n",XLM_Base);
	fileName  = new char[120];
	//	srama     = Map(0x000000, 0x2A000);
	//	sramb     = Map(0x200000, 0x2A000);
	//	fpga      = Map(0x400000, 0x20);
}

/*!
Destructor.
*/
XLM::~XLM()
{
  //	delete [] fileName;
}

/*
XLM::XLM(const XLM& rhs) :
  CVMUSB(rhs)
{
}
*/

XLM& XLM::operator= (const XLM& aXLM)
{
  if (this != &aXLM) {

    //    AVmeModule::operator= (aXLM);
  }
  return *this;
}

int
XLM::operator==(const XLM& rhs) const
{
  return (operator==(rhs));
}

// Functions for class XLM
// FPGA functions

/*!
Configuration function.  This function opens the configuration file and writes each
byte to a space in the ASRAM A.  The boot source is also set to ASRAM A, and the 
FPGA is boot from this.  All Busses are realeased at the end.
Configuration net files are generated by the user.   

\param configfile - Character pointer to the configuration file.

CAUTION:  The only check done by this function is the length of the net file.  In no way
does it verify the functionality, usefulness, correctness, or safety of the
resulting configuration.
*/
void
XLM::loadFirmware(string path) throw(std::string)
{
  //  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
  uint32_t base = XLM_Base;
  cerr << hex << "Loading firmware for XLM at " << base << endl << dec;

  // Prep the FPGA for loading.  Specifically:
  // 1. Set the load source to SRAMA
  // 2. Force a falling edge on the FGPA Reset.. while not modifying the state of the other
  //    components....and hold the FPGA reset.
  // 3. Force the FPGA off the bus.
  // 4. Request the bus to SRAM A.
  // With the exception of determining the state of the Interrupt/reset register so it can be manipulated,
  // these operations are done in an immediate list:

  // Get the state of the interrupt register.. well it's another crappy write only reigster so
  // let's assume everything is held reset and that we only want to start the fpga.

  uint32_t interruptRegister = 0; /* InterruptResetFPGA | InterruptResetDSP |
				     InterruptInterruptFPGA | InterruptInterruptDSP */

  // Build the list of operations:
  // Done in a block so the list is destroyed after it's executed:

  {
    CVMUSBReadoutList initList;
    initList.addWrite32(XLM_Base + ForceOffBus, registerAmod, ForceOffBusForce); // Inhibit FPGA Bus access.
    initList.addWrite32(XLM_Base + Interrupt,   registerAmod,  InterruptResetFPGA); // Hold FPGA reset.
    initList.addWrite32(XLM_Base + FPGABootSrc, registerAmod, BootSrcSRAMA); // Set boot source
    initList.addWrite32(XLM_Base + AccBus, registerAmod, REQ_A);                        //  Request bus A.

    
    // run the list:

    size_t dummy;		// For read buffer.
    size_t readSize;
    int status = pUSBController->executeList(initList,
					&dummy, sizeof(dummy), &readSize);
    if (status != 0) {
      string reason = strerror(errno);
      string msg = "CXLM::loadFirmware - failed to execute initialization list: ";
      msg       += reason;

      throw msg;
    }

    // I should have bus a:
    uint32_t owner =0;
    pUSBController->vmeRead32(XLM_Base + BUSAOwner, registerAmod, &owner);
    cerr << "BUSA Owner is: " << owner << endl;

    // Open and read the entire fpga file into memory (can't be too large)



    uint32_t  bytesInFile = fileSize(path);
    uint8_t*  contents    = new uint8_t[bytesInFile];
    uint32_t* sramAImage  = new uint32_t[bytesInFile]; // Each byte becomes an SRAM Longword.
    printf("bitfile is %d bytes long.\n",bytesInFile);
    memset(sramAImage, 0, bytesInFile * sizeof(uint32_t));

    // The remainder is in a try block so we can delete the file contents:

    try {
      // Read the file, convert it to an sram a image and load it into SRAM A:

      loadFile(path, contents, bytesInFile);	// Read the file into memory.

      // Skip the header:

      uint8_t* pc = contents;
      while (*pc != 0xff) {
	pc++;
	bytesInFile--;
      }

      remapBits(sramAImage, pc, bytesInFile);
      loadSRAMA(*pUSBController, sramAImage, bytesInFile*sizeof(uint32_t));
      
      // Release the SRAMA Bus, 
      // release the 'force'.
      // Remove the reset from the FPGA:

      cerr << "Firmware loaded in SRAMA\n";

      CVMUSBReadoutList  finalize;
      finalize.addWrite32(XLM_Base + BusRequest, registerAmod, (uint32_t)0);	// Release bus request.
      finalize.addWrite32(XLM_Base + ForceOffBus, registerAmod, (uint32_t)0); // Remove force
      finalize.addWrite32(XLM_Base + Interrupt, registerAmod, (uint32_t)0);	// Release FPGA reset
      status = pUSBController->executeList(finalize,
				       &dummy, sizeof(dummy), &readSize);
				       
      if (status != 0) {
	string reason = strerror(errno);
	string message = "CXLM::loadFirmware failed to execute finalization list: ";
	message       += reason;
	throw message;
      }
      printf("bitfile should now be loaded.\n");
      sleep(2);                                                         // wait for FPGA to load config
    }
    catch (...) {
      delete []contents;
      delete []sramAImage;
      throw;			// Let some higher creature deal with this.
    }
    delete []contents;
    delete []sramAImage;
    cerr << "FPGA Should now be started\n";
  }
}

/*!  
Checks the configuration of the loaded net file by accessing ASRAMA and invoking the CheckConfiguration
member function.  This is only used if the user has loaded a predefined word into the lowest address register
of ASRAM A.
*/
void
XLM::dummy()
{
  AccessBus(0x10000);
  //	CheckConfiguration(0);
  ReleaseBus();
}

/*!
Opens the bus for VME access.  Before opening, checks to see if the busses are available.

\param busses - A logical OR of the busses to be accessed:
 - 0x00001: ASRAM A.
 - 0x00002: AXRAM B.
 - 0x10000: Bus X for FPGA access.
 - 0x20000: Bus B for DSP access.
*/
void
XLM::AccessBus(long busses)
{
	int attempts = 20;
	uint32_t check = 0;
	//	printf("AccessBus\n");
	check = busses;     // hack to make always work
	for(int i =0; i < attempts; i++) {
	  pUSBController->vmeWrite32(XLM_Base+0x800000,0x09,busses);
	  //		vme[0] = busses;
	  pUSBController->vmeRead32(XLM_Base+0x800000,0x09,&check);
	  //		check = vme[0]&0x00030003;
	  //	  printf("AccessBus returns %x\n",check);
		attempts--;
		if(check == busses) {
		  break;
		}
		else {
		  usleep(BUSRETRYTIME);
		}
		
	} 
	if (check != busses) {
	  //	  cout << "Failed to grab busses in " << getName() << " (slot " << getSlot() << " )" << endl;
	  exit(0);
	}
}

/*!
Releases busses from VME Access.  
*/
void
XLM::ReleaseBus()
{
  pUSBController->vmeWrite32(XLM_Base+0x800000,0x09,0);
  //	printf("ReleaseBus\n");
}

/*!
Accesses the FPGA and writes to the reset register, forcing a boot from ASRAMA.  
Pauses for 0.5 seconds to allow the FPGA to read the boot source.
*/
void
XLM::BootFPGA()
{
  //	vme[1] = 1;
	usleep(10);
	//	vme[1] = 0;
	cout << "Booting FPGA ..." << endl;
	//	usleep(500000);		// Leave some time for FPGA to boot!
}

/*!
Writes to the FPGA boot source register, setting the boot source.
\param source - Long corresponding to the desired boot source.
 - 0x00000: Sector 0 of Flash Memory.
 - 0x00001: Sector 1 of Flash Memory.
 - 0x00002: Sector 2 of Flash Memory.
 - 0x00003: Sector 3 of Flash Memory.
 - 0x10000: ASRAM A.
*/
void 
XLM::SetFPGABoot(long source)
{
  //	vme[2] = source;
  //	long check = vme[2];
  //	if (check != source) {
  //		cout << "Failed to set boot source in " << getName() << " (slot " << getSlot() << " )" << endl;
  //		exit(1);
  //	}
}

/*!
Checks address 0x00000 of the FPGA Bus for a "signature" word written during configuration.
\param config - The signature word developed in the FPGA net file, configured by the user.
*/
void
XLM::CheckConfiguration(long config)
{
  //	string name = getName();
  //	config = long(Param_getVal(name.c_str(), "configuration"));
	//	AccessBus(0x10001);
	//	long check = fpga[0];
		long check = 0;
	//	ReleaseBus();
		if (check != config) {
		  //			cout << getName() << " XLM72 in slot " << getSlot() << " has wrong FPGA configuration" << endl;
	}
	else {
	  //		cout << getName() << " XLM72 in slot " << getSlot() << " successfully configured FPGA from:" << endl;
		cout << getFileName() << endl;
	}
}

/*!
You must do AcessBus(0x10001) before calling this routine.
 reads the memory locaion indicated.  You should call ReleaseBus() after you are finished reading this memory.
\param address - Integer corresponding to the address location to be read.  Address location is 
equal to four times the integer, since the lowest two address bits are always zero.
*/
long
XLM::ReadSRAMA(int address)
{

  long value;
  //  AccessBus(0x10001);  (moved external for efficiency)
  
  //  value = srama[address];
  //  ReleaseBus();

  return 0;
}
/*! 
Bus Ownership Register
\param bus - Character A,B, or X
Returns:
- 0: Bus is free
- 1: Bus controlled by VMEBus
- 2: Bus Controlled by FPGA
- 3: Bus Contolled by DSP
- 4: Invalid bus selection
*/

int
XLM::GetOwner(char bus)
{
  int value;
  if((bus=='A')||(bus=='a'))
    value=ownreg[0];
  else if((bus=='B')||(bus=='b'))
    value=ownreg[1];
  else if((bus=='X')||(bus=='x'))
    value=ownreg[2];
  else
    value = 4;

  return value;
}

/*! 
Reads ASRAM B.  Same Function as in SRAMA.  
\param address - Same usage as in ReadSRAMA.
*/
long
XLM::ReadSRAMB(int address)
{

  long value;
  //  AccessBus(0x10002);
  
  //  value = sramb[address];
  //  ReleaseBus();

  return 0;
}

/*!
You must do AcessBus(0x10001) before calling this routine.
 You should call ReleaseBus() after you are finished reading this memory.
Reads the FPGA Bus (X).  Same usage and functionality for ReadSRAMA and ReadSRAMB.
\param address - Same usage as ReadSRAMA and ReadSRAMB.
*/
long
XLM::ReadFPGA(int address)
{

  uint32_t value;
  uint32_t retval;
  retval = pUSBController->vmeRead32(XLM_Base+address, 0x09, &value);
  //  printf("ReadFPGA %x\n",XLM_Base+address);
  return value;
}
 
/*!
You must do AcessBus(0x10001) before calling this routine.
  You should call ReleaseBus() after you are finished reading this memory.
Accesses the FPGA bus, and writes to the indicated register.  Releases the bus upon completion.
\param address - FPGA Bus address location to be written to.  Note that the actual address is
four times the parameter, since the lowest two bits are always zero.
\param value - Value to be written to the address location.
*/
void
XLM::WriteFPGA(int address,long value)
{
  uint32_t retval;
  retval = pUSBController->vmeWrite32(XLM_Base+address, 0x09, value);
  //  printf("WriteFPGA addr %x value %lx\n",XLM_Base+address,value);
}

/*!
You must do AcessBus(0x10001) before calling this routine.
  You should call ReleaseBus() after you are finished reading this memory.Accesses the ASRAM A, and writes to the indicated register.  Releases the bus upon completion.
\param address - ASRAM A address location to be written to.  Note that the actual address is
four times the parameter, since the lowest two bits are always zero.
\param value - Value to be written to the address location.
*/
void
XLM::WriteSRAMA(int address,long value)
{
  //  AccessBus(0x10001);
  //  srama[address]=value;
  //  ReleaseBus();
}

/*!
You must do AcessBus(0x10001) before calling this routine.
  You should call ReleaseBus() after you are finished reading this memory.Accesses the ASRAM B, and writes to the indicated register.  Releases the bus upon completion.
\param address - ASRAM B address location to be written to.  Note that the actual address is
four times the parameter, since the lowest two bits are always zero.
\param value - Value to be written to the address location.
*/
void
XLM::WriteSRAMB(int address,long value)
{
  //  AccessBus(0x10002);
  //  sramb[address]=value;
  //  ReleaseBus();
}
/***** list operations *****/
void
XLM::initList()
{
  //  printf("initList\n");
  // CVMUSBReadoutList VMEList;
  VMEList.clear();
  numReads = 0;
  numWrites = 0;
  numDelays = 0;
}

void
XLM::clearList()
{
  //  printf("clearList\n");
  VMEList.clear();
  numReads = 0;
  numWrites = 0;
  numDelays = 0;
}

void
XLM::addWrite32(int address, long value)
{
  //  printf("addWrite32 addr %x value %lx\n",XLM_Base+address,value);
  VMEList.addWrite32((uint32_t)XLM_Base+address, (uint8_t)0x09, (uint32_t)value);
  numWrites++;
}

void
XLM::addRead32(int address)
{
  //  printf("addRead32 addr %x\n",address);
  VMEList.addRead32(XLM_Base+address, (uint8_t)0x09);
  numReads++;
}

void
XLM::addDelay(long value)
{
  //  printf("addDelay %lx\n",value);
  VMEList.addDelay((uint8_t)value);
  numDelays++;
}

int
XLM::executeList(long* response, unsigned int length, int* bytesRead)
{
  size_t value;
  int i,retval;
  long* resp;
  retval = pUSBController->executeList(VMEList, response, length, &value);
  *bytesRead =int(value);
  //  printf("execute list resp length %d\n",length);
  //  printf("retval %d, bytesRead %d\n",retval,value);
  //  printf("response buf at %lx\n",response);
  //  for (i=0;i<32;i++) {
  //    resp = response+long(i*4);
  //    printf("%d %lx\n",i,*resp);
  //  }
  //  printf("%d Write32s\n",numWrites);
  //  printf("%d Read32s\n",numReads);
  //  printf("%d Delays\n",numDelays);
}

/***************************************/

unsigned long
XLM::CheckFPGAMail()
{
	int attempts = 100;
	unsigned long check = 0;
	do {
		check = mail[0]&0xFFFF;
		attempts--;
	} while (check == 0 && attempts >= 0);
	if (check == 0) {
	  //		cout << "VME failed to detect mail from FPGA in " << getName() << " (slot " << getSlot() << " )" << endl;
	}
	ClearFPGAMail();
	return check;
}

void
XLM::ClearFPGAMail()
{
	clearmail[0] = 0x824;
}

// DSP Functions

void
XLM::ResetDSP()
{
	vme[1] = 2;
	vme[1] = 0;
	usleep(500000);		// Leave some time for DSP to reset!
}

void
XLM::BootDSP()
{
	ReadHPC();
	WriteHPC(hpc|0x00020002);
	cout << "Booting DSP ..." << endl;
	usleep(100000);		// Leave some time for DSP to boot!
}
	
long
XLM::ReadHPC()
{
	unsigned long low, high;
	high = dsp[0x100];
	// do it twice because of HPI bug
	high = dsp[0x100];
	low = dsp[0x101];
	hpc = low | (high<<16);
	return hpc;
}

void
XLM::WriteHPC(long value)
{
	unsigned long low, high;
	hpc = value;
	high = hpc>>16;
	low = hpc&0xFFFF;
	dsp[0] = high;
	dsp[1] = low;
}

long
XLM::ReadHPA()
{
	unsigned long low, high;
	high = dsp[0x102];
	// do it twice because of HPI bug
	high = dsp[0x102];
	low = dsp[0x103];
	hpa = low | (high<<16);
	return hpa;
}

void
XLM::WriteHPA(long value)
{
	unsigned long low, high;
	hpa = value;
	high = hpa>>16;
	low = hpa&0xFFFF;
	dsp[2] = high;
	dsp[3] = low;
}

long
XLM::ReadHPD()
{
	unsigned long low, high;
	high = dsp[0x106];
	// do it twice because of HPI bug
	high = dsp[0x106];
	low = dsp[0x107];
	hpd = low | (high<<16);
	return hpd;
}

void
XLM::WriteHPD(long value)
{
	unsigned long low, high;
	hpd = value;
	high = hpd>>16;
	low = hpd&0xFFFF;
	dsp[6] = high;
	dsp[7] = low;
}

unsigned long
XLM::CheckDSPMail()
{
	int attempts = 10000;
	unsigned long check = 0;
	do {
		check = (mail[0]&0xFFFF0000)>>16;
		attempts--;
	} while (check == 0 && attempts >= 0);
	if (check == 0) {
	  //		cout << "VME failed to detect mail from DSP in " << getName() << " (slot " << getSlot() << " )" << endl;
//	} else {
//		cout << "Mail obtained from DSP after " << 10000-attempts << " VME cycles" << endl;
	}
	ClearDSPMail();
	return check;
}

void
XLM::WaitForDSPFlag()
{
	int attempts =  10000;
	unsigned long check = 0x04000000;
	do {
		check = vme[0]&0x04000000;
		attempts--;
	} while (check == 0x04000000 && attempts >= 0);
	if (check == 0x04000000) {
	  //		cout << "DSP did not clear its flag in " << getName() << " (slot " << getSlot() << " )" << endl;
	} else {
		cout << "DSP flag cleared after " << 10000-attempts << " VME cycles" << endl;
	}
}

void
XLM::ClearDSPMail()
{
	clearmail[0] = 0x1048;
}

void
XLM::SendDSPMail(int mail)
{
	dspmail[0] = mail;
}

void
XLM::LoadDSP(char dspfile[80])
{
	FILE *file;
	short data, nbytes, bytes[25], length;
	char str[80];
	//	string name = getName();
	//fileName = Param_getStrVal(name.c_str(), "DSPCode");
	file = fopen(dspfile, "r");
	if (!file) {
		cout << "Failed to open DSP code file " << fileName << endl;
		exit(1);
	}
	// skip first data
        char* pStringResult = fgets(str, 80, file);
	// reset HPI address to 0
	hpa = 0;
	// Load code in DSP through HPI
	while (!feof(file)) {
		pStringResult = fgets(str, 80, file);
		length = strlen(str);
		if (length == 9) {
			str[0] = 48;
			str[1] = 120;
			sscanf(str, "%lx,", &hpa);
//			cout << "Found jump to address " << hex << hpa << endl;
		} else {
			nbytes = (strlen(str)-1)/3;
			sscanf(str, "%hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx %hx ",
			&bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5], &bytes[6], &bytes[7], &bytes[8], &bytes[9],
			&bytes[10], &bytes[11], &bytes[12], &bytes[13], &bytes[14], &bytes[15], &bytes[16], &bytes[17], &bytes[18], &bytes[19],
			&bytes[20], &bytes[21], &bytes[22], &bytes[23]);
			for (short i=0; i<nbytes; i+=4) {
				// First byte
				hpd = bytes[i]<<24;
				// second byte
				hpd |= bytes[i+1]<<16;
				// third byte
				hpd |= bytes[i+2]<<8;
				// fourth byte
				hpd |= bytes[i+3];
				WriteHPA(hpa);
				WriteHPD(hpd);
				hpa += 4;
			}
		}
	}
	fclose(file);
	//	cout << getName() << " XLM72 in slot " << getSlot() << " successfully loaded DSP code from:" << endl;
	cout << fileName << endl;
}

/**
 * loads a firmware file into memory.  By this time the file is known to exist and be readable
 * and the size has been determined so:
 * @param path - Absolute or relative path to the file.
 * @param contents - Pointer to a buffer that will hold the file.
 * @param bytes - Number of bytes in the file.
 * @throw std::string on any system call error.
 */
void
XLM::loadFile(std::string path, void* contents, uint32_t size) throw(std::string)
{
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    string error = strerror(errno);
    string msg   = "CXLM::loadFile - Failed to open the file: ";
    msg         += error;
    throw msg;
  }

  // read can be partial... this can happen on signals or  just due to buffering;
  // therefore the loop below is the safe way to use read(2) to load a file.
  // TODO:  We need to make a library of such 'simple little things'... as I'm sure I've used
  //        loops like this in several places in my life.
  // 
  uint8_t* p = reinterpret_cast<uint8_t*>(contents); // qty read on each read(2) call are bytes.
  try {
    while (size > 0) {
      ssize_t bytesRead = read(fd, p, size);
      if (bytesRead < 0) {
        // only throw if errno is not EAGAIN or EINTR.

        int reason = errno;
        if ((reason != EAGAIN) && (reason != EINTR)) {
          string error = strerror(reason);
          string msg   = "CXLM::loadFile - read(2) failed on firmware file: ";
          msg         += error;
          throw msg;
        }
      }
      else {
        size -= bytesRead;
        p    += bytesRead;
      }
    }
  }
  catch (...) {
    // Close the file... and rethrow.
    
    close(fd);
    throw;
  }
  // close the file

  close(fd);                    // should _NEVER_ fail.
}
/**
 * Returns the size of a file.  The file must already exist and  be a valid target for 
 * stat(2). As this is used in the firmware load process, this has typically been assured by
 * a call to validFirmwareFile.
 * @param path - Absolute or relative path to the firmware file.
 * @return size_t
 * @retval Number of bytes in the file.  This includes 'holes' if the file is spares.
 * @throw std::string - if stat fails.
 */
uint32_t
XLM::fileSize(string path) throw(std::string)
{
  struct stat fileInfo;
  int status = stat(path.c_str(), &fileInfo);
  if(status) {
    string msg = strerror(errno);
    string error = "CXLM::fileSize - Unable to stat firmware(?) file: ";
    error       += msg;
    throw error;
  }
  return static_cast<uint32_t>(fileInfo.st_size); // Limited to 4Gbyte firmware files should be ok ;-]
}

/**
 * Remap the bits of a firmware file into an SRAM image.  Each byte maps to some scattered set of
 * bits in an SRAM longword.  The mapping is defined by the table bitMap below.
 * @param sramImage - Buffer to hold the sramImage.
 * @param fileImage - Buffer holding the raw file contents.
 * @param bytes     - Number of bytes in fileImage.  It is up to the caller to ensure that the
 *                    size of sramImage is at least bytes*sizeof(uint32_t) big.
 */
void
XLM::remapBits(void* sramImage, void* fileImage, uint32_t bytes)
{
  static const uint32_t bitMap[]  = {
    0x4, 0x8, 0x10, 0x400, 0x40000, 0x80000, 0x100000, 0x4000000
  };

  uint8_t*   src  = reinterpret_cast<uint8_t*>(fileImage);
  uint32_t*  dest = reinterpret_cast<uint32_t*>(sramImage);

  for(uint32_t i =0; i < bytes; i++) {
    uint32_t  lword = 0;                // build destination here.
    uint8_t   byte  = *src++;   // Source byte
    uint32_t  bit   = 1;
    for (int b = 0; b < 8; b++) { // remap the bits for a byte -> longword
      if (byte & bit) {
        lword |= bitMap[b];
      }
      bit = bit << 1;
    }
    *dest++ = lword;            // set a destination longword.
  }

}
/**
 * given a block of bytes, loads it into SRAMA.
 * - This is done 256bytes at a time.
 * - This block transfers are done.
 * - For now we don't trust Jan to do block writes larger than that quite correctly.
 * - We _do_ build a list of block writes and execute them all at once.
 * - Assumption: the image is an even multiple of uint32_t.
 *
 * @param controller - CVMUSB controller object reference.
 * @param image      - Bytes to load in the SRAMA
 * @param bytes      - sizeof the image in bytes.
 * @throw std::string - in the event of an error executing a VM-USB list.
 */
void
XLM::loadSRAMA(CVMUSBRemote& controller, void* image, uint32_t bytes) throw(std::string)
{
  static const size_t   blockSize = 256;
  static const size_t   vblockSize = blockSize;
  uint32_t              nBytes    = bytes;

  // for now load it one byte at a time... in 256 tansfer chunks:

  

  uint32_t*           p    = reinterpret_cast<uint32_t*>(image);
  static uint32_t    dest = sramA();

  cerr << hex << "LOADSRAMA - SRAM A base addresss is " << dest << endl << dec;

  if (bytes == 0) return;       // Stupid edge case but we'll handle it correctly.

  while (bytes > blockSize*sizeof(uint32_t)) {
    CVMUSBReadoutList  loadList;
    for (int i =0; i < blockSize; i++) {
      loadList.addWrite32(dest, sramaAmod,  *p++);
      dest += sizeof(uint32_t);
    }
    bytes -= blockSize*sizeof(uint32_t);
    // Write the block:

    uint32_t data;
    size_t   dsize;
    int status = controller.executeList(loadList,
                                        &data,
                                        sizeof(data), &dsize);
    if (status < 0) {
      string error = strerror(errno);
      string msg   = "CXM::loadSRAMA - list execution failed to load the SRAM: ";
      msg         += error;
      throw msg;
    }

  }
  // Handle any odd partial block:

  if (bytes > 0) {
    CVMUSBReadoutList loadList;
    while (bytes > 0) {
      loadList.addWrite32(dest, sramaAmod, *p++);
      dest += sizeof(uint32_t);
      bytes -= sizeof(uint32_t);
    }
    // Write the block:

    size_t readData;
    size_t readDataSize;
    int status = controller. executeList(loadList, &readData, 
					 sizeof(size_t), &readDataSize);
    if (status < 0) {
      string error = strerror(errno);
      string msg   = "CXM::loadSRAMA - list execution failed to load the SRAM: ";
      msg         += error;
      throw msg;
    }
  }

  return;
  /// Some tests for Jan.

  uint32_t* compareData = new uint32_t[blockSize];
  uint32_t src         = sramA();
  uint32_t   bytesRead;
  size_t   bytesLeft   = nBytes;

  while(bytesLeft > blockSize * sizeof(uint32_t)) {
    CVMUSBReadoutList verifyList;
    verifyList.addBlockRead32(src, blockTransferAmod,
                              blockSize);
    verifyList.dump(cerr);
    int stat = controller.executeList(verifyList,
                                      compareData,
                                      blockSize*sizeof(uint32_t),
                                      &bytesRead);
    string msg = strerror(errno);
    cerr << "Status " << stat 
         << " Read Size: " << blockSize*sizeof(uint32_t)
         << "Actual read " << bytesRead << endl;
    if (stat != 0) {
      cerr << "Failure reason: " << msg << endl;
    }

    bytesLeft -= blockSize*sizeof(uint32_t);
    src       += blockSize*sizeof(uint32_t);
  }
  

    // Verify the load:

  return;                       // For now for speed.

  cerr << "Verifying SRAMA contents\n";
  p        = reinterpret_cast<uint32_t*>(image);
  dest     = sramA();
  uint32_t* pSram = new uint32_t[vblockSize];
  size_t    xfered = 0;

  while (nBytes > vblockSize * sizeof(uint32_t)) {
    CVMUSBReadoutList vlist;
    uint32_t  daddr = dest;
    uint32_t* pr = pSram;
    for (int i =0; i < vblockSize; i++) {
      controller.vmeRead32(daddr,sramaAmod, pr);
      daddr += sizeof(uint32_t);
      pr++;
    }
    pr = pSram;

    for (int i =0; i < vblockSize; i++) {
      if (*p != *pr) {
        cerr << hex << "Mismatch at address " << dest << ": SB: " << *p << " Was: " << *pr  << endl <<dec;
      }

      p++;
      pr++;
      dest += sizeof(uint32_t);
    }

    nBytes -= vblockSize*sizeof(uint32_t);
    cout << '.';
    cout.flush();

  }
  cout << '\n';
  delete []pSram;
  cerr << "Verification complete\n";
}
/*! 
  Convenience function to return the base address of SRAM A
  @return uint32_t
  @retval -base + SRAMA

*/
uint32_t
XLM::sramA()
{
  //  return m_pConfiguration->getUnsignedParameter("-base") + SRAMA;
  return XLM_Base + SRAMA;
}
/*!
  Convenience function to return the base address of SRAM B
  @return uint32_t
  @retval -base + SRAM
*/
uint32_t 
XLM::sramB()
{
  //  return m_pConfiguration->getUnsignedParameter("-base") + SRAMB;
  return XLM_Base + SRAMB;
}
/*!
  Convenience function to return the base address of the FPGA register set.  These are the 
  registers that are maintained by the FPGA firmware, not the interface registers.
  @return uint32_t
  @retval -base + FPGA
*/
/* 
uint32_t
XLM::FPGA()
{
  uint32_t b =    m_pConfiguration->getUnsignedParameter("-base");
  return b + FPGABase;
}
*/
