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
# @file   CVardbRingBuffer.h
# @brief  C++ API for representing ring buffers in a variable data base.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARDBRINGBUFFER_H
#define CVARDBRINGBUFFER_H

#include <string>
#include <vector>

class CVarMgrApi;

/**
 * @class CVardbRingBuffer
 *
 *   This class provides an API for creating and manipulating ring buffer
 *   object definitions in the variable database.
 *
 */
class CVardbRingBuffer
{
    // Public data types.

public:
    typedef struct _RingInfo {
        std::string s_name;
        std::string s_host;
        unsigned    s_dataSize;
        unsigned    s_maxConsumers;
    } RingInfo, *pRingInfo;
    
    // Private attribute data.
    
private:
    CVarMgrApi*   m_pApi;
    
    // Canonicals:
    
public:
    CVardbRingBuffer(const char* pDbUri);
    virtual ~CVardbRingBuffer();
    
    // API:
    
public:
    bool haveSchema();
    void createSchema();
    
    void create(
        const char* name, const char* host, unsigned maxData = 8*1024*1024,
        unsigned maxConsumers = 100
    );
    void destroy(const char* name, const char* host);
    void setMaxData(const char* name, const char* host, unsigned newValue);
    void setMaxConsumers(const char* name, const char* host, unsigned newValue);
    RingInfo ringInfo(const char* name, const char* host);
    std::vector<RingInfo> list();
    
    void setEditorPosition(const char* name, const char* host, int x, int y);
    int getEditorXPosition(const char* name, const char* host);
    int getEditorYPosition(const char* name, const char* host);
    
    // utility methods:
    
private:
    std::string ringDir(const char* name, const char* host) const;
    std::string ringParentDir() const;
    
    std::string usToString(unsigned val);
    void rmTree(std::string dirName);
    
    std::string ringDirToName(std::string dirName);
    std::string ringDirToHost(std::string dirName);
};


#endif
