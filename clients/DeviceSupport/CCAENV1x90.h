/*!
  \class CCAENV1x90
   The CCAENV1x90 class provides a C++ class based interface
to the CAEN V1190 and V1290 TDC module.  These are TDC modules 
that are based on the CERN TDC chip.   For each module of this sort
you have in your system, you must create an instance of this class.
Note that since CAEN is gravitating away from geographical 
addressing, this initial version will only support base address
access.

This module is a very complex module. Be sure you understand it 
completely (read the hardware manual) before you attempt to make 
of it.  Did we remember to advise you to read the hardware manual?

From the software point of view, there are two major modes of
operation:
- Continuous storage mode: In this mode hits are accepted relative
  to a t=0 time.  The hit times are recorded relative to that time.
- Trigger matching mode: This mode is more like what NSCL users
  are used to seeing.  The module has a gate, and all hits that come
  in within some "matching window" of the gate are accepted.
  The TDC chips are built to run in common stop mode, however there's
  a 1usec delay on the gate that allows a limited common start range.
  Trigger matching mode is further characterized by the matching 
  parameters:
  - Window offset- the time relative to the trigger at which
    the matching begins.  This can be positive or negative.
  - Window Width - Thesize, in time, of the matching window.
  - Reject margin- A usually small window on to the left of the
    actual matching window that allows for the fact that events may
    not have made it into the L1 buffer at the time the window opens.
  - Extra search margin - an extra time on the end of the match
    window to ensure that all hits that occured during the
    search window have been written to the L1 buffer from which
    the match is done.

The 1290 module has an additional mode: High resolution mode.  In that
mode, sets of TDC channels are ganged together with phase shifted 
clocks and recombined to get a time resolution of 25ps.  In this 
mode, the pair trigger mode is not allowed.

Wherever possible, we attempt to enforce all restrictions via 
software.

Change Log:

$Log$
Revision 4.2  2004/11/16 15:24:48  ron-fox
- Port to the gnu 3.x compiler set.
- Integrate buid of tests.
- Integrate build of docos.

Revision 1.4  2004/11/16 15:23:29  ron-fox
- Port -> gcc/g++ 3.x
- Support integrated test building.
- Support integrated doxygen docu7mentation building.

Revision 1.1.2.4  2004/07/06 13:39:08  ron-fox
Fix the experimentally determined error in leading/trailing edge detection.

Revision 1.1.2.3  2004/07/01 12:31:05  ron-fox
Correct edge discrimination enum problems due to error in manual.

Revision 1.1.2.2  2004/06/29 14:50:15  ron-fox
Merge in development line for the CAENV1x90 module.

*/
#ifndef __CCAENV1x90_H
#define __CCAENV1x90_h


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif



#ifndef __DESIGNBYCONTRACT_H
#include "DesignByContract.h"
#ifndef __DESIGNBYCONTRACT_H
#define __DESIGNBYCONTRACT_H
#endif
#endif

#ifndef VMEMODULE_H
#include <VmeModule.h>
#endif


// Class definitions.


class CCAENV1x90 
{
  // Exported data types:

public:
  /*!
    This enum defines the possible signals that the PROG_OUT
    ECL signal can represent.
  */
  typedef enum _ECLOutputSelect {
    DATA_READY,
    FULL,
    ALMOST_FULL,
    ERROR
  } ECLOutputSelect;

