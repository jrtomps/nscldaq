#include <config.h>
#ifndef __XLM_H
#define __XLM_H

using namespace std;
#include <sys/types.h>

#include <string.h>
#include "AVmeModule.h"
//  #include "/usr/src/install/llnlReadout-3.4/devices/CReadoutHardware.h"
#include <CReadoutModule.h>
#include <CVMUSBRemote.h>
#include <CVMUSBReadoutList.h>

// forward class defs
//class CReadoutModule;
//class CVMUSB;
//class CVMUSBReadoutList;
                               
// class XLM  : public CReadoutHardware
class XLM  : public AVmeModule
{
protected:
  // AVmeModule* m_pConfiguration;
	long	config;
	char    *fileName;
	volatile long	*vme,*srama,*sramb;
	volatile long    *ownreg;
	volatile long    *fpga,*dsp,*mail; 
	volatile long    *clearmail, *dspmail;
	long	hpc, hpa, hpd;
	int numReads;
	int numWrites;
	int numDelays;
	CVMUSBRemote* pUSBController;
 
public:
	// Constructors, destructors and other cannonical operations: 
	XLM(string name, unsigned int crate, unsigned int slot);
	XLM(const XLM& rhs); //!< Copy constructor.
	~XLM ( );
  
  XLM& operator= (const XLM& rhs); //!< Assignment
  int         operator==(const XLM& rhs) const; //!< Comparison for equality.
  int         operator!=(const XLM& rhs) const {return !(operator==(rhs));}
  
  // Selectors for class attributes:
public:
	long getConfiguration() {return config;}
	char* getFileName() {return fileName;}
  
  // Class operations:
public:
	void loadFirmware(std::string path) throw(std::string);
	void Initialize(string name, unsigned int crate, unsigned int slot);
	void dummy();
	void AccessBus(long busses);
	void ReleaseBus();
	void BootFPGA();
	void SetFPGABoot(long source);
	void CheckConfiguration(long config);
	unsigned long CheckFPGAMail();
	void ClearFPGAMail();
	void ResetDSP();
	void BootDSP();
	long ReadHPC();
	void WriteHPC(long value);
	long ReadHPA();
	void WriteHPA(long value);
	long ReadHPD();
	void WriteHPD(long value);
	unsigned long CheckDSPMail();
	void ClearDSPMail();
	void SendDSPMail(int mail);
	void WaitForDSPFlag();
	void LoadDSP(char dspfile[80]);
	/*  SRAM A,B Read added by M. Famiano */
	long ReadSRAMA(int address);
	long ReadSRAMB(int address);
	long ReadFPGA(int address);
	void WriteFPGA(int address, long value);
	void WriteSRAMA(int address, long value);
	void WriteSRAMB(int address, long value);
	void initList();
	void clearList();
	void addWrite32(int address, long value);
	void addRead32(int address);
	void addDelay(long value);
	int executeList(long* response, unsigned int length, int* value);
	int GetOwner(char bus);
	void loadFile(std::string path, void* contents, uint32_t size) throw(std::string);
	uint32_t fileSize(string path) throw(std::string);
	void remapBits(void* sramImage, void* fileImage, uint32_t bytes);
	void loadSRAMA(CVMUSBRemote& controller, void* image, uint32_t bytes) throw(std::string);
	/*************************************/
	protected:
	uint32_t sramA();             // Return base address of SRAM A
	uint32_t sramB();             // Return base address of SRAM B
  //  uint32_t FPGA();              // Return base address of FPGA 'registers'.
};

#endif
