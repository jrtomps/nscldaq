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
# @file   CFileIterator
# @brief  Provide class to iterate over files.
# @author <fox@nscl.msu.edu>
*/
#ifndef CFILEITERATOR_H
#define CFILEITERATOR_H
#include <string>
#include <vector>

/**
 * @class CFileIterator
 *
 *  Iterate over a set of files providing a visitor mechanism.
 *  This can be used as a generic chunk of code to be used in programs that
 *  operate on one or more files.
 */

class CFileIterator {
    // Internal classes:
    
public:
    /**
     * base class user must extend to visit files.
     */
    class CVisitor {
    public:
        virtual void operator()(CFileIterator* pIterator, std::string filename) {
            pIterator->onSuccess();
        }
    };
    // Attributes:
    
private:
    std::vector<std::string> m_files;
    int m_nFileCount;
    int m_nErrorCount;
    int m_nSuccessCount;
    
    // canonicals:
    
public:
    CFileIterator(int nFiles, const char** files);
    virtual ~CFileIterator();
    
    
    // Final methods:
    
public:
    void foreach(CVisitor& v);                         // Iteration driver.
    
    // overridables -- These have some reassonble defaults.
    
public:
    virtual void onError(std::string msg);            // Report error messages.
    virtual void onSuccess();                         // Report success
    virtual void onEnd();                             // Report end of iteration.
  

};

#endif
