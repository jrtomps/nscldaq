#ifndef __SIS3300_H
#define __SIS3300_H

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#define __SPECTRODAQ_H
#endif


#ifndef __STL_STRING
#include <string>
#define __STL_STRING
#endif

/*!
   Preliminary class to setup and read the SIS3300 flash adc.
   Supported functions are to:

   Initialize the module to work in external trigger:
   Start or stop mode, single event mode
   with specified Clock source and sample size, and pre/post trigger delays.

   This class does not support  ADC thresholds, or
   multievent mode  but they can be added later.
 
   */
class CSIS3300 {
public:
  typedef enum _ClockSource {
    Internal100Mhz = 0,
    Internal50Mhz  = 1,
    Internal25Mhz  = 2,
    Internal12_5Mhz= 3,
    Internal6_25Mhz= 4,
    Internal3_125Mhz=5,
    ExternalFp      =6,
    ExternalP2      =7 
  } ClockSource;		//!< the enum values match the bit field values
  typedef enum _SampleSize {
    Sample128K     = 0,
    Sample16K      = 1,
    Sample4K       = 2,
    Sample2K       = 3,
    Sample1K       = 4,
    Sample512      = 5,
    Sample256      = 6,
    Sample128      = 7
  } SampleSize; 

private:
  unsigned long           m_nBase;            //!< Module abs. VME base
  // Register pointers:
  volatile unsigned long* m_pCsrs;            //!< CSR bank 0 - 0x3000.
  volatile unsigned long* m_pEi1;             //!< Event information for bank1.
  volatile unsigned long* m_pEi2;              //!< Event info for bank 2.
  volatile unsigned long* m_pEi3;             //!< Event info for bank 3.
  volatile unsigned long* m_pEi4;             //!< Event info for bank 4.

  volatile unsigned long* m_pModuleId;	      //!< Module ID register.
  volatile unsigned long* m_pCsr;	      //!< Control status register.  
  volatile unsigned long* m_pAcqReg;	     //!< Acquisition control register.
  volatile unsigned long* m_pResetKey;	      //!< Module reset key register.
  volatile unsigned long* m_pStart;	      //!< Start Key register.
  volatile unsigned long* m_pEventConfig;    //!< Global Event config register.
  volatile unsigned long* m_pStartDelay;	//!< Start delay register.
  volatile unsigned long* m_pStopDelay;	      //!< Stop  delay register.
  volatile unsigned long* m_pAddressReg; //!< Address register (detect eov) 
  volatile unsigned long* m_pEventDirectory;   //!< Ptr to the event directory.
  volatile unsigned long* m_pBank1Buffers[4];   //!< Bank data memory.
  volatile unsigned long* m_pThresholds[4]; //!< threshold registers.

  // State which determines how the module is set up:
 
  void*         m_nFd;		//!< Fd open on the vme device driver.
  ClockSource m_eClock;		    //!< Source of sampling clock.
  bool        m_fStartDelayEnabled; //!< true if start delay register is used.
  unsigned int m_nStartDelayClocks; //!< clocks to load into start delay reg.
  bool        m_fStopDelayEnabled;  //!< true if stop delay register is used.
  unsigned int m_nStopDelayClocks;//!< clocks to load into stop delay register.
  bool       m_fGateMode;	//! true if module in gate mode.
  bool       m_fStopTrigger;    //! True if stop is trigger.
  SampleSize m_ePagesize;	//!< Size of acquisition page (enum).
  unsigned int m_nPagesize;	//!< Size of acquisition page in samples
  bool       m_fPageWrap;	//!< True if allowing data to wrap the pages
  bool       m_fThresholdLt[8];	//!< True if chann  threshold is a < threshold
  unsigned int m_nThresholds[8];	//!< ADC channel threshold values.
public:
  // Constructors and other canonical member functions:

  CSIS3300(unsigned long nBaseAddress,
	   int nCrate = 0);
private:
  CSIS3300(const CSIS3300& rhs);
  CSIS3300& operator=(const CSIS3300& rhs);
  int      operator==(const CSIS3300& rhs);
public:
  ~CSIS3300();

  // Selectors:
public:
  volatile unsigned long* getModuleBase() { 
    return m_pCsrs;
  }
  string getModuleId() const;
  volatile unsigned long* getCsr() {
    return m_pCsr;
  }
  volatile unsigned long* getAcquisitionRegister() {
    return m_pAcqReg;
  }
  volatile unsigned long* getResetKeyRegister() {
    return m_pResetKey;
  }
  volatile unsigned long* getStartKeyRegister() {
    return m_pStart;
  }
  volatile unsigned long* getEventConfigRegister() {
    return m_pEventConfig;
  }
  volatile unsigned long* getStartDelayRegister() {
    return m_pStartDelay;
  }
  volatile unsigned long* getStopDelayRegister() {
    return m_pStopDelay;
  }
  volatile unsigned long* getEventDirectory() {
    return m_pEventDirectory;
  }
  volatile unsigned long* getGroup1Pointer() {
    return m_pBank1Buffers[0];
  }
  volatile unsigned long* getGroup2Pointer() {
    return m_pBank1Buffers[1];
  }
  volatile unsigned long* getGroup3Pointer() {
    return m_pBank1Buffers[2];
  }
  volatile unsigned long* getGroup4Pointer() {
    return m_pBank1Buffers[3];
  }

  // Configuration functions:
 
  void SetClock(ClockSource eSource);
  void SetStartDelay(bool Enable = false, unsigned int nClocks = 0);
  void SetStopDelay(bool Enable  = false, unsigned int nClocks = 0);
  void GateMode(bool Enable = false);
  void TriggerOnStop(bool Enable=true);
  void SetSampleSize(SampleSize eSamples);
  void EnableWrap(bool Enable=true);

  void SetThresholds(bool* pLessThan,
		     unsigned int* pValues);

  // Configuration inquiry functions.

  enum CSIS3300::ClockSource  getCurrentClockSource();
  bool         isStartDelayEnabled();
  unsigned int getStartDelayClocks();
  bool         isStopDelayEnabled();
  unsigned int getStopDelayClocks();
  unsigned int getThresholdValue(unsigned int channel) const {
    return m_nThresholds[channel];
  }
  bool         isLtThreshold(unsigned int channel) const {
    return m_fThresholdLt[channel];
  }


  //  Data taking functions. 
public:
  void InitDaq();
  void StartSampling();
  bool WaitUntilDone(int timeout);
  unsigned int ReadGroup1(DAQWordBufferPtr& pBuffer);
  unsigned int ReadGroup2(DAQWordBufferPtr& pBuffer);
  unsigned int ReadGroup3(DAQWordBufferPtr& pBuffer);
  unsigned int ReadGroup4(DAQWordBufferPtr& pBuffer);
  unsigned int ReadAllGroups(DAQWordBufferPtr& pBuffer);
  void ClearDaq();
protected:
  unsigned int ReadAGroup(DAQWordBufferPtr& pBuffer,
			  volatile unsigned long* pAddressReg,
			  unsigned long pBase);

};



#endif
