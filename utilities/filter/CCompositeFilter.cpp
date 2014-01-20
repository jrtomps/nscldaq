#include <iostream>
#include <iomanip>
#include <CCompositeFilter.h>

/**! The default constructor
  Does nothing but initialize an empty vector of filters
*/ 
CCompositeFilter::CCompositeFilter()
  : m_filter()
{}

/**! The copy constructor
* Performs a deep copy of the filters. Clones of each of the filters
* in the target composite are made.
*
* \param rhs the filter to copy
*/
CCompositeFilter::CCompositeFilter(const CCompositeFilter& rhs)
  : m_filter()
{
  const_iterator it = rhs.begin();
  const_iterator itend = rhs.end();

  while (it!=itend) {
    m_filter.push_back((*it)->clone());
    ++it;
  } 
}

/**! Assignment 

  Performs a deep copy of the target. The target's state is copied 
  entirely prior to assignment. Following this operation, this will not own pointers 
  to the same objects as were pointed to by the target's pointers. Instead, 
  the various pointers will refer to copies of the objects pointed to by the 
  target's pointers.

  \param rhs the object to copy 
  \return a reference to this after the copy operation
*/
CCompositeFilter& CCompositeFilter::operator=(const CCompositeFilter& rhs)
{
  if (this != &rhs) {
    const_iterator it = rhs.begin();
    const_iterator itend = rhs.end();

    // First fill a vector with all the newly
    // copied filters. If this fails, then we know the 
    // original filters were not modified.
    FilterContainer copy;
    while (it!=itend) {
      copy.push_back((*it)->clone());
      ++it;
    } 

    // We made it here so clear our container
    // and copy all of the new pointer values
    clearFilters();
    m_filter = copy;
  }

  return *this;
}

/**! Destructor
  1. Frees all dynamically allocated objects
  2. Clears the container holding the pointers to those objects.
*/
CCompositeFilter::~CCompositeFilter()
{
  clearFilters();
}

/**! Virtual copy constructor
  Returns a dynamically allocated object whose state is a copy of this.
  Ownership of the returned object belongs to the caller. The semantics 
  of the copy operation for this class is described in the
  CCompositeFilter::CCompositeFilter(const CCompositeFilter&) method.

  \return a pointer to the new dynamically allocated object
*/
CCompositeFilter* CCompositeFilter::clone() const 
{
  return new CCompositeFilter(*this);
}

/**! Append a filter to the container
*
   By registering a filter to this, a copy of the filter is actually
   registered. Filters are appended to the back of the container every
   time this method is called. As a result, the order of registration
   is preserved at execution time.

  \param filter a template of the filter to register
*/
void CCompositeFilter::registerFilter(const CFilter* filter)
{
  m_filter.push_back(filter->clone());
}

/**! Handle a generic ring item
    This is handler effectively just iterates through the set of 
    registered filters and calls their respective handleRingItem(const CRingItem*)
    methods. Memory is managed during the iterations such that each subsequent 
    filter receives as input the output of the previous. It is possible for a filter
    to return a pointer to the same object as was passed it and this is handled properly.
    
  \param item the ring item to process
  \return a pointer to the output of the last filter registered to the composite
*/  
const CRingItem* CCompositeFilter::handleRingItem(const CRingItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  while (it!=itend) {

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleRingItem(pItem0);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle state change items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleStateChangeItem(const CRingStateChangleItem*) is 
   called.
*/
const CRingItem* CCompositeFilter::handleStateChangeItem(const CRingStateChangeItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  const CRingStateChangeItem* state_item = 0;
  while (it!=itend) {

    state_item = static_cast<const CRingStateChangeItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleStateChangeItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle scaler items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleScalerItem(const CScalerItem*) is 
   called.
*/
const CRingItem* CCompositeFilter::handleScalerItem(const CRingScalerItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  const CRingScalerItem* state_item = 0;
  while (it!=itend) {

    state_item = static_cast<const CRingScalerItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleScalerItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle text items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleTextItem(const CTextItem*) is 
   called.
*/
const CRingItem* CCompositeFilter::handleTextItem(const CRingTextItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  const CRingTextItem* state_item = 0;
  while (it!=itend) {

    state_item = static_cast<const CRingTextItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleTextItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle physics event items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handlePhysicsEventItem(const CPhysicsEventItem*) is 
   called.
*/
const CRingItem* CCompositeFilter::handlePhysicsEventItem(const CPhysicsEventItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  const CPhysicsEventItem* state_item = 0;
  while (it!=itend) {

    state_item = static_cast<const CPhysicsEventItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handlePhysicsEventItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle physics event items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handlePhysicsEventCountItem(const CRingPhysicsEventCountItem*) is 
   called.
*/
const CRingItem* CCompositeFilter::handlePhysicsEventCountItem(const CRingPhysicsEventCountItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  const CRingPhysicsEventCountItem* state_item = 0;
  while (it!=itend) {

    state_item = static_cast<const CRingPhysicsEventCountItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handlePhysicsEventCountItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle physics event items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleFragmentItem(const CFragmentItem*) is 
   called.
*/
const CRingItem* CCompositeFilter::handleFragmentItem(const CRingFragmentItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  const CRingItem* pItem0=item;
  const CRingItem* pItem1=pItem0;
  const CRingFragmentItem* state_item = 0;
  while (it!=itend) {

    state_item = static_cast<const CRingFragmentItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleFragmentItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Clear and resize filter container
    Frees all memory pointed to by pointers in the container and
    then resizes the container to zero size.
*/
void CCompositeFilter::clearFilters()
{
  iterator it    = begin();
  iterator itend = end();

  while (it!=itend) {
    delete *it;
    ++it;
  }
  m_filter.clear();
}
