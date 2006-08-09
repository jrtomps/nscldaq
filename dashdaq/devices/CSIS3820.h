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

#ifndef __CSIS3820_H
#define __CSIS3820_H

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
   CSIS3820 is a class that supports the SIS 3820 scaler module
   this module currently has three supported modes of operation:
   - Latching scaler
   - Multi-event scaler
   - Histogramming scaler.
   
   The hardware also supports a preset scaler mode which is not supported
   by this driver software.

   The support software sets up the module via a configure/initialize model.
   That is, after constructing a module, you can make all sorts of configuration
   requests.. none of which actually touch any hardware.   The initialize() member
   is then called and it sets up the modules as described by the current values of 
   the configuration.

   Each configuration parameter xxx has a configurexxx member which sets it and
   a cgetxxx parameter which interrogates it.

*/
class CSIS3820 : public CVMEReadableOjbect
{
  // Data types (public):

public:
  typedef enum _mode {
    Latching,
    MCS,
    Histogramming,
  } mode;

  typedef enum _DataFormat {
    format32,
    format24,
    format16,
    format08
  } DataFormat;
 
  typedef enum _LNESource {
    LNEVME,
    LNEFrontPanel,
    LNE10MHz
  } LNESource;

  typedef enum _ArmEnableSource {
    ArmFPLNE
  } ArmEnableSource;

  typedef enum _InputMode {
    NoInputs,
    LneInhibitLne,
    LneInhibitLneAndAll,
    LneInhibitAll,
    LneInhibitBanks8
  } InputMode;

  typedef enum _OutputMode {
    SDRAM,
    Clock,
    Unused
  } OutputMode;

  typedef enum _Polarity {
    normal,
    inverted
  } Polarity;
  
  // Configuration data:

private:
  CVMEAddressRange* m_pVme;
  mode            m_mode;
  bool            m_enableRefPulser;
  uint32_t        m_mcsPreset;
  uint32_t        m_lnePrescale;
  bool            m_disableClear;
  DataFormat      m_dataFormat;
  LNESource       m_lneSource;
  ArmEnableSource m_armEnableSource;
  bool            m_SDRAMisFIFO;
  bool            m_inputsInverted;
  InputMode       m_inputMode;
  bool            m_outputsInverted;
  OutputMode      m_outputMode;
  uint32_t        m_channelCopyDisables;
  uint32_t        m_channelDisables;
  

  /// Constructors etc.

public:
  CSIS3820(CVMEInterface& interface, uint32_t base);
  CSIS3820(const CSIS3820& rhs);
  virtual ~CSIS3820();
  
  CSIS3820& operator=(const CSIS3820& rhs);
  int operator==(const CSIS3820& rhs) const;
  int operator!=(const CSIS3820& rhs) const;

  ///   Configuration getters/setters.

  void configMode(mode operatingMode);
  mode cgetMode() const;

  void configEnableRefPulser();
  void configDisableRefPulser();
  bool cgetRefPulserEnabled() const;

  void     configMCSPreset(uint32_t cycles);
  uint32_t cgetMCSPreset() const;

  void     configLNEPrescale(uint32_t scaledown);
  uint32_t cgetLNEPrescale() const;

  void configDisableClearOnLNE();
  void configEnableClearOnLNE();
  bool cgetClearOnLNEEnabled() const;

  void       configDataFormat(DataFormat format);
  DataFormat cgetDataFormat() const;

  void       configLNESource(LNESource source);
  LNESource  cgetLNESource() const;

  void            configArmEnableSource(ArmEnableSource source);
  ArmEnableSource cgetArmEnableSource() const;

  void configSDRAMasFIFO();
  void configSDRAMasRAM();
  bool cgetSDRAMisFIFO() const;

  void     configInputPolarity(Polarity mode);
  Polarity cgetInputPolarity() const;

  void      configInputMode(InputMode mode);
  InputMode cgetInputMode() const;

  void     configOutputsPolarity(Polarity mode);
  Polarity cgetOutputPolarity() const;
  
  void       configOutputMode(OutputMode mode);
  OutputMode cgetOutputMode() const;

  void     configCopyDisableMask(uint32_t disableMask);
  uint32_t cgetCopyDisableMask () const;

  void       configChannelDisableMask(uint32_t disableMask);
  uint32_t   cgetChannelDisableMask() const;

  // key register immediate functions:


  void LNE();
  void Arm();
  void Reset();


  /// virtual function overrides:

public:
  virtual void    initialize();
  virtual size_t  read(void* buffer);
  virtual size_t  largestEvent();
  virtual CVMEList& addReadoutToList(CVMEList& list);
  

  /// Private helpers:

  CVMEAddressRange& registers();
  void     regWrite(uint32_t offset, uint32_t value);
  uint32_t regRead(uint32_t offset);

  void setConfigurationDefaults();
  void copyIn(const CSIS3280& rhs) const;

  size_t     readShadow(void* buffer);
  size_t     readSDRAM(void* buffer);

  CVMEList& addShadowRead(CVMEList& list);
  CVMEList& addSDRAMRead(CVMEList& list);

  uint32_t liveChannels();


};


#endif
