/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CIncrementalChannel.cpp
# @brief  Implement the logic of summing incremental channels.
# @author <fox@nscl.msu.edu>
*/


#include "CIncrementalChannel.h"

/**
 * construtor
 *    Zeros the sum.
 */
CIncrementalChannel::CIncrementalChannel() :
    m_sum(0)
    {}
    
    
/**
 *  update
 *      Add a new increment to the sum.
 *  @param value - scaler value over some interval.
 *  @param width - Number of bits in the scaler.
 */
void
CIncrementalChannel::update(unsigned value, unsigned width)
{
    
    // The seeming laborious work here ensures that everything is done with
    // 64 bits of width.
    
    uint64_t mask = 1;
    mask = (mask << width) - 1;    // width bits of one.
    
    uint64_t increment = value;
    increment = increment & mask;
    m_sum += increment;
}

/**
 * operator uint64_t
 *
 *  @return uint64_t the sum.
 */
CIncrementalChannel::operator uint64_t()
{
    return m_sum;
}