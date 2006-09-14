/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CSIS3300_H
#define __CSIS3300_H

#ifndef __CVMEREADABLEOBJECT_H
#include <CVMEReadableObject.h>
#endif


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

class CVMEList;
class CVMEInterface;
class CVMEAddressRange;
/*!
   This class provides support for the SIS 3300 flash adc.  While the module
   can be configured in multi-event mode, at present, we can only support
   readout in single event mode.  Multi-event mode can be supported later for
   programmatic event-by-event reads, but not for list based reads since
   the location read from will vary from event to event, not in general supported
   by list processors.

   The programming module is that one:
   - Constructs an object for each physical module.
   - Configures the object as desired.
   - calls initialize() to make the hardware correspond to the 
     configuration.
   - Invokes read() to read an event from the module into a buffer.
   - Invokes addReadoutToList to add the necessary instructions to a readout list
     for list driven readout software.

*/
class CSIS3300 : public CVMEReadableObject
{
  // data types;
public:
  typedef enum _ClockSource {
    external,
    internal3pt125KHz,
    internal6pt250KHz,
    internal12500KHz,
    internal20_25MHz,		//!< 20 or 25 mhz depending on module fpga clock.k
    internal40_50MHz,
    internal80_100Mhz
  } ClockSource;

  typedef enum _PageSize {
    page128K,
    page16K,
    page4K,
    page2K,
    page1K,
    page512,
    page256,
    page128
  } PageSize;

  typedef struct _Threshold {
    uint16_t s_value;
    bool     s_le;
  } Threshold;

  // Configuration and other data.
private:
  uint32_t m_baseAddress;
  CVMEAddressRange* m_pRegisters;
  CVMEAddressRange* m_pCommonRegisters;
  CVMEAddressRange* m_ppGroupRegisters[4];


  uint32_t m_IMUSTBEFIRST;	// >>>> All config items after this one.
  bool m_triggerOutputEnabled;
  bool m_triggerOutputInverted;
  bool m_triggerActiveWhenArmed;
  uint32_t  m_bankFullOutputMask;
  bool m_bank1ClockEnable;
  bool m_bank2ClockEnable;
  bool m_autoBankSwitch;
  bool m_randomClockMode;
  bool m_autoStart;
  bool m_multiEventMode;
  bool m_enableStartDelay;
  bool m_enableStopDelay;
  bool m_enableFPStartStop;
  bool m_enableFPGateMode;
  ClockSource m_clockSource;

  uint16_t m_startDelay;
  uint16_t m_stopDelay;
  uint16_t m_timestampPredivider;

  PageSize m_pageSize;
  bool m_wrap;
  bool m_gateChaining;
  Threshold m_thresholdInfo[8];
  bool m_externRandomClockMode;
  bool m_enableMuxMode;


  uint8_t m_clockPredivider;
  uint8_t m_muxModeSamples;
  bool    m_cfdEnabled;
  uint8_t m_cfdNumerator;
  uint8_t m_cfdDenominator;
  uint8_t m_cfdWidth;
  uint16_t m_chainMaxEvents;
  uint8_t m_groupReadMask;
  bool m_tagGroups;
  bool m_tagData;
  uint16_t m_dataTag;
  uint32_t m_IMUSTBELAST;	// >>>> Put no more config items after this.

  // State used in the 'fancier' readout modes.

  uint32_t m_currentBank;

  //  Constructors and other canonical functions:
public:
  CSIS3300(CVMEInterface& interface, uint32_t base);
  CSIS3300(const CSIS3300& rhs);
  virtual ~CSIS3300();
  CSIS3300& operator=(const CSIS3300& rhs);
  int operator==(const CSIS3300& rhs) const;
  int operator!=(const CSIS3300& rhs) const;

  // Configuration setters and getters:

  void configTriggerOutput(bool enabled);
  bool cgetTriggerOutputIsEnabled() const;

  void configTriggerOutputInverted(bool inverted);
  bool cgetTriggerOutputIsInverted() const;

  void configTriggerActiveWhenArmed(bool activeWhenArmed);
  bool cgetTriggerIsActiveWhenArmed() const;

  void configBankFullOutputMask(uint32_t mask);
  uint32_t cgetBankFullOutputMask() const;

  void configEnableBank1Clock(bool enabled);
  bool cgetBank1ClockIsEnabled() const;

  void configEnableBank2Clock(bool enabled);
  bool cgetBank2ClockIsEnabled() const;

