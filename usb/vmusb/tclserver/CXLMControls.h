
#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif

// Forward class definitions:

class CControlModule;
class CVMUSB;
class CVMUSBReadoutList;

///////////////////////////////////////////////////////////////////////////////
//////////////////////// SLOW CONTROLS FOR FIRMWARE LOADS /////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace XLM
{


class CXLMControls : public CControlHardware
{
  public:
  virtual void clone( const CXLMControls& rhs);

  virtual void onAttach (CControlModule& config);
  virtual void Initialize(CVMUSB& controller);
  virtual std::string Update(CVMUSB& vme);
  virtual std::string Set(CVMUSB& vme, std::string what, std::string value);
  virtual std::string Get(CVMUSB& vme, std::string what);

};


} // end XLM namespace
