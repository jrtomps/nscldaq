
#include <CMxDCRCBus.h>
#include <MADC32Registers.h>

#include <CVMUSB.h>
#include <CControlModule.h>
#include <CVMUSBReadoutList.h>
#include <VMEAddressModifier.h>
#include <sstream>
#include <memory>

using namespace std;

CMxDCRCBus::CMxDCRCBus () : CControlHardware() {}

void CMxDCRCBus::clone (const CControlHardware& rhs) {}

void CMxDCRCBus::onAttach(CControlModule& config) 
{
  m_pConfig = &config;
  config.addIntegerParameter("-base");
}

void CMxDCRCBus::Initialize(CVMUSB& ctlr) {}
std::string CMxDCRCBus::Update(CVMUSB& ctlr) {}
std::string CMxDCRCBus::Set(CVMUSB& ctlr, std::string what, std::string value) 
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");

  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());

  list->addWrite16(base+NIMBusyFunction, VMEAMod::a32UserData, 3);

  size_t nBytesRead = 0;
  uint32_t data[128];
  int status = ctlr.executeList(*list, data, sizeof(data), &nBytesRead);

  if (status<0) {
    stringstream errmsg;
    errmsg << "CMxDCRCBus::activate() - executeList returned status = ";
    errmsg << status;

    throw errmsg.str();
  }

}


std::string CMxDCRCBus::Get(CVMUSB& ctlr, std::string what) {}

void CMxDCRCBus::activate(CVMUSB& ctlr) {

  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  
  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());
  list->addWrite16(base+NIMBusyFunction, VMEAMod::a32UserData, 3);

  size_t nBytesRead = 0;
  uint32_t data[128];
  int status = ctlr.executeList(*list, data, sizeof(data), &nBytesRead);

  if (status<0) {
    stringstream errmsg;
    errmsg << "CMxDCRCBus::activate() - executeList returned status = ";
    errmsg << status;

    throw errmsg.str();
  }
}
