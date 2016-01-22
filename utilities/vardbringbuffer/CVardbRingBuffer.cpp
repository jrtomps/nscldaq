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
# @file   CVardbRingBuffer.cpp
# @brief  Implement the VarDB ring buffer C++ API.
# @author <fox@nscl.msu.edu>
*/
#include "CVardbRingBuffer.h"
#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>
#include <sstream>
#include <stdlib.h>

/**
 * constructor
 *    Create an api object and save it in our member data
 *
 *  @param pDbUri - uri that designates the database we are connecting to.
 */
CVardbRingBuffer::CVardbRingBuffer(const char* pDbUri) :
    m_pApi(0)
{
    m_pApi = CVarMgrApiFactory::create(pDbUri);
}

/**
 * destructor
 */
CVardbRingBuffer::~CVardbRingBuffer()
{
    delete m_pApi;
}
/**
 *  haveSchema
 *     @return bool - true if the ring buffer schema is set up. false if not
 */
bool
CVardbRingBuffer::haveSchema()
{
    // If the schema exists we can cd to the top level directory:
    // 
    bool result;   
    try {
        m_pApi->cd(ringParentDir().c_str());
        m_pApi->cd("/");
        result = true;
    }
    catch (...) {
        result = false;
    }
    return result;
}

/**
 * createSchema
 *   If the schema does not exist, create it.
 */
void
CVardbRingBuffer::createSchema()
{
    if (!haveSchema()) {
        m_pApi->mkdir(ringParentDir().c_str());
    }
}

/**
 * create
 *    Create a new ring buffer description.
 *
 * @param name - name of the ring.
 * @param host - host in which the ring will be created.
 * @param maxData - Size of the data part of the ring buffer in bytes
 * @param maxConsumers - Number of consumer struct will be created in the header.
 *
 *  @throws - std::exception - If the ring already exist.
 */
void
CVardbRingBuffer::create(
    const char* name, const char* host, unsigned maxData , unsigned maxConsumers 
)
{
    std::string dirName = ringDir(name, host);
    m_pApi->mkdir(dirName.c_str());      // Throws if exists.
    m_pApi->cd(dirName.c_str());
    
    m_pApi->declare("datasize", "integer", usToString(maxData).c_str());
    m_pApi->declare("maxconsumers", "integer", usToString(maxConsumers).c_str());
    m_pApi->declare("editorx", "integer", "0");
    m_pApi->declare("editory", "integer", "0");
    
    m_pApi->cd("/");
}

/**
 * destroy
 *    Kill off a directory tree --- that happens to contain a ring buffer
 *    definition.
 *
 * @param name - name of the ring buffer.
 * @param host - Host in which the ring buffer lives.
 */
void
CVardbRingBuffer::destroy(const char* name, const char* host)
{
    std::string dirName = ringDir(name, host);
    rmTree(dirName);
}

/**
 * setMaxData
 *    Modifies the maxdata value for an existing ring buffer definition.
 *
 * @param newValue - New value for the maxdata value.
 */
void
CVardbRingBuffer::setMaxData(
    const char* name, const char* host, unsigned newValue
)
{
    std::string dirName = ringDir(name, host);
    m_pApi->cd(dirName.c_str());
    
    m_pApi->set("datasize", usToString(newValue).c_str());
    
    m_pApi->cd("/");
}
/**
 * setMaxConsumers
 *    Modify the maximum number of consumers a ring buffer can have
 *    simultaneously.
 * @param newValue - new number.
 */
void
CVardbRingBuffer::setMaxConsumers(
    const char* name, const char* host, unsigned newValue
)
{
    m_pApi->cd(ringDir(name, host).c_str());
    
    m_pApi->set("maxconsumers", usToString(newValue).c_str());
    
    m_pApi->cd("/");
}

/**
 * ringInfo
 *    Return information about the specified ring.
 *
 * @param name - ring name.
 * @param host - Host of ring.
 *
 * @return RingInfo
 */
CVardbRingBuffer::RingInfo
CVardbRingBuffer::ringInfo(const char* name, const char* host)
{
    RingInfo result;
    m_pApi->cd(ringDir(name, host).c_str());
    
    result.s_name = name;
    result.s_host = host;
    result.s_dataSize     = strtoul(m_pApi->get("datasize").c_str(), NULL, 0);
    result.s_maxConsumers =
        strtoul(m_pApi->get("maxconsumers").c_str(), NULL, 0);
    
    m_pApi->cd("/");
    
    return result;
}
/**
 * list
 *    List information about all of the ring buffers.
 * @return std::vector<RingInfo> - vector, one RingInfo struct per ringbuffer.
 */
