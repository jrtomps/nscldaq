

#include "CMediator.h"

#include <iostream>
#include <CDataSource.h>
#include <CDataSink.h>
#include <CFilter.h>

#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CPhysicsEventItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

/**! Constructor

  Constructs the mediator object. This object owns its referenced members.

  \param source a pointer to a CDataSource
  \param filter a pointer to a CFilter
  \param sink a pointer to a CDataSink

*/
CMediator::CMediator(CDataSource* source, CFilter* filter, CDataSink* sink)
: m_pSource(source),
  m_pFilter(filter),
  m_pSink(sink),
  m_nToProcess(-1),
  m_nToSkip(-1)
{}

/**! Destructor
  Delete the owned objects. It is possible that these were never initialized,
  so it is important to verify that they point to something other than NULL.

  Following a delete the pointers are reset to 0. This protects against double
  frees when a user decides to call an objects destructor explicitly and then
  it goes out of scope.
*/
CMediator::~CMediator() 
{

  if (m_pSource!=0) {
    delete m_pSource;
    m_pSource = 0;
  }

  if (m_pFilter!=0) {
    delete m_pFilter;
    m_pFilter = 0;
  }

  if (m_pSink!=0) {
    delete m_pSink;
    m_pSink = 0;
  }

  m_nToProcess = -1;
  m_nToSkip = -1;

}

/**! The main loop
  This is the workhorse of the application. Items are retrieved 
  from the source, passed to the filter, and then the item returned 
  by the filter is sent to the
  sink. During each iteration through this process, the item retrieved
  from the source and the filter are properly managed.
*/
void CMediator::mainLoop()
{
  const CRingItem* item=0;
  
  // Dereference our pointers before entering
  // the main loop
  CDataSource& source = *m_pSource;
  CDataSink& sink = *m_pSink;

  // Set up some counters
  int tot_iter=0, proc_iter=0, nskip=0, nprocess=0;

  nskip = m_nToSkip;
  nprocess = m_nToProcess;

  while (1) {
    
    // Check if all has been processed that was requested
    if (proc_iter>=nprocess && nprocess>=0) {
      break;
    }
  
    // Get a new item
    // Exit if the item returned is null
    item = source.getItem();
    if (item==0) {
      break;
    }

    // only process if we have skipped the requested number
    if (tot_iter>=nskip) {

      // handle the item and get a pointer to 
      // to the new item
      const CRingItem* new_item = handleItem(item);

      // Send the new item on to the sink
      sink.putItem(*new_item);

      // It is possible that the filter did nothing more than
      // return a pointer to the same object. Therefore, only
      // delete the returned item if it is different.
      if ( new_item != item ) {
        delete new_item;
      } 
      
      // Increment the number processed
      ++proc_iter;
    }
    

    // delete original item
    // THE FILTER MUST NOT HAVE DELETED THE OBJECT PASSED IT!!!
    delete item;

    // Increment our counter
    ++tot_iter;
  }

}



const CRingItem* CMediator::handleItem(const CRingItem* item)
{
  // initial pointer to filtered item
  const CRingItem* fitem = item;
  switch(item->type()) {

    // State change items
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      fitem = m_pFilter->handleStateChangeItem(static_cast<const CRingStateChangeItem*>(item));
      break;

      // Documentation items
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
      fitem = m_pFilter->handleTextItem(static_cast<const CRingTextItem*>(item));
      break;

      // Scaler items
    case PERIODIC_SCALERS:
      fitem = m_pFilter->handleScalerItem(static_cast<const CRingScalerItem*>(item));
      break;

      // Physics event item
    case PHYSICS_EVENT:
      fitem = m_pFilter->handlePhysicsEventItem(static_cast<const CPhysicsEventItem*>(item));
      break;

      // Physics event count
    case PHYSICS_EVENT_COUNT:
      fitem = m_pFilter->handlePhysicsEventCountItem(static_cast<const CRingPhysicsEventCountItem*>(item));
      break;

      // Event builder fragment handlers
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
      fitem = m_pFilter->handleFragmentItem(static_cast<const CRingFragmentItem*>(item));
      break;

      // Handle any other generic ring item...this can be 
      // the hook for handling user-defined items
    default:
      fitem = m_pFilter->handleRingItem(item);
      break;
  }

  return fitem;
}
