
#ifndef CMXDCRCBUS_H
#define CMXDCRCBUS_H

#include <string>
#include <CControlHardware.h>

class CVMUSB;
class CControlModule;

class CMxDCRCBus : public ::CControlHardware
{
  public:
    CMxDCRCBus();

    virtual void clone(const CControlHardware& rhs);

    virtual void onAttach(CControlModule& config);
    virtual void Initialize(CVMUSB& ctlr);
    virtual std::string Update(CVMUSB& ctlr);
    virtual std::string Set(CVMUSB& ctlr, std::string what, std::string value);
    virtual std::string Get(CVMUSB& ctlr, std::string what);

  private:
    void activate(CVMUSB& ctlr);
};

#endif