std::vector<CVardbRingBuffer::RingInfo>
CVardbRingBuffer::list()
{
    std::vector<RingInfo> result;
    
    // List the ring buffers:
    
    std::vector<std::string> ringDirs = m_pApi->ls(ringParentDir().c_str());
    for (auto p = ringDirs.begin(); p != ringDirs.end(); p++) {
        std::string name = ringDirToName(*p);
        std::string host = ringDirToHost(*p);
        
        result.push_back(ringInfo(name.c_str(), host.c_str()));
    }
    
    return result;
}

/**
 * setEditorPosition
 *   Set a new position for the ring buffer on the editor canvas.
 *
 * @param name - Name of the ring buffer.
 * @param host - Host in whih the ring buffer is created (tcp://host/ring).
 * @param x    - X coordinate of canvas position.
 * @param y    - Y coordinate of canvas position.
 */
void
CVardbRingBuffer::setEditorPosition(const char* name, const char* host, int x, int y)
{
    std::string dir =ringDir(name, host);
    m_pApi->cd(dir.c_str());
    
    m_pApi->set("editorx", usToString(x).c_str());
    m_pApi->set("editory", usToString(y).c_str());
    
    m_pApi->cd("/");
}

/**
 * getEditorXPosition
 *
 * @param name - ring buffer name.
 * @param host - host in which the ring buffer is created.
 * @return int - X coordinate of ring buffer editor canvas position.
 */
int
CVardbRingBuffer::getEditorXPosition(const char*  name, const char* host)
{
    std::string var = ringDir(name, host);
    var += "/editorx";
    
    return atoi(m_pApi->get(var.c_str()).c_str());
}
/**
 * getEditorYPosition
 *
 * @param name - ring buffer name.
 * @param host - host in which the ring buffer is created.
 * @return int - Y coordinate of ring buffer editor canvas position.
 */
int
CVardbRingBuffer::getEditorYPosition(const char*  name, const char* host)
{
    std::string var = ringDir(name, host);
    var += "/editory";
    
    return atoi(m_pApi->get(var.c_str()).c_str());
}
/*-----------------------------------------------------------------------------
 *  Private utility methods.
 */
/**
 * ringDir
 *    @param name - ring buffer name.
 *    @param host - host in which the ring buffer will be created.
 *    @return std::string - full path to the designated ring buffer.
 */
std::string
CVardbRingBuffer::ringDir(const char* name, const char* host) const
{
    std::string result = ringParentDir();
    result +=  "/";
    result += name;
    result += "@";
    result += host;
    
    return result;
}

/**
 * ringParentDir
 *
 *  @return std::string - Return the directory in which the rings are held.
 */
std::string
CVardbRingBuffer::ringParentDir() const
{
    return std::string("/RingBuffers");
}

/**
 * usToString
 *   @param val - value to convert.
 *   @return std::string - value converted to its string rep.
 */
std::string
CVardbRingBuffer::usToString(unsigned val)
{
    std::ostringstream os;
    os << val;
    return os.str();
}
/**
 *  rmTree
 *     Remove a subtree from the database
 *  @param dirName - directory at the top of the tree to remove.
 *  @note - dirName is also removed.
 *  @note - we do things this way because the user may want to put additional
 *          data and even subdirectories in a ring buffer.
 */
void
CVardbRingBuffer::rmTree(std::string dirName)
{
    std::string here = m_pApi->getwd();
    m_pApi->cd(dirName.c_str());
    
    // Delete variables.
    
    std::vector<CVarMgrApi::VarInfo> vars = m_pApi->lsvar();
    for (auto p = vars.begin(); p != vars.end(); p++) {
        m_pApi->rmvar(p->s_name.c_str());
    }
    
    // recurse to delete subdirs.
    
    std::vector<std::string> subdirs = m_pApi->ls();
    for (auto p = subdirs.begin(); p != subdirs.end(); p++) {
        rmTree((*p).c_str());
    }
    
    // delete dirName.
    
    m_pApi->cd(here.c_str());
    m_pApi->rmdir(dirName.c_str());
}

/**
 * ringDirToName
 *    Given a ring directory, parse out the piece before the @ .. which is
 *    the ring name.
 *
 * @param dir - ring directory name.
 * @return std::string
 */
std::string
CVardbRingBuffer::ringDirToName(std::string dir)
{
    size_t index = dir.find('@');
    return dir.substr(0, index);
}
/**
 * ringDirToHost
 *    Given a ring directory, parse out the piece after the @ .. which is the
 *    host the ring is in.
 *
 *  @param dir - directory to parse.
 *  @return std::string
 */
std::string
CVardbRingBuffer::ringDirToHost(std::string dir)
{
    size_t index = dir.find('@');
    return dir.substr(index+1);
}