  struct TriggerConfiguration {
    unsigned short s_MatchWidth;
             short s_MatchOffset;
    unsigned short s_MatchExtra;
    unsigned short s_RejectMargin;
    unsigned short s_Subtracting;
  };
  typedef enum _EdgeMode {
    EdgeMode_Pair       = 0 ,
    EdgeMode_Leading    = 2,
    EdgeMode_Trailing   = 1,
    EdgeMode_Both       = 3

  } EdgeMode;
  typedef enum _Resolution {
    Res_25ps    = 3,			// 1290 only.
    Res_100ps   = 2,
    Res_200ps   = 1,
    Res_800ps   = 0
  } Resolution;
  typedef enum _LEResolution {
    LE_100ps        = 0,
    LE_200ps       = 1,
    LE_400ps       = 2,
    LE_800ps       = 3,
    LE_1600ps      = 4,
    LE_3120ps      = 5,
    LE_6250ps      = 6,
    LE_12500ps     = 7
  } LEResolution;
  typedef enum _PWResolution {
    PW_100ps       = 0,
    PW_200ps       = 1,  
    PW_400ps       = 2,
    PW_800ps       = 3,
    PW_1600ps      = 4,
    PW_3200ps      = 5,
    PW_6250ps      = 6,
    PW_12500ps     = 7,
    PW_25ns        = 8,
    PW_50ns        = 9,
    PW_100ns       = 10,
    PW_200ns       = 11,
    PW_400ns       = 12,
    PW_800ns       = 13
  } PWResolution;

  typedef enum _DeadTime {
    DT_5ns       = 0,
    DT_10ns      = 1,
    DT_30ns      = 2,
    DT_100ns     = 3
  } DeadTime;
  // Instance data.
  typedef enum _HitMax {
    HITS_0       = 0,			// No hits.
    HITS_1       = 1,
    HITS_2       = 2,
    HITS_4       = 3,
    HITS_8       = 4,
    HITS_16      = 5,
    HITS_32      = 6,
    HITS_64      = 7,
    HITS_128     = 8,
    HITS_UNLIMITED = 9		// No limit on hits.
  } HitMax;
  typedef enum _L1Size {
    L1_2wds,
    L1_4wds,
    L1_8wds,
    L1_16wds,
    L1_32wds,
    L1_64wds,
    L1_128wds,
    L1_256wds
  } L1Size;

  // public data

public:
  // Error masks for Get/Set ErrorEnables:
  
  static const unsigned short ERR_VERNIER;
  static const unsigned short ERR_COARSE;
  static const unsigned short ERR_SELECT;
  static const unsigned short ERR_L1PARITY;
  static const unsigned short ERR_TFIFOPARITY;
  static const unsigned short ERR_MATCHERROR;
  static const unsigned short ERR_RFIFOPARITY;
  static const unsigned short ERR_RDOSTATE;
  static const unsigned short ERR_SUPPARITY;
  static const unsigned short ERR_CTLPARITY;
  static const unsigned short ERR_JTAGPARITY;

  // Tap masks for calibrate delayline.
  
  static const unsigned short TAP_CONTACT1;
  static const unsigned short TAP_CONTACT2;
  static const unsigned short TAP_CONTACT3;

  
private:
  unsigned long m_nBase;	//!< VME base address of the module.
  unsigned int  m_nCrate;	//!< VME crate stuffed in.
  unsigned int  m_nSlot;	//!< Geo value.
  CVmeModule
            m_pRegisters;	//!< Device register pseudo pointer.
  unsigned int  m_nModel;	//!< Model, e.g. 1190 or 1290.
  unsigned char m_cVersion;	//!< Sub-model e.g. A,B or N.
  unsigned int  m_nSerialNumber; //!< Module serial number.
  unsigned int  m_nChipCount;	//!< Number of chips in the board
  unsigned int  m_nChannels;	//!< Number of channels on the board.
  unsigned int  m_nBoardRevision; //!< Board hardware revision level.
  bool          m_fCanHiRes;	//!< True if high resolution capable.
  bool          m_fIsHiResMode; //!< True if module is in hi res.
  bool          m_fTriggerMatching; //!< true if module has got
                                    //!< trigger match mode on.

  // Constructors and canonical operations.

public:
  CCAENV1x90(unsigned int nSlot, unsigned int nCrate,
	     unsigned long nBase); //!< geo not supported for these.
  ~CCAENV1x90();
private:			// Copy etc. not allowed.
  CCAENV1x90(const CCAENV1x90& rhs);
  CCAENV1x90& operator= (const CCAENV1x90& rhs);
  int         operator==(const CCAENV1x90& rhs) const;
  int         operator!=(const CCAENV1x90& rhs) const;
public:


  // Selectors:


public:

