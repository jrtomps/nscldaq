/*!
   CAENV890 provides class level driver support for the CAEN
   V890 multihit TDC.   This module is a bit weird.  All setup is done
   by interacting with an on-board microprocessor via a set of communications
   registers.  An extensive set of uProcessor commands and their parameters.
   See the member function implementation documentation for more details.

   In most cases to use the board it is not necessary to know the details of
   this communication as that's hidden from you.

*/
#ifndef __V890_H
#define __V890_H


class CAENV890 {
  int             m_nSlot;
  int             m_nCrate;
  bool            m_fGeographical;
  unsigned long   m_lBase;
  void* m_pModule;
  void*           m_pVmeDevice;
  unsigned        m_nModuleType;
public:
  static const int m_nTdcClock;	// ns per tdc clock.
  static const int m_nTdcMaxVal; // Maximum tdc value.
  typedef struct TriggerInfo {	//!< Struct returned by ReadTriggerConfig
    unsigned short Window;
    short Offset;
    short ExtraSearchWindow;
    short RejectMargin;
    short SubtractionEnabled;
  };
  typedef enum _EdgeDetectSelection {	
    EdgePairs    = 0,
    LeadingEdge  = 1,
    TrailingEdge = 2,
    EitherEdge   = 3
  } EdgeDetectSelection;       //!< Values for edge detection.

  typedef enum _ResolutionSelection {
    ps800      = 0,
    ps200      = 1,
    ps100      = 2
  } ResolutionSelection;	//!< Values for resolution.

  typedef enum _MaxHitSelection {
    HitMax0,
    HitMax1,
    HitMax2,
    HitMax4,
    HitMax8,
    HitMax16,
    HitMax32,
    HitMax64,
    HitMax128,
    HitMaxNoLimit
  } MaxHitSelection;
public:
  // Constructors and canonical functions:

  CAENV890(int           nSlot, 
	   int           nCrate = 0, 
	   bool          fGeo   = true, 
	   unsigned long lBase  = 0);
  ~CAENV890();
  
  // Selectors:
  
public:
  int getSlot() const {
    return m_nSlot;
  }
  int getCrate() const {
    return m_nCrate;
  }
  bool getGeo() const {
    return m_fGeographical;
  }
  unsigned long getBase() const {
    return m_lBase;
  }
  void* getRegisters() {
    return m_pModule;
  }
  void* getVmeDevice() {
    return m_pVmeDevice;
  }
  unsigned getModuleType() const {
    return m_nModuleType;
  }

  // Operations on the module:


  unsigned getSlotRegister() const; //!< Get hardware slot.
  void     EmptyEvent(bool state);  //!< allow/disallow empty events.
  bool     EmptyEventOn();	    //!< Empty events allowed? 
  void     Reset();                 //!< Soft reset the module.
  void     Clear();                 //!< Soft clear the module.
  void     TestMode(bool enable, unsigned long nPattern = 0);
  void     LoadDefaultConfig();
  bool     isTriggerMatching();
  void     ReadTriggerConfig(TriggerInfo* pTriggerInfo);

  /// Trigger mode management:

  void SetTriggerMatchingMode();
  void SetContinuousStorageMode();
  bool isTriggerMatchingMode();

  /// Channel enables:

  void EnableChannel(int nChan);
  void DisableChannel(int nChan);
  void EnableAllChannels();
  void DisableAllChannels();
  void SetChannelMask(unsigned short* pMask);
  void ReadChannelEnables(unsigned short* pEnables);
  void SetWindowWidth(int ns);
  int GetWindowWidth();
  void SetWindowOffset(int ns);
  int GetWindowOffset();
  void EnableTriggerSubtraction();
  void DisableTriggerSubtraction();
  bool TriggerSubtractionEnabled();
  void SetEdgeDetection(EdgeDetectSelection edge);
  EdgeDetectSelection GetEdgeDetection();
  void SetResolution(ResolutionSelection res);
  ResolutionSelection GetResolution();
  void EnableDelimeters();
  void DisableDelimeters();
  bool DelimetersEnabled();
  void SetMaxHits(MaxHitSelection hits);
  MaxHitSelection GetMaxHits();
  bool DataReady();

  // Data transfer from the module:
  
  unsigned int Read(unsigned int nBufferSize, void* pBuffer);


protected:
  void MapModule();
  void* MapRegions(void* pfd, unsigned long base);
  void WriteMicro(unsigned short opcode);
  unsigned short ReadMicro();
  void ReadMicroArray(unsigned short opcode,
		      unsigned short nWords, void* pBuffer);
  void WriteMicroArray(unsigned short opcode,
		       unsigned short nWords, void* pBuffer);
  static int ReadModuleType(void* prom);
  static int  TicksToNs(int ticks);
  static int  NsToTicks(int ns);
};

#endif
