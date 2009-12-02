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

#include <config.h>
#include "CCAENV830Module.h"
#include "CCAENV830.h"
#include "CBoolConfigParam.h"
#include "CIntConfigParam.h"
#include "CIntArrayParam.h"

#include <daqinterface.h>
#include <TCLInterpreter.h>
#include <assert.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
/*!
    Construct a scaler module reader.  This does not actually
    construct the underlying CCAENV830 module.  Instead it
    constructs the configuration parameters for the module
    and defines them to the base class.  The module driver  itself
    will be constructed as a result of the Initialize() call.  

    \param rName (const string& [in]):
        Name to give to this module.  This name will be
	registered to the interpreter as a command that can be
	used to configure the module. 
    \param rInterp (CTCLInterpreter& rInterp [in]):
       The interpreter on which this module will be registered as a
       command
*/
CCAENV830Module::CCAENV830Module(const string& rName, CTCLInterpreter& rInterp) :
  CDigitizerModule(rName, rInterp),
  m_pModule(0),
  m_nId(0),
  m_fHeader(true),
  m_fWide(false),
  m_fManualClear(false),
  m_fPacketize(false),
  m_fVmeTrigger(false),
  m_lEnables(0xffffffff),
  m_nCrate(0)
{
  ParameterIterator i;
  CBoolConfigParam* pb;      
  CIntArrayParam*   pa;
  CIntConfigParam*  pi;

  

  // Register the parameters, set their limits anbd default values.

  AddIntParam(string("base"),0); // Base address.

  AddIntParam(string("slot"),2); // Geo address:
  i = Find(string("slot"));	// Set limits on geo:
  assert(i != end());
  pi = (CIntConfigParam*)(*i);
  pi->setRange(2, 25);		// Slot1 is arbiter!!.

  AddIntParam(string("crate"), 0);
  i = Find(string("crate"));	// Set limits on the crate.
  assert(i != end());
  pi= (CIntConfigParam*)(*i);
  pi->setRange(0, 255);

  AddBoolParam(string("geo"), true); // Default to geo addressing.

  AddIntArrayParam(string("enables"), 32, 1);
  i = Find(string("enables"));
  assert(i != end());
  pa= (CIntArrayParam*)(*i);
  pa->setRange(0, 1);		// It's really a set of flags.

  AddIntParam(string("trigger"),1); // Random trigger most usual.
  i = Find(string("trigger"));
  assert(i != end());
  pi = (CIntConfigParam*)(*i);
  pi->setRange(0,2);		// Encoded value.

  AddBoolParam(string("wide"), false); // Wide or narrow read.
  AddBoolParam(string("header"), true);	// Include header in output.
  AddBoolParam(string("autoreset"),true); // Reset on trigger?
  AddBoolParam(string("fpclearmeb"), false); // MEB Cleared by FP?
  AddBoolParam(string("manualclear"), false); // Clear after read.
  AddBoolParam(string("packetize"), false); // Prepend packet info
  AddBoolParam(string("vmetrigger"), false);
  AddStringArrayParam(string("parameters"), 32); // For scaler disp.

  
  AddIntParam(string("id"));	// Id of VPacket created.
  i = Find(string("id"));
  assert(i != end());
  pi = (CIntConfigParam*)(*i);
  pi->setRange(0, 65535);	// id fits in a word.
  
}
/*!
  Destruction of the module implies destruction of the card driver.
*/
CCAENV830Module::~CCAENV830Module()
{
  delete m_pModule;
}
/*!
  Initialize the module.
  This means:
  - Destroying any existing module driver.
  - Recovering and processing the configuration so that we
    now how to build and configure a new module driver.
  - constructing a new module driver.

*/
void
CCAENV830Module::Initialize()
{
  // We'll need these to manipulate the module.

  ParameterIterator i;
  CBoolConfigParam* pb;      
  CIntArrayParam*   pa;
  CIntConfigParam*  pi;
  int base;
  bool geo;
  CCAENV830::TriggerMode  trigger;
  bool autoreset;
  bool fpclearmeb;

 
  

  delete m_pModule;		// Get rid of the old module.
  m_pModule = 0;

  // Recover the configuration.

  
  i    = Find("base");
  pi   = (CIntConfigParam*)(*i); // Assume construction assured existence.
  base = pi->getOptionValue();

  i    = Find("slot");
  pi   = (CIntConfigParam*)(*i);
  m_nSlot = pi->getOptionValue();

  i    = Find("geo");
  pb   = (CBoolConfigParam*)(*i);
  geo  = pb->getOptionValue();


  i    = Find("crate");
  pi   = (CIntConfigParam*)(*i);
  m_nCrate = pi->getOptionValue();

  i    = Find("enables");
  pa   = (CIntArrayParam*)(*i);
  m_lEnables = 0;
  for(int i =0; i < 32; i++) {
    if ((*pa)[i]) {
      m_lEnables |= (1 << i);
    }
  }

  i       = Find("trigger");
  pi      = (CIntConfigParam*)(*i);
  switch(pi->getOptionValue()) {
  case 0:
    trigger = CCAENV830::Disabled;
    break;
  case 1:
    trigger = CCAENV830::Random;
    break;
  case 2:
    trigger = CCAENV830::Periodic;
    break;
  default:
    throw string("Invalid trigger mode in CCAENV830 module config");
    
  }

  i       = Find("wide");
  pb      = (CBoolConfigParam*)(*i);
  m_fWide = pb->getOptionValue();

  i       = Find("header");
  pb      = (CBoolConfigParam*)(*i);
  m_fHeader = pb->getOptionValue();

  i       = Find("autoreset");
  pb      = (CBoolConfigParam*)(*i);
  autoreset = pb->getOptionValue();

  i       = Find("fpclearmeb");
  pb      = (CBoolConfigParam*)(*i);
  fpclearmeb = pb->getOptionValue();

  i       = Find("id");
  pi      = (CIntConfigParam*)(*i);
  m_nId   = pi->getOptionValue();


  i       = Find("manualclear");
  pb      = (CBoolConfigParam*)(*i);
  m_fManualClear = pb->getOptionValue();

  i       = Find("packetize");
  pb      = (CBoolConfigParam*)(*i);
  m_fPacketize = pb->getOptionValue();

  i       = Find("vmetrigger");
  pb      = (CBoolConfigParam*)(*i);
  m_fVmeTrigger = pb->getOptionValue();


  // construct/configure the new module.

  m_pModule = new CCAENV830(m_nSlot, m_nCrate, geo, 
			    (unsigned long)base);

  m_pModule->SetEnableMask(m_lEnables);
  m_pModule->SetTriggerMode(trigger);
  if(m_fWide) {
    m_pModule->SetWide();
  } 
  else {
    m_pModule->SetNarrow();
  }
  if(m_fHeader) {
    m_pModule->EnableHeader();
  } 
  else {
    m_pModule->DisableHeader();
  }
  if(autoreset) {
    m_pModule->EnableAutoReset();
  } 
  else {
    m_pModule->DisableAutoReset();
  }
  m_pModule->FPClearsMEB(fpclearmeb);


  // Clear the MEB:

  m_pModule->Clear();
}
/*!
   Prepare to read an event (first or after the prior one).
   This is a NO-OP as the module does not need any resetting etc.
*/
void
CCAENV830Module::Prepare()
{}
/*!
   Read an Event from the module.  See the class documentation
   for the data format.
   
   \param  rBuffer (DAQWordBufferPtr& [out]):
      'pointer' to the buffer to fill with data.

   \note
       The buffer pointer is advanced beyond the data event to
       point to the next free word.
*/
void
CCAENV830Module::Read(DAQWordBufferPtr& rBuffer)
{
  // Maximum buffer is 32 channels (longs) 
  //                 + 1  Their header (long)
  //                 + 5  My max header size (words).

  short Buffer[ 32+1*sizeof(long)/sizeof(short) + 5];

  int nBytes = Read(Buffer);
  int nWords = nBytes/sizeof(short);
#ifdef CLIENT_HAS_POINTER_COPYIN
  rBuffer.CopyIn(Buffer, 0, nWords);
  rBuffer+= nWords;
#else
  for(int i =0; i < nWords; i++) {
    *rBuffer = Buffer[i];
    ++rBuffer;
  }
#endif

}
/*!
   Read the module into an ordinary buffer.
*/
int
CCAENV830Module::Read(void* pBuffer)
{
  bool jumbo = daq_isJumboBuffer();

  short*  p((short*)pBuffer);
  int nHeaderWords = 0;

  // Fill in what we can of the event header:

  short*  pwc = p;
  if(m_fPacketize) {		// Put in packet header...
    ++p;			// Leave room for word count.
    if (jumbo) ++p;		// which might be a longword.
    *p = m_nId;		// Id of the packet.
    ++p;
    UShort_t hdr = 0;
    if(m_fHeader) hdr |= 0x8000;
    if(m_fWide)   hdr |= 0x4000;
    hdr |= (m_nSlot << 9);
    hdr |= m_nCrate;
    *p = hdr;
    ++p;
    nHeaderWords+= jumbo ? 4 : 3;		// size, id, description.

    if(m_fWide) {			// If wide, put enables:
      *p = (UShort_t)(m_lEnables & 0xffff);
      ++p;
      *p = (UShort_t)((m_lEnables >> 16) & 0xffff);
      ++p;
      nHeaderWords+= 2;		// Enables mask.
    }
  }
  if(m_fVmeTrigger) {
    m_pModule->Trigger();
  }
  for(int i =0; i < 100; i++) {
    if(m_pModule->isDataReady()) 
      break;
  }
  int nWords = m_pModule->ReadEvent(p);
  if(m_fManualClear) {
    m_pModule->Clear();		// If desired, manually clear mod.
  }


  if(m_fPacketize) {		// Wordcount if packetizing.
    nWords        += nHeaderWords;
    if (jumbo) {
      union {
	unsigned int   l;
	unsigned short w[2];
      } lw;
      lw.l = nWords;
      *pwc++ = lw.w[0];
      *pwc   = lw.w[1];
    }
    else {
      *pwc = nWords;
    }
  }
  return (nWords);
    
}
/*!
   Clear is not required on an event by event basis since it's
   done in read to minimize dropped counts.
*/
void
CCAENV830Module::Clear() {
}

