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
# @file   CFileIterator.cpp
# @brief  Implementation of the file iterator base class.
# @author <fox@nscl.msu.edu>
*/
#include "CFileIterator.h"
#include <iostream>               // For default onError, onEnd handlers.
#include <algorithm>
#include <functional>

/**
 * constructor
 *    @param nFiles    - Number of files in the list.
 *    @param files     - Pointers to the filenames.
 */
CFileIterator::CFileIterator(int nFiles, const char** files) :
    m_nFileCount(0), m_nErrorCount(0), m_nSuccessCount(0)
{
    // Edge case for no files needs to be handled properly:
    
    if (nFiles > 0) {
        m_files.insert(m_files.begin(), files, files+nFiles);
    }
}
/**
 *  destructor
 */
CFileIterator::~CFileIterator() {}

/**
 * foreach
 *    For each member of the vector of files, the visitor
 *    is called with this *this and the filename passed in as
 *    parameters.
 *
 * @param v - visitor functor to call.
 */
void
CFileIterator::foreach(CFileIterator::CVisitor& v)
{
    m_nFileCount    = 0;
    m_nErrorCount   = 0;
    m_nSuccessCount = 0;
    std::for_each (
        m_files.begin(), m_files.end(),
        std::bind(std::ref(v),  this, std::placeholders::_1)
    );
    onEnd();
}
/**
* onError
*    This can be called if an error occured.  It:
*    outputs its parameter to std::cerr
*    Counts a file
*    Counts an error.
*
*    @param msg - error message to output.
*
*    Visitors must call this.
*/
void
CFileIterator::onError(std::string msg)
{
    std::cerr << msg << std::endl;
    m_nErrorCount++;
    m_nFileCount++;
}
/**
 * onSuccess
 *   Can be called if file processing was successful.
 *   just increments the file count. and success counts.
 *   Visitors must call this.
 */
void
CFileIterator::onSuccess()
{
    m_nSuccessCount++;
    m_nFileCount++;
}
/**
 * onEnd
 *   Called by the iteration framework at the end
 *   of iteration. If the file count is nonzero then
 *   the iteration statistics are output to std::error.
 *   IF the file count is zero it's assumed that either there were
 *   no file (probably bad) or that the client did not want end of run
 *   statistics printed out.
 */
void
CFileIterator::onEnd()
{
    if (m_nFileCount > 0) {
        std::cerr << "Files processed: " << m_nFileCount << " Successful: "
            << m_nSuccessCount << " Failed: " << m_nErrorCount << std::endl;
    }
}
