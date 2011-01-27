/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:


     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CRingTextItem.h"
#include <string.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////
//
//   Constructors and other canonical member functions.
//

/*!
   Construct a ring item that contains text strings.
   The item will have a timestamp of 'now' and an offset time of 0.
   \param type       - the ring item type. This should be 
                        PACKET_TYPES or MONITORED_VARIABLES.
   \param theStrings - the set of strings in the ring buffer item.

*/
CRingTextItem::CRingTextItem(uint16_t type, vector<string> theStrings) :
  CRingItem(type, bodySize(theStrings))
{
  init();			// Sets up m_pItem and cursor.
  copyStrings(theStrings);

  m_pItem->s_timeOffset = 0;
  time(&(m_pItem->s_timestamp));
}
/*!
  Construct a ring buffer, but this time provide actual values for the
  time offset and time stamp.
  \param type    - Type of ring item... see the previous constructor.
  \param strings - The strings to put in the buffer.
  \param offsetTime - The time in to the run at which this is being inserted.
  \param timestamp  - The absolute time when this is being created.

*/
CRingTextItem::CRingTextItem(uint16_t       type,
			     vector<string> strings,
			     uint32_t       offsetTime,
			     time_t         timestamp) :
  CRingItem(type, bodySize(strings))
{
  init();
  copyStrings(strings);

  m_pItem->s_timeOffset = offsetTime;
  m_pItem->s_timestamp  = timestamp;
  
}
/*!
   Construct from an undifferentiated ring item.
   If the ring item does not have a type that is consistent with
   a text item type that is a strong error.
   \param rhs  - The ring item from which this is constructed.
   
   \throw bad_cast - if rhs is not a text ring item.
*/
CRingTextItem::CRingTextItem(const CRingItem& rhs) throw(bad_cast): 
  CRingItem(rhs)
{
  if (!validType()) throw bad_cast();

  init();
}

/*!
  Copy construction.  
  \param rhs - the item we are being copied from.
*/
CRingTextItem::CRingTextItem(const CRingTextItem& rhs) :
  CRingItem(rhs)
{
  init();
}

/*!
  Destructor just chains to base class.
*/
CRingTextItem::~CRingTextItem()
{}

/*!
  Assignment
  \param rhs - the item being assigned to this.
  \return CRingTextItem&
  \retval *this
*/
CRingTextItem&
CRingTextItem::operator=(const CRingTextItem& rhs)
{
  if (this != &rhs) {
    CRingItem::operator=(rhs);
    init();
  }
  return *this;
}
/*!
  Comparison for equality.  No real point in comparing the item pointers.
  unless this == &rhs they will always differ.
  \param rhs - the item being compared to *this
  \return int
  \retval 0        - Not equal
  \retval nonzero  - equal.
*/
int
CRingTextItem::operator==(const CRingTextItem& rhs) const
{
  return CRingItem::operator==(rhs);
}
/*!
  Comparison for inequality.
  \param rhs      - the item being compared to *this.
  \retval 0       - Items are not inequal
  \retval nonzero - items are inequal

  \note My stilted English is because C++ allows perverse cases where 
  a == b  does not necesarily imply !(a != b) and vica versa. In fact, these
  operators can be defined in  such a way that they have nothing whatever to do
  with comparison (just as ostream::operator<< has nothing to do with 
  shifting). however  my definition is sensible in that a == b is the logical converse of 
  a != b, and vicaversa, and these operators really do compare.
*/
int 
CRingTextItem::operator!=(const CRingTextItem& rhs) const
{
  return !(*this == rhs);
}


///////////////////////////////////////////////////////////////////////////////////////
//
// accessors (both selectors and mutators).
//

/*!
    \return vector<string>
    \retval The strings that were put in the item unpacked into elements of the vector.
*/
vector<string>
CRingTextItem::getStrings() const
{
  vector<string> result;
  const char*     pNextString = m_pItem->s_strings;

  for (int i = 0; i < m_pItem->s_stringCount; i++) {
    string aString = pNextString;
    pNextString   += aString.size() + 1;  // +1 for the trailing null.

    result.push_back(aString);

  }

  return result;
}
/*!
   Modify the buffered value of the run time offset.  This may be done if you use the
   simplified constuctor and only later figure out what the run time offset actually is.
   \param offset
*/
void
CRingTextItem::setTimeOffset(uint32_t offset)
{
  m_pItem->s_timeOffset = offset;
}
/*!
   \return uint32_t
   \retval the time offset value.
*/
uint32_t
CRingTextItem::getTimeOffset() const
{
  return m_pItem->s_timeOffset;
}
/*!
   Set a new value for the timestamp of the item.
*/
void
CRingTextItem::setTimestamp(time_t stamp)
{
  m_pItem->s_timestamp = stamp;
}
/*!
   \return time_t
   \retval absolute timestamp from the buffer.

*/
time_t
CRingTextItem::getTimestamp() const
{
  return m_pItem->s_timestamp;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities.
//

/*
**  Compute the size of the body of a text item buffer.  This is the sum of the sizes of the string
**  +1 for each string to allow for the null termination byte, + sizeof(TextItem) 
**   - sizeof(RingItemHeader) - sizeof(char) 
** (this last - sizeof(char) is the dummy s_strings[] array size.
*/
size_t 
CRingTextItem::bodySize(vector<string> strings) const
{
  size_t result = sizeof(TextItem) - sizeof(RingItemHeader) - sizeof(char);
  for (int i=0; i < strings.size(); i++) {
    result += strings[i].size() + 1;
  }
  return result;
}
/*
**  Returns true if the type of the current item is a valid text item.
**
*/
bool
CRingTextItem::validType() const
{
  uint32_t t = type();
  return ((t == PACKET_TYPES)               ||
	  (t == MONITORED_VARIABLES));
}
/*
** Copies a set of strings into the item's 
** s_scalers region.   Each string is followed by a 
** null.  
**   When done the cursor is updated to point past the item.
**   When done, s_stringCount is updated to the number of strings.
*/
void
CRingTextItem::copyStrings(vector<string> strings)
{
  m_pItem->s_stringCount = strings.size();
  char* p                = m_pItem->s_strings;
  for (int i = 0; i < strings.size(); i++) {
    strcpy(p, strings[i].c_str());
    p += strings[i].size() + 1;
  }
  setBodyCursor(p);
  updateSize();
}
/*
** Initialize m_pItem from the underlying item.
*/
void
CRingTextItem::init()
{
  m_pItem = reinterpret_cast<pTextItem>(getItemPointer());
}