  unsigned int getModel() const { //!< Return model number.
    return m_nModel;
  }
  unsigned char getVersion() const { //!< Return type (e.g. 'N').
    return m_cVersion;
  }
  unsigned int getSerialNumber() const { //!< Module serial #
    return m_nSerialNumber;
  }
  unsigned int getChipCount() const { //!< # TDC Chips on board.
    return m_nChipCount;
  }
  unsigned int getChannelCount() const { //!< # Channels.
    return m_nChannels;
  }

  // Class operations:

public:
  unsigned short SR();		//!< Return the value of the board
                                //!< status register.
  bool isSetSR(unsigned short bitnum); //!< true if sr has 1 << bitnum set.
  unsigned short ReadCR();	//!< Return the value of the ctl reg.
  bool isSetCR(unsigned short bitnum); //!< true if sr has 1<< bitnum set.

  // Functionality exposed by the control register:

  void Terminate();		//!< Software terminate module.
  void Unterminate();		//!< Software unterminate module.
  void TerminateWithSwitch();	//!< Set termination to be via hardware switch
  void EnableTriggerTagTime();	//!< Put time tag in event.
  void DisableTriggerTagTime();	//!< don't put time tag in event.
 
  // Status register functionality:

  bool DataReady();		//!< True if there's data.
  bool AlmostFull();		//!< True if the buffer's almost full.
  bool isFull();		//!< True if the buffer is full.
  bool isTriggerMatching();	//!< True if module says it's trigger matching.
  bool isHeaderEnabled();	//!< True if TDC Header enabled.
  bool isTerminated();		//!< True if terminated regardless of how.
  bool HadError(unsigned int nChip);		//!< True if the TDC had an error since last reset.
  int  ReadResolution();	//!< Return resolution in ps/LSBit.
  bool isPairMode();		//!< True if the module is in pair mode.
  bool WereTriggersLost();	//!< True if triggers were lost.

  void SetGeographicalID(unsigned short int); //!< Set virtual slot number.
  unsigned short GetGeographicalID(); //!< Return the geographical id.

  // Key register access functions (sorry that's an SIS -ism).
  
  void Reset() ;			//!< Software rest the module.
  void Clear();			//!< Software clear the module.
  void EventReset();		//!< Resynchronize event counter.
  void Trigger();		//!< Create a software trigger.

  // Access various register counters.

  unsigned long TriggerCount();	//!< # Triggers since last clear.
  unsigned short EventCount();	//!< # events in output buffer.
 

  // Full levels:

  void SetAlmostFullLevel(unsigned int nWords);	//!< Set the almost full level.
  unsigned short  GetAlmostFullLevel(); //!< Where is the almost full threshold.
  

  // Define what the ECL output is set to be.

  void DefineECLOutput(ECLOutputSelect signal);	//!< Define the ECL PROG_OUT
  ECLOutputSelect GetECLOutputDefinition(); //!< What is the ECL PROG_OUT

  // State of the event fifo.  This is used to see what
  // an event in the buffer memory is going to look like:

  unsigned short EventFIFOCount(); //!< Return the # events in the fifo.
  unsigned long ReadEventFIFO(); //!< Return the next event description.
  bool         isEventFIFOReady(); //!< Events in the fifo?
  bool         isEventFIFOFull(); //!< Event fifo full?
  unsigned short FIFOEventNumber(unsigned long fifoentry); //!< Decode event # from Fifo.
  unsigned short FIFOWordCount(unsigned long fifoentry); //!< Decode event size from FIFO.


  // Here begin the whole series of operations that involve the
  // micro sequencer.  The labels in the text below 
  // are taken from the corresponding sections in the hardware
  // manual for the board.

  // 5.2 Acquisition modee opcodes:

  void TriggerMatchMode();	//!< Module-> trigger match mode (0x0000)
  void ContinuousStorageMode();	//!< Module-> cont. store mode   (0x0100)
  void TransferUntilDone();	//!< keep token  (0x0300)
  void TransferOneAtATime();	//!< Give token after 1 word (0x0400)
  void LoadDefaultConfig();	//!< reset power up config (0x0500)
  void SaveUserConfig();	//!< Save config->NVRAM (0x0600).
  void LoadUserConfig();	//!< Load user NVRAM config  (0x0700)
  void AutoLoadUserConfig();	//!< Load user NVRAM on power up (0x0800)
  void AutoLoadDefaultConfig();	//!< Load defaults on power up (0x0900)

