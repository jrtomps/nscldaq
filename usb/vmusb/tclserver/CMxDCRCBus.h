
#ifndef CMXDCRCBUS_H
#define CMXDCRCBUS_H

#include <string>
#include <cstdint>
#include <utility>
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
    uint16_t readResponse(CVMUSB& ctlr); 
    std::pair<uint16_t,uint16_t> parseAddress(std::string what);

    void addParameterWrite(CVMUSBReadoutList& list, 
                           std::pair<uint16_t,uint16_t> addr,
                           uint16_t value);
    void addParameterRead(CVMUSBReadoutList& list, 
                           std::pair<uint16_t,uint16_t> addr);

    bool responseIndicatesError(uint16_t datum);
    std::string convertResponseToErrorString(uint16_t datum);
};

#endif