  void configAutoBankSwitchEnabled(bool enabled);
  bool cgetAutoBankSwitchIsEnabled() const;

  void configRandomClockEnable(bool enabled);
  bool cgetRandomClockIsEnabled() const;

  void configAutoStartEnable(bool enabled);
  bool cgetAutoStartIsEnabled() const;

  void configMultiEventMode(bool enable);
  bool cgetIsMultiEventMode() const;

  void configEnableStartDelay(bool enable);
  bool cgetStartDelayIsEnabled() const;

  void configEnableStopDelay(bool enable);
  bool cgetStopDelayIsEnabled() const;

  void configEnableFrontPanelStartStop(bool enable);
  bool cgetFrontPanelStartStopIsEnabled() const;

  void configEnableFPGateMode(bool enable);
  bool cgetFPGateModeIsEnabled() const;

  void configExternRandomClockMode(bool on);
  bool cgetIsExternRandomClockMode() const;

  void configClockSource(ClockSource source);
  ClockSource cgetClockSource() const;

  void configMultiplexMode(bool enabled);
  bool cgetMultiplexMode() const;

  void configStartDelay(uint16_t delay);
  uint16_t cgetStartDelay() const;

  void configStopDelay(uint16_t delay);
  uint16_t cgetStopDelay() const;

  void configTimestampPredivider(uint16_t divisor);
  uint16_t cgetTimestampPredivider() const;

  void configPageSize(PageSize size);
  PageSize cgetPageSize() const;

  void configWrapEnable(bool enable);
  bool cgetWrapIsEnabled() const;

  void configGateChaining(bool enable);
  bool cgetIsGateChaining() const;

  void configChannelThreshold(unsigned channel,
			      Threshold value);
  Threshold cgetChannelThreshold(unsigned channel) const;

  void configClockPredivider(uint8_t divisor);
  uint8_t cgetClockPredivider() const;

  void configMuxModeSamples(uint8_t samples);
  uint8_t cgetMuxModeSamples() const;

  void configCFDNumerator(uint8_t numerator);
  uint8_t cgetCFDNumerator() const;

  void configCFDDenominator(uint8_t denominator);
  uint8_t cgetCFDDenominator() const;

  void configCFDWidth(uint8_t width);
  uint8_t cgetCFDWidth() const;

  void configCFDEnable(bool enable);
  bool cgetCFDIsEnabled() const;

  void configChainMaxEvents(uint16_t maxEvents);
  uint16_t cgetChainMaxEvents() const;

  void configGroupEnables(uint8_t mask);
  uint8_t cgetGroupEnables() const;

  void configTagGroups(bool tagThem);
  bool cgetTagGroups() const;

  void configDataTag(bool tagThem, uint16_t tagId = 0);
  bool cgetTaggingData() const;
  uint16_t cgetDataTag() const;

  // Direct access tothe device.

  uint32_t readFirmware();

  void     start();
  void     stop();
  void     reset();

  // Overrides of virtual functions.
public:
  virtual void initialize();
  virtual size_t read(void* buffer);
  virtual size_t largestEvent();
  virtual CVMEList& addReadoutTolist(CVMEList& list);


  // Additional read like functions:

public:
  size_t readSingle(void* buffer,  uint32_t event = 0);
  size_t readMulti(void* buffer, uint32_t firstEvent = 0, uint32_t lastEvent = 0);
  size_t readMultiBuffered(void* buffer, int block,
			   uint32_t firstEvent=0, uint32_t lastEvent=0);



  // Private utility functions:
private:
  void copyIn(const CSIS3300& rhs);
  void setDefaults();
  CVMEAddressRange& getControlRegisters();
  CVMEAddressRange& getCommonRegisters();
  CVMEAddressRange& getGroupRegisters(uint8_t group);
  void              key(uint32_t register);

  void     writeReg(CVMEAddressRange& page, uint32_t register, uint32_t value);
  uint32_t readReg(CVMEAddressRange& page,  uint32_t register);



  size_t pageSizeToSampleCount() const;
  size_t sampleToOffset(uint32_t sample, uint32_t event = 0);
  size_t readSelectedGroups(void* buffer, uint32_t startOffset, uint32_t endOffset,
			   uint32_t wrapOffset);
  size_t readGroup(void* buffer, int block, int group,
		   uint32_t startOffset, uint32_t endOffset, uint32_t wrapOffset);
  uint32_t eventDirOffset() const;	
};


#endif