  // 5.3 Trigger  opcodes.

  void SetWindowWidth(unsigned int nWidth);
				//!< Set trigger matching window width (0x1000)
  void SetWindowOffset(int nOffset); //!< Set trigger matching window offset (0x1100)
  void SetExtraSearchMargin(unsigned int nMargin); 
                                //!< Set trigger matching extra margin (0x1200)
  void SetRejectMargin(unsigned int nMargin);
				//!< Set trigger matching window reject margin (0x1300)
  void EnableTriggerTimeSubtraction(); //!< Subtract trigger time from hits (0x1400)
  void DisableTriggerTimeSubtraction();	//!< Subtract trigger time from hits (0x1500)
  TriggerConfiguration
       GetTriggerConfiguration();	//!< Return current trigger config (0x1800)
  unsigned int GetMatchWindow(TriggerConfiguration config);
           int GetWindowOffset(TriggerConfiguration config);
  unsigned int GetExtraSearchMargin(TriggerConfiguration config);
  unsigned int GetRejectMargin(TriggerConfiguration config);

  bool         isTriggerTimeSubtracted(TriggerConfiguration config);

  //  5.4 Trigger opcodes.
  
  void     SetEdgeDetectMode(EdgeMode nEdgeMode); //!< Set edge detect config (0x2200)
  EdgeMode GetEdgeDetectMode();	                  //!< Read edge detect mode (0x2300)
  void     SetIndividualLSB(Resolution nResolution); //!< Set resolution of lsb. (0x2400)
  void     SetPairResolutions(LEResolution nLeadingEdge,
			      PWResolution nPulseWidth); //!< Set pair resolution info (0x2500)
  unsigned short GetResolution();                  //!< Read module resolution see below (0x2600)
  Resolution InterpretEdgeResolution(unsigned short nResolution);
                                                   //!< Interpret resolution as individual edge.
  LEResolution InterpretLEResolution(unsigned short nResolution);
				//!< Interpret LE resolution in pair mode.
  PWResolution InterpretWidthResolution(unsigned short nResolution);
				//!< Interpret Width resolution in pair mode.
  void SetDoubleHitResolution(DeadTime nDead); //!< Set dead time between hits. (0x28))
  DeadTime GetDoubleHitResolution(); //!< Read double hit dead time (0x2900)

  // 5.5 TDC Readout OPCODES:

  void EnableTDCEncapsulation(); //!< Encapsulate data from a tdc (0x3000)
  void DisableTDCEncapsulation(); //!< Don't encapsulate data from a tdc (0x3100)
  bool isTDCEncapsulationOn();	//!< true if EnableTDCEncapsulation (0x3200)
  void SetMaxHitsPerEvent(HitMax nHits); //!< limit hits/event (0x3300)
  HitMax GetMaxHitsPerEvent();	//!< What's max hits/event (0x3400)
  void EnableErrorMark();	//!< turn on error mark words (0x3500)
  void DisableErrorMark();	//!< Turn off error mark words(0x3600)
  void EnableBypassOnError();	//!< Turn on TDC bypass on error (0x3700)
  void DisableBypassOnError();	//!< Turn off TDC Bypasson error (0x3800)
  void SetErrorEnables(unsigned short nErrors);	//!< selectively enable error types.
  unsigned short GetErrorEnables(); //!< Get error enables.
  void SetL1Size(L1Size nL1Size); //!< Set level 1 fifo size.
  L1Size GetL1Size();		//!< Get the level 1 fifo size.


  // 5.6 Channel enable opcodes. nn below is a tdc chip number.
  
