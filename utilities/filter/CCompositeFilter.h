

#ifndef CCOMPOSITEFILTER_H
#define CCOMPOSITEFILTER_H

#include <vector> 
#include "CFilter.h"

class CCompositeFilter : public CFilter
{
  public:
    // Basic typedefs for use with the class
    typedef std::vector<CFilter*> FilterContainer;
    typedef FilterContainer::iterator iterator;
    typedef FilterContainer::const_iterator const_iterator;

  public:
    FilterContainer m_filter; //!< The list of filters

  public:
    // Canonical methods
    CCompositeFilter();
    CCompositeFilter(const CCompositeFilter&);
    CCompositeFilter& operator=(const CCompositeFilter&);

    // Filter registration
    void registerFilter(const CFilter* filter);

    // Virtual copy constructor 
    CCompositeFilter* clone() const;

    virtual ~CCompositeFilter();

    // Handlers
    virtual const CRingItem* handleRingItem(const CRingItem* item);
    virtual const CRingItem* handleStateChangeItem(const CRingStateChangeItem* item);
    virtual const CRingItem* handleScalerItem(const CRingScalerItem* item);
    virtual const CRingItem* handleTextItem(const CRingTextItem* item);
    virtual const CRingItem* handlePhysicsEventItem(const CPhysicsEventItem* item);
    virtual const CRingItem* handlePhysicsEventCountItem(const CRingPhysicsEventCountItem* item);
    virtual const CRingItem* handleFragmentItem(const CRingFragmentItem* item);

    // iterator interface
    iterator begin() { return m_filter.begin(); }
    const_iterator begin() const { return m_filter.begin(); }

    iterator end() { return m_filter.end(); }
    const_iterator end() const { return m_filter.end(); }

    void clear() { clearFilters() ;}
    size_t size() const { m_filter.size();}

  private:
    // Free all dynamically allocated memory resize container 
    void clearFilters();

};

#endif
