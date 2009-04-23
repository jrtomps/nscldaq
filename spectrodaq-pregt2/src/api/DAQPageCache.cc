/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#ifndef DAQOBJECT_H
#include <DAQObject.h>
#endif

#ifndef DAQPAGECACHE_H
#include <DAQPageCache.h>
#endif

#ifndef DAQBUFFERPAGE_H
#include <DAQBufferPage.h>
#endif

/*==============================================================*/
/** @fn DAQPageCache::DAQPageCache()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
DAQPageCache::DAQPageCache() {
  my_head = -1;
}

/*==============================================================*/
/** @fn DAQPageCache::DAQPageCache(const DAQPageCache& rCache)
* @brief Copy constructor.
*
* Copy constructor.
*
* @param rArry The other cache.
* @return this
*/                                                             
DAQPageCache::DAQPageCache(const DAQPageCache& rCache) {
  my_cache.clear();
  my_head = -1;
  copyToThis(rCache);
}

/*==============================================================*/
/** @fn DAQObjectArray::~DAQObjectArray() 
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
DAQPageCache::~DAQPageCache() {
  my_cache.clear();
  my_head = -1;
}

/*==============================================================*/
/** @fn int DAQPageCache::get(DAQObjectArray& rArry,long aBytes) 
* @brief Get pages from this cache.
*
* Get pages from this cache.  The parameter aBytes indicates 
* the number of bytes required.
*
* @param rArry.  Output.  The pages requested.
* @param aBytes.  The number of bytes required.
* @return The number of bytes returned.
*/                                                             
int DAQPageCache::get(DAQObjectArray& rArry,long aBytes) {
  DAQBufferPage *pPage = NULL;
  int pgsiz = 0;
  int pos = 0;

  rArry.clear();
  if (aBytes <= 0) return(0);

  // If there are some pages then we can return some pages
  if ((my_head >= 0)&&(my_head < my_cache.length)) {
    pPage = (DAQBufferPage*)(my_cache.elements[my_head]);
    pgsiz = pPage->GetLength();
    int npg = aBytes / pgsiz;
    if ((aBytes % pgsiz) > 0) npg++; // for the remainder.
    rArry.renew(npg); // Set to the number of pages

    while ((my_head < my_cache.length)&&(pos < npg)) {
      rArry.elements[pos] = my_cache.elements[my_head];
      pos++;
      my_head++;
    }

    // Remove unused slots
    if (pos < npg) rArry.resize(pos);
  } 

  // If there are no pages reset the cache
  if (my_head >= my_cache.length) {
    my_cache.clear();
    my_head = -1;
  }

  // If the total byte count for all pages is greater
  // than that requested, then return only the request count. 
  int bcnt = pos * pgsiz;
  return((bcnt > aBytes) ? aBytes : bcnt); 
}

/*==============================================================*/
/** @fn int DAQPageCache::getAll(DAQObjectArray& rArry) 
* @brief Get all the pages from this cache.
*
* Get all available pages from this cache.  
*
* @param rArry.  Output.  The pages requested.
* @return The number of bytes returned.
*/                                                             
int DAQPageCache::getAll(DAQObjectArray& rArry) {
  rArry.clear();
  if (my_head < 0) return(0);

  // Compute the number of pages available
  int avail = my_cache.length - my_head;
  rArry.renew(avail);

  // Put the pages in the array.
  int pos = 0;
  while (my_head < my_cache.length) {
    rArry.elements[pos] = my_cache.elements[my_head];
    pos++;
    my_head++;
  }

  // Reset the cache
  my_cache.clear();
  my_head = -1;

  // Compute the byte count
  DAQBufferPage *pPage = (DAQBufferPage*)(rArry.elements[0]);
  int pgsiz = pPage->GetLength();

  // Return the total number of bytes
  return(avail * pgsiz);
}

/*==============================================================*/
/** @fn bool DAQPageCache::put(DAQObjectArray& rArry) 
* @brief Put pages in this cache.
*
* Put pages in this cache.  The cache must be empty
* to add more pages.  This requirement is meant to ensure
* that cached pages are in the order specified by the
* server.
*
* @param rArry.  The pages to add.
* @return True if pages were added to the cache.
*/                                                             
bool DAQPageCache::put(DAQObjectArray& rArry) {
  if (my_head >= 0) return(false);
  if (rArry.length <= 0) return(false);

  int pos = 0;
  my_cache.renew(rArry.length);
  for (int i = 0; i < rArry.length; i++) {
    // Do not add NULL pages
    if (rArry.elements[i] != NULL) {
      my_cache.elements[pos] = rArry.elements[i];
      pos++;
    }
  }

  if (pos > 0) {
    my_head = 0;
    if (pos != my_cache.length) my_cache.resize(pos);
  } else {
    my_cache.clear();
    my_head = -1;
  }

  return((pos > 0) ? true : false);
}

/*==============================================================*/
/** @fn bool DAQPageCache::isEmpty() 
* @brief Check if the cache is empty.
*
* Return true if the cache is empty and false otherwise.
*
* @param None
* @return True if the cache is empty
*/                                                             
bool DAQPageCache::isEmpty() {
  return ((my_head >= 0) ? false : true);
}

/*==============================================================*/
/** @fn DAQPageCache& DAQPageCache::operator=(const DAQPageCache& rCache) 
* @brief Assign a cache to this one.
*
* Assign another DAQPageCache to this one.
*
* @param rCache The other cache.
* @return A reference to this.
*/                                                             
DAQPageCache& DAQPageCache::operator=(const DAQPageCache& rCache) {
  copyToThis(rCache);
  return(*this);
} 

/*==============================================================*/
/** @fn void DAQObjectArray::copyToThis(const DAQPageCache& rCache) 
* @brief Assign a cache to this one.
*
* Assign another DAQPageCache to this one.
*
* @param rCache The other cache.
* @return None
*/                                                             
void DAQPageCache::copyToThis(const DAQPageCache& rCache) {
  my_cache.clear();
  my_cache = rCache.my_cache;
  my_head = rCache.my_head;
} 
