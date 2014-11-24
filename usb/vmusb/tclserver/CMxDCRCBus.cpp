
#include <CMxDCRCBus.h>
#include <MADC32Registers.h>

#include <CVMUSB.h>
#include <CControlModule.h>
#include <CVMUSBReadoutList.h>
#include <VMEAddressModifier.h>
#include <sstream>
#include <memory>
#include <limits>
#include <cstdio>
#include <iostream>

using namespace std;


/**!
 *
 */
CMxDCRCBus::CMxDCRCBus () : CControlHardware() {}

/**!
 *
 */
void CMxDCRCBus::clone (const CControlHardware& rhs) {}

/**!
 *
 */
void CMxDCRCBus::onAttach(CControlModule& config) 
{
  m_pConfig = &config;
  config.addIntegerParameter("-base");
}

/**!
 *
 */
void CMxDCRCBus::Initialize(CVMUSB& ctlr) {}

/**!
 *
 */
std::string CMxDCRCBus::Update(CVMUSB& ctlr) {}

/**!
 *
 */
std::string CMxDCRCBus::Set(CVMUSB& ctlr, std::string what, std::string value) 
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");


  // extract the device address and parameter address encoded in "what" argument
  pair<uint16_t,uint16_t> busAddress = parseAddress(what);

  uint16_t dataToWrite = atoi(value.c_str());

  // Create a list and append the ensemble of commands for a parameter write 
  // to it.
  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());
  addParameterWrite(*list, busAddress, dataToWrite);

  size_t nBytesRead = 0;
  uint32_t data[128];
  int status = ctlr.executeList(*list, data, sizeof(data), &nBytesRead);
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::Set - executeList returned status = ";
    errmsg << status;

    return errmsg.str();
  }

  // Read the response
  uint16_t response = readResponse(ctlr);
  if (responseIndicatesError(response)) {
    return convertResponseToErrorString(response);
  } 

  uint16_t dataRead;
  status = ctlr.vmeRead16(base+RCData, VMEAMod::a32UserData, &dataRead); 
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::Set - failure reading back written value. ";
    errmsg << "Status = " << status;
    return errmsg.str();
  }

  if (dataRead != dataToWrite) {
    string errmsg("ERROR - CMxDCRCBus::Set - ");
    errmsg += "Failed to read back same value as was written.";
    return errmsg;
  }

  return string("OK");
}



/**!
 *
 */
std::string CMxDCRCBus::Get(CVMUSB& ctlr, std::string what) 
{
  
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  
  // extract the device address and parameter address encoded in "what" argument
  pair<uint16_t,uint16_t> busAddress = parseAddress(what);

  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());
  addParameterRead(*list, busAddress);

  size_t nBytesRead = 0;
  uint16_t data[1];
  int status = ctlr.executeList(*list, data, sizeof(data), &nBytesRead);
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::Get - executeList returned status = ";
    errmsg << status;

    return errmsg.str();
  }

  // Read the response
  uint16_t response = readResponse(ctlr);
  if (responseIndicatesError(response)) {
    return convertResponseToErrorString(response);
  } 

  uint16_t dataRead;
  status = ctlr.vmeRead16(base+RCData, VMEAMod::a32UserData, &dataRead); 
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::Get - failure reading back written value. ";
    errmsg << "Status = " << status;
    return errmsg.str();
  }

  std::stringstream retstr;
  retstr << "OK - " << dataRead;
  return retstr.str();

}

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


uint16_t CMxDCRCBus::readResponse(CVMUSB& ctlr)
{
    
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  
  // To ensure that we at least do one iteration through this
  uint16_t data = 0;
  do {
    data = ctlr.vmeRead16(base+RCStatus, VMEAMod::a32UserData, &data);
  } while (data!=0);

  return data;
}

std::pair<uint16_t,uint16_t> CMxDCRCBus::parseAddress(std::string what)
{
  uint16_t devNo=0;
  uint16_t addr=0;
  int ntokens = sscanf(what.c_str(), "d%hua%hu", &devNo, &addr);
  if (ntokens!=2) {
    throw string("CMxDCRCBus::parseAddress Failed to parse address string");
  }

  return make_pair(devNo, addr);
}


/**!
 *
 */
bool CMxDCRCBus::responseIndicatesError(uint16_t datum)
{
  return (((datum & RCSTAT_ADDRCOLLISION) | (datum & RCSTAT_NORESPONSE))!=0);
}

/**!
 *
 */
std::string CMxDCRCBus::convertResponseToErrorString(uint16_t datum) 
{

  string message;

  // at the moment there are only two error flags
  if (datum & RCSTAT_ADDRCOLLISION) {
    message = "ERROR - Address collision during last RC-bus operation";  
    message += " : code=";
    message += to_string(datum);
  } else if (datum & RCSTAT_NORESPONSE) {
    message = "ERROR - No response during last RC-bus operation"; 
    message += " : code=";
    message += to_string(datum);
  } else {
    message = "ERROR - Unknown error code returned from last RC-bus operation";
    message += " : code=";
    message += to_string(datum);
  }

  return message;
}


void CMxDCRCBus::addParameterWrite(CVMUSBReadoutList& list,
                                   std::pair<uint16_t,uint16_t> addresses,
                                   uint16_t value)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  list.addWrite16(base+RCModNum, VMEAMod::a32UserData, addresses.first);
  list.addWrite16(base+RCOpCode, VMEAMod::a32UserData, RCOP_WRITEDATA);
  list.addWrite16(base+RCAddr,   VMEAMod::a32UserData, addresses.second);
  list.addWrite16(base+RCData,   VMEAMod::a32UserData, value);
}

void CMxDCRCBus::addParameterRead(CVMUSBReadoutList& list,
                                   std::pair<uint16_t,uint16_t> addresses)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  list.addWrite16(base+RCModNum, VMEAMod::a32UserData, addresses.first);
  list.addWrite16(base+RCOpCode, VMEAMod::a32UserData, RCOP_READDATA);
  list.addWrite16(base+RCAddr,   VMEAMod::a32UserData, addresses.second);
  list.addWrite16(base+RCData,   VMEAMod::a32UserData, 0);
}
