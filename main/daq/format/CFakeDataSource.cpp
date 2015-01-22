
#include <CFakeDataSource.h>
#include <CRingItem.h>

CRingItem*
CFakeDataSource::getItem()
{
  return new CRingItem(1);
}
