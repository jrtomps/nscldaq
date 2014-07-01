
#include <string.h>

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CControlModule.h>
#include <CXLMControls.h>
#include <CXLM.h>

using namespace std;


namespace XLM
{

///////////////////////////////////////////////////////////////////////////////
/////////////////////// SLOW CONTROLS FOR FIRMWARE LOADS //////////////////////
///////////////////////////////////////////////////////////////////////////////

CXLMControls::CXLMControls(string name)
  : CControlHardware(name)
{}

void CXLMControls::onAttach(CControlModule& config)
{
  m_pConfig = &config;
  m_pConfig->addParameter("-base", CConfigurableObject::isInteger, NULL, "0");
  m_pConfig->addParameter("-firmware", Utils::validFirmwareFile, NULL, "");
}

void CXLMControls::Initialize(CVMUSB& controller)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  string path = m_pConfig->cget("-firmware");
  loadFirmware(controller, base, SRAMA, path);
}

void CXLMControls::clone(const CControlHardware& controller) {}

string CXLMControls::Update(CVMUSB& controller)
{
  return "ERROR - Update is not implemented for CXLMControls";
}

string CXLMControls::Set(CVMUSB& controller, string what, string val) 
{
  return "ERROR - Set is not implemented for CXLMControls";
}

string CXLMControls::Get(CVMUSB& controller, string what)
{
  return "ERROR - Get is not implemented for CXLMControls";
}

} // end XLM namespace
