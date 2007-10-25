#ifndef __NODESCOREBOARD_H
#define __NODESCOREBOARD_H
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
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

/*!
 *   NodeScoreboard is responsible for keeping track of the
 * nodes which have provided fragments to an event that is being
 * assembled.   Some compromises were made to support performance:
 * - Nodes required are held in a uint32_t mask.  This limits the
 *   number of nodes that can participate in assembly to 32.
 * - Node ids get mapped to bit numbers in this mask via a rather large array.
 * - The node id array is statically shared amongst all the assemblies (e.g.
 *   the event assembler cannot assemble events from more than one set of nodes
 *   in an instance (this is not a real restriction.. just run multiple copies
 *   of the program.
 * - Exceeding these limits results in an exception being thrown.
 */
class NodeScoreboard
{
	// class level data
	
	static uint16_t m_nodeToBit[0x10000];  // Maps node ids -> mask bits.
	static uint32_t m_nodeNextBit;         // Next bit to use for mapping.
	static uint32_t m_requiredNodes;       // Nodes needed to complete an event.
	
private:
	// Member data
	
	uint32_t m_contributedNodes;            // Nodes seen so far.
private:
public:
	
	// Canonical member functions:
	
	NodeScoreboard();
	NodeScoreboard(const NodeScoreboard& rhs);
	NodeScoreboard& operator=(const NodeScoreboard& rhs);
	int operator==(const NodeScoreboard& rhs) const;
	int operator!=(const NodeScoreboard& rhs) const;
	
	// class level member functions
public:
	static void neededNodes(std::vector<uint16_t> nodes);

	
	// object level member functions:
public:
	void addNode(uint16_t node);
	bool isComplete() const;
	void clear();

};

#endif /*NODESCOREBOARD_H_*/
