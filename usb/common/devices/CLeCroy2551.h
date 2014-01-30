
#ifndef CLECROY2551_H
#define CLECROY2551_H

#include "CReadoutHardwareT.h"
#include "CCBD8210CrateController.h"
#include "CCBD8210ReadoutList.h"
#include <CReadoutModule.h>
#include "CCamacBranchException.h"
//class CReadoutModule;

namespace csr
{

template<class Controller, class RdoList> class CLeCroy2551;

typedef 
CLeCroy2551<CCBD8210CrateController,CCBD8210ReadoutList> CCBD8210LeCroy2551;

template<class Controller, class RdoList>
class CLeCroy2551 : public CReadoutHardwareT<Controller,RdoList>
{

    private:
    CReadoutModule* m_pConfig;

    public:
    CLeCroy2551();
    CLeCroy2551(const CLeCroy2551& rhs);
    CLeCroy2551& operator=(const CLeCroy2551& rhs);
    virtual ~CLeCroy2551();


    void onAttach(CReadoutModule& config);
    void Initialize(Controller& config);
    void addReadoutList(RdoList& config);
    void onEndRun(Controller& config) {}

    CLeCroy2551* clone() const {
        return new CLeCroy2551(*this);
    }

    private:
    void addClearAll(RdoList& list);
    void addRead(RdoList& list,int chan) throw(CInvalidA);
//    void addReadAndClear(RdoList& list,int a);
//    void addTestLAM(RdoList& list);
    void addReadAll(RdoList& list);
};

}
#include "CLeCroy2551.hpp"

#endif
