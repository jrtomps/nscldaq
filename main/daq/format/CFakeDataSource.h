
#ifndef CFAKEDATASOURCE_H
#define CFAKEDATASOURCE_H

#include <CDataSource.h>

class CRingItem;

/**! \brief A fake source that continually produces begin run items
 *
 */
class CFakeDataSource : public CDataSource
{
  /**! \brief Generates an empty ring item of type 1 */
  CRingItem* getItem();
};

#endif
