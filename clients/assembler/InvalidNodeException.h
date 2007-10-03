#ifndef INVALIDNODEEXCEPTION_H_
#define INVALIDNODEEXCEPTION_H_
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

#ifndef __EXCEPTION_H
#include <Exception.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

/*!
 *    This class provides an exception that is thrown when
 * the input stage e.g. detects an attempt to use a node fragment
 * queue that does not exist.  This will be because the node
 * is not in the configuration for the event assembler.
 */

class InvalidNodeException : public CException
{
private:
	uint16_t             m_node;
	mutable std::string  m_reason;
	
public:
	InvalidNodeException(uint16_t node, 
			     std::string reason);
	virtual ~InvalidNodeException();
	
	// Special stuff for this exception:
public:
	uint16_t getNode() const;
	
	// Virtual method overrides:
	
	virtual const char* ReasonText() const;
	virtual Int_t ReasonCode() const;

};

#endif /*INVALIDNODEEXCEPTION_H_*/