  void EnableChannel(unsigned short nChannel); //!< Enbable 1 channel (0x40nn)
  void DisableChannel(unsigned short nChannel);	//!< Disable a channel (0x41nn)
  void EnableAllChannels();	//!< Enable all tdc channels (0x4200)
  void DisableAllChannels();	//!< Disable all tdc channels (0x4300)
  void SetChannelEnables(STD(vector)<unsigned short> masks); //!< Set enables mask for all chans (0x4400)
  void GetChannelEnables(STD(vector)<unsigned short>& masks); //!< read enables mask. (0x4500)
  void SetChipEnables(unsigned short nChip,
		      unsigned int nMask); //!< Set the enables for a single chip. (0x460n)
  unsigned int GetChipEnables(unsigned short nChip); //!< read enables for a chip. (0x470n)

  // 5.7 Adjustment opcodes.
  
  void SetGlobalOffset(unsigned short nCoarse,
		       unsigned short nVernier); //!< Set global coarse/fine offsets (0x5000)
  void ReadGlobalOffset(unsigned short& nCoarse,
			unsigned short& nVernier); //!< Fetch the global offsets (0x5100).
  void SetChannelOffset(unsigned short nChannel,
			unsigned short nOffset); //!< Set a channel offset (0x52nn).
  unsigned short GetChannelOffset(unsigned int nChannel); //!< Retrieve channel offset (0x53nn).
  // V1290 only:

  void CalibrateDelayLine(unsigned short nChip,
			  unsigned short Tap1Contact,
			  unsigned short Tap2Contact,
			  unsigned short Tap3Contact,
			  unsigned short Tap4Contat); //!< Set delayline tap config (0x450n)
  void GetDelayLineCalibration(unsigned short nChip,
			       unsigned short& Tap1Contact,
			       unsigned short& Tap2Contact,
			       unsigned short& Tap3Contact,
			       unsigned short& Tap4Contact); //!< Get delay line tap config (0x460n)
  void SaveDelayLineCalibrations(); //!< Write calibrations -> eprom (0x5600)
  
  // 5.8 Miscellaneous 

  unsigned int GetChipId(unsigned short nChip);	//!< Get chip id (0x600n)
  void GetuCFirmwareInfo(unsigned short& nRevision,
			 unsigned short& nDay,
			 unsigned short& nMonth,
			 unsigned short& nYear); //!< Firmware rev/date (0x6100).
  unsigned short GetChipErrors(unsigned short nChip); //!< Read errors from 1 chip (0x740n).

  // 5.9 system testing.

  void EnableTestMode(unsigned int nValue); //!< Chip->Test mode. (0xc500).
  void DisableTestMode(); //!< Turn off chip test mode (0xc600).

  // Data acquisition functions (finally)

  unsigned int ReadData(void* pBuffer,
			unsigned int nMaxLongs);	//!< Read up to nMaxLongs.
  unsigned int ReadPacket(void* pBuffer,
			  unsigned int nMaxLongs); //!< Read trigger matched event up to nMaxLongs
  unsigned int ReadValid(void* pBuffer,
			 unsigned int nMaxLongs); //!< Read all valid words, up to nMaxLongs.

  
  void WaitMicroWrite();	//!< Wait for micro writable.

  // utility functions

protected:
  bool ValidBoard(CVmeModule& pROM); //!< Is this a 1x90?
  void BoardProperties(CVmeModule& pROM);
  void WriteCR(unsigned short mask); //!< Write mask to status register.
  void BitSetCR(unsigned short bit); //!< set specified bit# in cr.
  void BitClearCR(unsigned short bit); //!< clear specified bit# in cr.

  void WriteMicro(unsigned short nWord); //!< Write word to the micro sequencer
  unsigned short ReadMicro();	//!< read a word from the micro sequencer.
  void WriteMicroBlock(void* pWords, 
		       unsigned int nWords); //!< Write words to the micro seqeuncer.
  void ReadMicroBlock(void* pWords,
		      unsigned int nWords); //!< Reads words from the micro sequencer.
  void MicroTransaction(unsigned short opcode,
			void* pWords,
			unsigned int nWords); //!< Do an opcode and read a block.
  unsigned long ReadPromLong(CVmeModule& prombase, 
			     unsigned long ByteOffset);

};
#endif
