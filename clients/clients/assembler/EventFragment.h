#ifndef __EVENTFRAGMENT_H
#define __EVENTFRAGMENT_H

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


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


#ifndef __STL_VECTOR
#include <vector>     // Must also pull in size_t.
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


/*!
   Base class for all event fragments.  This is not
   abstract, extensions of this class add functionality
   rather than polymorphically implementing existing interfaces.
*/
class EventFragment
{
private:
  uint16_t  m_node;
  uint16_t  m_type;
  std::vector<uint16_t> m_body;

public:
  EventFragment(uint16_t node,
		uint16_t type,
		void*    body
		size_t   words);
  EventFragment(uint16_t node,
		uint16_t type,
		std::vector<uint16_t> body);
  
  uint16_t node() const;
  uint16_t type() const;
  std::vector<uint16_t> body() const;
  size_t   size() const;
  uint16_t& operator[](size_t index);


protected:
  // utilities:


  static uint16_t extractType(uint16_t* rawBuffer);
  static uint32_t extractSize(uint16_t* rawBuffer);
  static uint16_t extractNode(uint16_t* rawBuffer);
  static uint16_t extractSsig(uint16_t* rawBuffer);
  static uint32_t extractLsig(uint16_t* rawBuffer);
  static uint16_t extractEntityCount(uint16_t* rawBufer);
  static uint16_t* bodyPointer(uint16_t* rawBuffer);




  static uint32_t tohl(uint32_t datum, uint32_t lsig);
  static uint16_t tohs(uint16_t datum, uint16_t ssig);

  static uint32_t getLongword(uint16_t* buffer,
			      size_t wordOffset,
			      uint32_t lsig);
  static uint16_t getWord(uint16_t* buffer,
			  size_t wordOffset,
			  uint16_t ssig);
		     
			      


};


#endif
