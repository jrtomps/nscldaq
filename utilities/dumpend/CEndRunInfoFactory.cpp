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
# @file   CEndRunInfoFactory.cpp
# @brief  Implements the end run info factory.
# @author <fox@nscl.msu.edu>
*/

#include "CEndRunInfoFactory.h"
#include "CEndRunInfo11.h"
#include "CEndRunInfo10.h"
#include <DataFormat.h>

#include <stdexcept>
#include <exception>
#include <io.h>

/**
 * create
 *    This overload creates a new end run info class from an open file:
 *    -   Seek the file to the beginning.
 *    -   Read the first ring item.
 *    -   If the first ring item is a ring format item, return an 11 end run object.
 *    -   If the first ring item is not a ring format item, return a 10.x end run object.
 *    -   If the first ring item is a ring format item but not for 11, throw std::domain_error.
 *
 *   @param fd - file descriptor open on the file.
 *   @throw std::domain_error - if it's clear we can't build the correct object.
 *   @note - as a side effect, the file is rewound.
 */

CEndRunInfo*
CEndRunInfoFactory::create(int fd)
{
    lseek(fd, 0, SEEK_SET);               // Rewind the file.
    
    // Read the header first to see if it's a format item:
    
    
    RingItemHeader hdr;
    io::readData(fd, &hdr, sizeof(hdr));
    lseek(fd, 0, SEEK_SET);   // rewind again.

    if (hdr.s_type == RING_FORMAT) {
        // If the item is a ring format item, read the entire item.
        // 1.  If it's 11 major version build a CEndRunInfo10
        // 2.  Any other major version, throw domain error.
        
        DataFormat item;
        io::readData(fd, &item, sizeof(DataFormat));
        lseek(fd, 0, SEEK_SET);                  // Final rewind.
        if (item.s_majorVersion == 11) {
            return create(nscldaq11, fd);
        } else {
            throw std::domain_error("Looks like this file format is newer than I can handle");
        }
        
    } else {
        // If there's not a ring format item at the start of the file it must
        // be a 10.x guy:
        
        return create(nscldaq10, fd);
    }
}
/**
 * create
 *     This overload creates an end run info object of the specified type.
 *     Note that the item is dynamically pooofed into being so the caller
 *     is responsible for delete-ing it when done.
 *
 *    @param version - version of the end run info to create.
 *    @param fd   - File descriptor on file on which the ring item will be made.
 *    @return CEndRunInfo* pointer to dynamically created end run info object
 *                         of the right type
 *    @throw std::domain_error if type is not valid.
 */
CEndRunInfo*
CEndRunInfoFactory::create(CEndRunInfoFactory::DAQVersion version, int fd)
{
    switch (version) {
        case nscldaq11:
            return new CEndRunInfo11(fd);
        case nscldaq10:
            return new CEndRunInfo10(fd);
        default:
            throw std::domain_error("Invalid daq version to create");
    }
}