
#ifndef CULMTRIGGER_H
#define CULMTRIGGER_H

#include "CReadoutHardwareT.h"
#include "CCamacBranchException.h"
#include <CReadoutModule.h>
//class CReadoutModule;
#include <iosfwd>

template<class Controller, class RdoList> class CULMTrigger;

template<class Controller, class RdoList>
class CULMTrigger : public CReadoutHardwareT<Controller,RdoList>
{

    public:
    enum TStampMode { EXTERNALCLOCK=1, EXTERNALLATCH=2 }; 

    private:
    CReadoutModule* m_pConfig;

    public:
    CULMTrigger();
    CULMTrigger(const CULMTrigger& rhs);
    CULMTrigger& operator=(const CULMTrigger& rhs);
    virtual ~CULMTrigger();


    void onAttach(CReadoutModule& config);
    void Initialize(Controller& config);
    void addReadoutList(RdoList& config);
    void onEndRun(Controller& config);

    CULMTrigger* clone() const {
        return new CULMTrigger(*this);
    }

    private:
    static bool isBooted;
    static bool validFirmwareFile(std::string name, std::string value, void* arg);

    int loadFirmware(Controller& controller, const std::string& filename);
    int loadFirmware2(Controller& controller, const std::string& filename);
    void SendChar(Controller& ctlr, unsigned char c);
    unsigned int determineNBytes(Controller& controller, std::ifstream& file);

    bool isConfigured(Controller& ctlr);

    int doClear(Controller& ctlr);
    
    // Setters
    int setGo(Controller& ctlr, bool onoff);
    int setEnable(Controller& ctlr, uint16_t bits);
    
    // This is the same as Daniel's "Select" method
    int setTStampMode(Controller& ctlr, TStampMode mode);
    
    // Getters
    int getGo(Controller& ctlr, bool& onoff);
    int getTStampMode(Controller& ctlr, TStampMode& mode);

}; // class ULMTrigger

#include "CULMTrigger.hpp"

#endif
