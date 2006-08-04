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

#ifndef __CAEN32_H
#define __CAEN32_H

#ifndef __CVMEREADABLEOBJECT_H
#include "CVMEReadableObject.h"
#endif


#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

// forward definitions

class CVMEInterface;
class CVMEList;
class CVMEAddressRange;

/*!
   Class to support  the CAEN 32 channel digitizer set based on a 32 channel
   dual chip peak sensing adc motherboard with personality daughterboards that
   turn it into  the following modules:
   - V775  TDC
   - V785  Peak Sensing ADC
   - V792  Common gated QDC
   - V862  Individually gated QDC
   - V11785 Dual range ADC.

   Note that the operations that configure this module come in two flavors:
   - set*:get* will perform immediate operations on the module.  Only a minimum
     of functions will do this so that interfaces that are not efficient for
     single shot transfers are more efficiently utilized.
   - config*:cget*  operate on a memorized configuration that is used to set up
     the module by initialize.
*/
class CAEN32 : public CVMEReadableObject
{
  // Published data types:
public:
  typedef enum _cbltPosition {
    Unchained,
    First,
    Last,
    Middle
  } cbltPosition;

  typedef enum _action {
    keep,
    discard
  } action;

  typedef enum _tdcmode {
    commonstart,
    commonstop
  } tdcmode;

  typedef enum _thresholdRange {
    small,
    large,
  } thresholdRange;

  // Configuration:
private:
  CVMEAddressRange*      m_pRegisters;
  uint32_t               m_base;
  uint32_t               m_cardType;      
  uint8_t                m_crate;
  bool                   m_hardwareGeo;  
  uint8_t                m_softGeo;
  bool                   m_keepUnderThreshold;
  bool                   m_keepOverflows;
  bool                   m_keepInvalid;
  bool                   m_TDCcommonStop;
  uint16_t               m_TDCtimeRange;
  uint16_t               m_QDCIped;
  bool                   m_enable;
  std::vector<bool>      m_channelEnables;
  std::vector<uint16_t>  m_thresholds;
  uint16_t               m_fastClearWindow;
  bool                   m_smallThresholds;
  cbltPosition           m_chainPosition;
  uint32_t               m_cbltBase;

  bool                   m_clearAfterRead;

  // Construction and canonicals.  There's no real reason to exclude
  // copy operations.

public:
  CAEN32(CVMEInterface& interface, uint32_t base);
  CAEN32(const CAEN32& rhs);
  virtual ~CAEN32();
  
  CAEN32& operator=(const CAEN32& rhs);
  int operator==(const CAEN32& rhs) const;
  int operator!=(const CAEN32& rhs) const;

  // Set/get members:

  uint16_t getFirmware();
  uint32_t getCardType();
  uint16_t getSerial();

  // Configuration members

  void    configCrate(uint8_t crate);
  uint8_t cgetCrate() const;

  void                  configThresholds(std::vector<uint16_t> thresholds);
  std::vector<uint16_t> cgetThresholds() const;

  void    configHardwareGeo(bool hasHardwareGeo);
  void    configGeo(uint8_t geo);
  uint8_t cgetGeo() const;

  void configUnderThresholdAction(action what);
  void configOverflowAction(action what);
  void configInvalidAction(action what);

  void configCardEnable(bool enabled);
  bool cgetCardEnable() const;

  void configChannelEnable(uint8_t channel, bool enabled);
  bool cgetChannelEnable(uint8_t channel) const;

  void configFastClearInterval(uint16_t window);
  uint16_t cgetFastClearInterval() const;

  void configThresholdMeaning(thresholdRange range);
  thresholdRange cgetThresholdMeaning() const;

  void configCBLTMembership(cbltPosition membership, uint32_t cbltBase);
  cbltPosition cgetCBLTPosition() const;
  uint32_t     cgetCBLTBase() const;

  void configClearAfterRead(bool clear);
  bool cgetClearAfterRead() const;


  // Device specific configuration:

      // V775 only:

  void     configTDCMode(tdcmode mode);
  tdcmode  cgetTDCMode() const;
  void     configTDCRange(uint16_t range);
  uint16_t cgetTDCRange() const;


      // V792, V862 only:

  void     configQDCCompensationCurrent(uint16_t iped);
  uint16_t cgetQDCCompensationCurrent() const;


  // members with immediate effect on the device:

  void clearData();
  void reset();
  bool dataPresent();
  bool globalDataPresent();
  bool isBusy();
  bool isGlobalBusy();
  bool isEmpty();
  bool isFull();

  // Event read members... that are non standard.

  size_t readAllEvents(void* buffer); //!< Drain the MEB of all events.

  // Virtual overrides

  virtual void initialize();
  virtual size_t read(void* buffer);
  virtual CVMEList& addReadoutToList(CVMEList& list);
  virtual size_t largestEvent();


private:
  void setConfigurationDefaults();
  void copyIn(const CAEN32& rhs);
  bool validDeviceType() const;
  static void validChannel(uint8_t channel, const char* msg);
  bool   isTdc() const;
  bool   isQdc() const;

  uint16_t readreg(uint32_t offset);
  void     writereg(uint32_t offset, uint16_t value);
 
  void     bitset1(uint16_t mask);
  void     bitclr1(uint16_t mask);

  void     bitset2(uint16_t mask);
  void     bitclr2(uint16_t mask);


  uint16_t status1();
  uint16_t status2();

  uint32_t meb();

  void setThreshold(uint32_t index, uint16_t value);

};

#endif
