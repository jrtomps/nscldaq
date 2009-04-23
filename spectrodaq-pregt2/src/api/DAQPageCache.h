#ifndef DAQPAGECACHE_H
#define DAQPAGECACHE_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQCONFIG_H
#include <daqconfig.h>
#endif

#ifndef DAQOBJECT_H
#include <DAQObject.h>
#endif

#ifndef DAQ_SYNCHRONIZABLE_H
#include <Synchronizable.h>
#endif

#ifndef DAQOBJECTARRAY_H
#include <DAQObjectArray.h>
#endif

/**
* @class DAQPageCache
* @brief DAQPageCache class.
*
* The DAQPageCache class for caching buffer pages by 
* DAQ allocators.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DAQPageCache : public DAQObject, public Synchronizable {
  public: 
    DAQPageCache();           // Default constructor 
    DAQPageCache(const DAQPageCache&);   // Copy constructor 
    virtual ~DAQPageCache();          // Destructor
    DAQPageCache& operator=(const DAQPageCache&);  // Assignment
    int get(DAQObjectArray&,long); // Get pages from this cache
    int getAll(DAQObjectArray&); // Get all pages from this cache
    bool put(DAQObjectArray&);  // Add pages to this cache
    bool isEmpty(); // Check if the cache is empty

  protected:
    void copyToThis(const DAQPageCache&);

    DAQObjectArray my_cache;
    int my_head;
};

#endif
