#define autoread                  // set for ECL signal commanded readout cycle

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <iostream>
#include <string>

/* global variables */

/*
static int ShiftRegLen;
static int NumBoards;
static int NumChips;
static int SelectedChannel = 0;
static int SelectedChip = 1;
static char result[24];
static char res1;
*/

/*
**
**  
INCLUDE FILES
**
*/

#ifndef __unix__
#include "cpus.h"
#endif

#ifdef __unix__
#include <stdlib.h>
#include <daqinterface.h>
#include <spectrodaq.h>
#endif

#include <daqdatatypes.h>
#include <camac.h>
#include <macros.h>

#include <XLM.h>
#include <sis3301.h>
// #include <HiRACard.h>

#ifdef VME16
#undef VME16
#endif
#if CPU == MICROPROJECT
#define VME16 0	
#endif

#ifndef __unix__
#include <vme.h>
#endif
#include <buftypes.h>

#include <CEventSegment.h>

extern INT16 second;

/*********************************************************************/
/*********************************************************************/

class 
HIRA : public CEventSegment
{
 public:
  /* The Following are used for Packet Identification */
  
  static const unsigned short HIRA = 0x3100;
  static const unsigned short VMEADC = 0x4144;
  static const unsigned short VMETDC = 0x5444;
  static const unsigned short VMEQDC = 0x5144;
  static const unsigned short VMEIGQDC = 0x4947;
  static const unsigned short VMEFADC = 0x4641;
  static const unsigned short VMEXLM = 0x584C;
  
  CSIS3301 *FlashAdc(0x33000000);
  //  XLM *theXLM("hira_card",7);  // XLM80 is VME slot 7
  XLM *theXLM("hira_card",6);  // XLM XXV is VME slot 6
  /*******************************************/
   
  HIRA();
  virtual ~HIRA(){};

  void ClkBit(int byte)
  {
    if (byte == 1)
      {
	theXLM.WriteFPGA(5,serin);  // apply the serial bit first
	theXLM.WriteFPGA(5,serclk|serin); //Clock Signal high
	theXLM.WriteFPGA(5,serin); //Clock Signal Low
      }
    else 
      {
	theXLM.WriteFPGA(5,0);   //Clock low, ser low	 
	theXLM.WriteFPGA(5,serclk);   //Clock high, ser low
	theXLM.WriteFPGA(5,0);   //Clock low, ser low	 
      }
    return;
  }
  
  void Clear();
  virtual DAQWordBufferPtr& Read(DAQWordBufferPtr& bufpt);
  void Initialize();
  virtual unsigned int MaxSize() {return 0;}
  
};
