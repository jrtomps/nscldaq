

#ifndef CFILTER_H
#define CFILTER_H

#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CPhysicsEventItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

/**! \class CFilter
  This class is a base class for all filter objects. It provides a 
  default definitions for all of its methods that does nothing more
  than return the pointer passed as an argument. For this reason, one
  could consider this as a transparent filter that can be inserted into
  the data stream.

  All derived methods must return a newly allocated CRingItem. There is
  a one-to-one relationship between object input to the filter and objects
  output from the filter. The user is not responsible for delete the 
  object passed into each method. In fact, doing so will cause a segmentation
  fault.
*/
class CFilter 
{
  public:
  
    // Virtual base class destructor
    virtual ~CFilter() {}

    // Virtual constructor
    virtual CFilter* clone() const { return new CFilter();}
  
    // The default handlers
    virtual CRingItem* handleRingItem(CRingItem* pItem) 
    {
       return pItem;
    }

    virtual CRingItem* handleStateChangeItem(CRingStateChangeItem* pItem) 
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleScalerItem(CRingScalerItem* pItem) 
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleTextItem(CRingTextItem* pItem) 
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* pItem) 
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* 
    handlePhysicsEventCountItem(CRingPhysicsEventCountItem* pItem) 
    {
       return static_cast<CRingItem*>(pItem);
    }

    virtual CRingItem* handleFragmentItem(CRingFragmentItem* pItem)
    {
       return static_cast<CRingItem*>(pItem);
    }

};

#endif
