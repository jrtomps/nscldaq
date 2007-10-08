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
#ifndef INPUTBUFFERFACTORY_H_
#define INPUTBUFFERFACTORY_H_

class InputBuffer;

/*!
 *   InputBufferFactory creates the approrpriate input
 * buffer class given a pointer to a raw buffer.
 */



class InputBufferFactory
{
public:
	static InputBuffer* create(void* pBuffer);
};

#endif /*INPUTBUFFERFACTORY_H_*/
