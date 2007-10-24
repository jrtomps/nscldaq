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
#include "NodeScoreboard.h"
#include "InvalidNodeException.h"
#include <RangeError.h>
#include <string.h>

using namespace std;

// Static data declarations:

uint16_t NodeScoreboard::m_nodeToBit[0x10000];
uint32_t NodeScoreboard::m_nodeNextBit(0x00000001);
uint32_t NodeScoreboard::m_requiredNodes(0);

//////////////////////////// Canonicals //////////////////////////////
/*!
 *   Construct the scoreboard.  
 */
NodeScoreboard::NodeScoreboard()
{
	clear();
}
/*!
 *  Copy constructor just copies the current scoreboard mask
 */
NodeScoreboard::NodeScoreboard(const NodeScoreboard& rhs) :
	m_contributedNodes(rhs.m_contributedNodes)
{
	
}
/*!
 *  Assignment:
 */
NodeScoreboard& 
NodeScoreboard::operator=(const NodeScoreboard& rhs)
{
	m_contributedNodes = rhs.m_contributedNodes; // ok even if &rhs == this
	return *this;
}
/*!
 * Equality holds if the contribedNodes member is the same in both.
 */
int
NodeScoreboard::operator==(const NodeScoreboard& rhs) const
{
	return m_contributedNodes == rhs.m_contributedNodes;
}
/*!
 *  Inequality, as always is the logical inverse of equality.
 */
int 
NodeScoreboard::operator!=(const NodeScoreboard& rhs) const
{
	return !(*this == rhs);
}
/////////////////////////////////////////////////////////////
/*!
 *   Setup the nodes needed and the node to bit mapping.
 * \param nodes vector of nodes required to make up an assembled event.
 * \throw CRangeError - if more than 32 nodes
 */
// static
void 
NodeScoreboard::neededNodes(std::vector<uint16_t> nodes)
{
	int nodeCount = nodes.size();
	if (nodeCount <= 32) {
		m_nodeNextBit   = 1;
		m_requiredNodes = 0;
		memset(m_nodeToBit, 0, sizeof(m_nodeToBit));
		
		for (int i=0; i < nodeCount; i++) {
			m_nodeToBit[nodes[i]]  = m_nodeNextBit;
			m_requiredNodes       |= m_nodeNextBit;
			m_nodeNextBit          = m_nodeNextBit << 1;
		}
	}
	else {
		throw CRangeError(0, 32, nodeCount,
				          "NodeScoreboard::neededNodes building set of needed nodes");
	}
}
/////////////////////////////////////////////////////////////
/*!
 * Add a node to the mask.  If the node does not have a 
 * corresponding mask (it maps to zero), we throw an 
 * InvalidNodeException
 * \param node - Node to add to the list of seen nodes.
*/
void 
NodeScoreboard::addNode(uint16_t node)
{
	uint32_t bit = m_nodeToBit[node];
	if (bit) {
		m_contributedNodes |= bit;
	}
	else {
		throw InvalidNodeException(node,
				                   string("NodeScoreboard::addNode - no map for node -> bitmask");
	}
}
/////////////////////////////////////////////////////////////
/*!
 * \return bool
 * \retval true - The event is complete.
 * \retval false - The event is not yet complete.
 */
bool 
NodeScoreboard::isComplete() const
{
	return ((m_requiredNodes & m_contributedNodes) == m_requiredNodes);
}
/////////////////////////////////////////////////////////////
/*!
 * Reset the set of nodes that have been received.
 */
void 
NodeScoreboard::clear()
{
	m_contributedNodes = 0;
}