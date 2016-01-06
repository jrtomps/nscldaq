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
# @file   CChannel.h
# @brief  Define a class that manages scaler sums.
# @author <fox@nscl.msu.edu>
*/

#ifndef CCHANNEL_H
#define CCHANNEL_H

#include <stdint.h>

/**
 * @class
 *     Encapsulates a scaler channel.  This is an abstract base class
 *     because the handling of incremental and non-incremental channels
 *     are so fundamentally different.
 */
class CChannel
{
public:
    virtual void updateunsigned value, unsigned width=32) = 0; // update with new value
    virtual uint64_t() = 0;                                  // fetch sum.
};

#endif
