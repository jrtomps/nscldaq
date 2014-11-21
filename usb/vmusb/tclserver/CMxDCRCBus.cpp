
#include <CMxDCRCBus.h>

#include <CVMUSB.h>
#include <CControlModule.h>

using namespace std;

CMxDCRCBus::CMxDCRCBus (string name) : CControlHardware(name) {}

void CMxDCRCBus::clone (const CControlHardware& rhs) {}

void CMxDCRCBus::onAttach(CControlModule& config) {}
void CMxDCRCBus::Initialize(CVMUSB& ctlr) {}
std::string CMxDCRCBus::Update(CVMUSB& ctlr) {}
std::string CMxDCRCBus::Set(CVMUSB& ctlr, std::string what, std::string value) {}
std::string CMxDCRCBus::Get(CVMUSB& ctlr, std::string what) {}

void CMxDCRCBus::activate(CVMUSB& ctlr) {
  
}
