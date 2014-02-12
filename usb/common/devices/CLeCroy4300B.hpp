
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include "CConfiguration.h"
#include "CamacSlotLimits.h"

template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>::CLeCroy4300B() 
   : m_pConfig(0)
{
}


template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>::CLeCroy4300B(const CLeCroy4300B& rhs)
   : CReadoutHardwareT<Controller,RdoList>(rhs)
{
    //    if (rhs.m_pConfig) {
    //        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
    //    }

}

template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>& 
CLeCroy4300B<Controller,RdoList>::operator=(const CLeCroy4300B& rhs)
{
    return *this;
}

template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>::~CLeCroy4300B()
{
//    if(m_pConfig) delete m_pConfig;
}


template<class Controller, class RdoList>
void  CLeCroy4300B<Controller,RdoList>::onAttach(CReadoutModule& config)
{
//    std::cout << "Attaching CLeCroy4300B" << std::endl;
    m_pConfig = &config;
    m_pConfig->addParameter("-slot",
                            CConfigurableObject::isInteger,
                            &SlotLimits,
                            "1");

    m_pConfig->addIntListParameter("-pedestals",16);

    m_pConfig->addIntegerParameter("-cmdregister",0);
    m_pConfig->addBooleanParameter("-camacclear",false);
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::Initialize(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    std::vector<int> peds = m_pConfig->getIntegerList("-pedestals");
    uint16_t q=0;

    // clear
    execClear(controller);

    writePedestals(controller,peds);

    setCommandRegister(controller);

}


template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::addReadoutList(RdoList& list)
{
    if (m_pConfig->getBoolParameter("-camacclear")) {
      addClear(list);
    }
}

template<class Controller, class RdoList> 
void 
CLeCroy4300B<Controller,RdoList>::writePedestals(Controller& controller, 
                                                 const std::vector<int>& peds)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    unsigned int nPeds = peds.size();
    if (nPeds>16) {
        std::cerr << "!!! CLeCroy4300B configured with more than 16 specifiers"
                  << std::endl;
        std::cerr << "    Only the first sixteen values will be used. " 
                  << std::endl;
    }

    int status = 0;
    for (int i=0; i<nPeds; ++i) {
        uint16_t qx = 0;
        uint16_t ped_i = static_cast<uint16_t>(peds.at(i));

        do {
            // clear to enable for input
            execClear(controller);

            // write the pedestal to chan i
            status = controller.simpleWrite16(slot,i,17,ped_i,qx);
            handleStatus(status);
            handleQ(qx);
        } while (qx==0);
    }
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::execClear(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    // clear to enable for input
    uint16_t qx;
    int status = controller.simpleControl(slot,0,9, qx);
    handleStatus(status);
    handleQ(qx);

}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::addClear(RdoList& list)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    // clear to enable for input
    list.addControl(slot,0,9);
}


template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::setCommandRegister(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    int cmdreg = m_pConfig->getIntegerParameter("-cmdregister"); 

    // Check that user has provided a value been specified.
    // If not, complain and return.
    if (cmdreg==0) {
        std::cerr << "!!! CLeCroy4300B slot " << slot;
        std::cerr << " has no command register value specified!" << std::endl;
        return;
    }

    uint16_t qx;
    uint16_t reg=0;

    // Write command register
    int status = controller.simpleWrite16(slot,0,16,cmdreg,qx);
    handleStatus(status);
    handleQ(qx);

    // wait a little
    usleep(10);

    // Read back the command register
    status = controller.simpleRead16(slot,0,0,reg,qx);
    handleStatus(status);
    handleQ(qx);

    // Is it the same
    if ( cmdreg != reg) {
        std::cerr << "!!! CLeCroy2551::Initialize(Controller&) Failed to read back" << std::endl;
        std::cerr << "    same value that was previously written." << std::endl;
    }

}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::handleStatus(int status)
{
    if (status<0) {
        std::cout << "Controller operation returned with error " << status << std::endl;
    }
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::handleQ(uint16_t q)
{
    if (q!=1) {
        std::cout << "Controller operation returned with q!=1" << std::endl;
    }
}


