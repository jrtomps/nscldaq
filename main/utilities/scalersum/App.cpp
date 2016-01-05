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
# @file   App.cpp
# @brief  Application driver class for offline scaler sum program.
# @author <fox@nscl.msu.edu>
*/

#include "App.h"
#include <CDataSource.h>
#include <CDataSourceFactory.h>
#include <ErrnoException.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



/**
 * constructor
 *
 * @param args - Command line arguments processed gengetopt.
 */
App::App(struct gengetopt_args_info& args) :
    m_omitLabels(false),
    m_flip(false),
    m_state(App::expectingStart)
{
    // Process the cooked parameters:
    
    if (args.omit_labels_given) m_omitLabels = true;
    if (args.flip_given)        m_flip       = false;
    if (args.name_file_given) {
        processNameFile(args.name_file_arg);
    }
    
    for (int i = 0; i < args.inputs_num; i++) {
       m_files.push_back(args.inputs[i]);
    }
}
/**
 * operator()
 *     Runs the analysis. Either process all the lines or 

/*---------------------------------------------------------------------------
 * public methods
 */

/**
 * operator()
 *     Process all of the input:
 *     -   If there are no input files, a data source for stdin is created
 *         and processed.
 *     -   If there are input files those are processed.
 *  @note living above all of this is a simple state machine with the state:
 *        -  expectingStart - Looking for a begin run.
 *        -  expectingEnd   - Processing scalers until an end run.
 */
void
App::operator()()
{
    std::vector<uint16_t> dummy;
    if (m_files.size() == 0) {
        std::unique_ptr<CDataSource>
            pDs(CDataSourceFactory::makeSource("-", dummy, dummy));
        processFile(*pDs);
    } else {
        for(auto p = m_files.begin(); p != m_files.end(); p++) {
            try {
                std::string uri = makeFileUri(*p);
                std::cerr << "Processing: " << uri << std::endl;
                std::unique_ptr<CDataSource> 
                    pDs(CDataSourceFactory::makeSource(uri, dummy, dummy));
                processFile(*pDs);
            }
            catch (CErrnoException& e) {
                std::string msg = "Unable to process file : ";
                msg += *p;
                msg += " : ";
                msg += e.ReasonText();
                throw std::runtime_error(msg);
            }
        }
    }
}

void
App::processFile(CDataSource& ds) {}

/**
 * dumpScalerNames
 *    For debugging purposes, dumps the scaler name map to the
 *    specified stream.
 *
 *    @param f  - output stream to which the data will be dumped:
 */
void
App::dumpScalerNames(std::ostream& f)
{
    for(auto p = m_channelNames.begin(); p != m_channelNames.end(); p++) {
        Channel        c = p->first;
        std::string name = p->second;
        
        f << "Channel: " << c.s_dataSource << '.' << c.s_channel << " - " <<
            name << std::endl;
    }
    
}

/*--------------------------------------------------------------------------
 * private methods
 */

/**
 * processNameFile
 *    Given a file of scaler names, fill in the scaler channel -> name map.
 *
 *  @param name - name of input file.
 *  @throws     - std::invalid_argument - if the name file does not exist.
 *  @note - May produce other deal breaker errors as (eg. std::runtime_error)
 *  @note - May produce warnings on std::cerr (e.g. for redefining a scaler).
 */

void
App::processNameFile(const char* name)
{
    std::ifstream nameFile(name);
    if (nameFile.fail()) {
        std::string msg = "Could not open scaler name file: ";
        msg += name;
        throw std::invalid_argument(msg);
    }
    
    Channel chanSpec;
    std::string chname;
    
    // TODO: parse lines via intermediate string so we can give better
    //       diagnostics.
    
    while(! nameFile.eof()) {
        std::string line;
        std::getline(nameFile, line, '\n');
        if(line.size() == 0) return;
        std::stringstream sline(line);
        
        
        sline >> chanSpec.s_dataSource >> chanSpec.s_channel;
        if(sline.fail()) {
            throw std::runtime_error("Invalid line in scaler name file");
        }
        std::getline(sline, chname, '\n');
        if (m_channelNames.find(chanSpec) != m_channelNames.end()) {
            std::cerr << "Warning redefining channel " << chanSpec.s_dataSource
                << '.' << chanSpec.s_channel << " name to be: " << chname << std::endl;
        }
        m_channelNames[chanSpec] = chname;
    }
    
    
}
/**
 * getScalerName
 *    Returns the name of a channel.  The name is looked up in the m_channelNames
 *    map.  If not found an default name is generated and returned.
 *
 *  @param ch  - Channel specification (source and channel number).
 *  @return std::string - the channel label.
 *  @note   m_channelNames may be modified.
 */
std::string
App::getScalerName(App::Channel& ch)
{
    // If necessary create/insert a new name:
    
    if (m_channelNames.find(ch) == m_channelNames.end()) {
        std::stringstream name;
        name << "Scaler-" << ch.s_dataSource << '.' << ch.s_channel;
        m_channelNames[ch] = name.str();
    }
    return m_channelNames[ch];
}
/**
 * makeFileUri
 *    Turn a filename into a URI
 *
 *  @param name - name of the file.
 *  @return std::string - URI pointing at the file.
 */
std::string
App::makeFileUri(std::string name)
{
      char*  fullPath = realpath(name.c_str(), NULL);
      if (fullPath) {
        std::string uri = "file://";
        uri += fullPath;
        free(fullPath);
        return uri;
      } else {
        // Error creating the path:
        
        std::string errnomsg = strerror(errno);
        std::string msg = "Unable to create a URI for ";
        msg += name;
        msg += " : ";
        msg += errnomsg;
        throw std::runtime_error(msg);
        
      }
}

/*-------------------------------------------------------------------------
 * In order to do a map whose keys are Channel structs wwe need to impose
 * a collation ordering.   We do that by first ordering by data source
 * and then by channel within the data source
 *   Here are the ordering functions std::map requires:
 */

// Strict ordering:

int
App::Channel::operator>(const App::Channel& rhs) const {
    if (s_dataSource > rhs.s_dataSource) return 1;
    if (s_dataSource == rhs.s_dataSource) {
        return s_channel > rhs.s_channel;
    }
    return 0;
}

int
App::Channel::operator<(const App::Channel& rhs) const {
    if (s_dataSource < rhs.s_dataSource) return 1;
    if (s_dataSource == rhs.s_dataSource) {
        return s_channel < rhs.s_channel;
    }
    return 0;
}

// Equality/inequality:

int
App::Channel::operator==(const App::Channel& rhs) const {
    return ((s_dataSource == rhs.s_dataSource) && (s_channel == rhs.s_channel));
}
int
App::Channel::operator!=(const App::Channel& rhs) const {
    return !(*this == rhs);
}

// Partial in equalities:

int
App::Channel::operator<=(const App::Channel& rhs) const {
    return !(*this > rhs);
}
int
App::Channel::operator>=(const App::Channel& rhs) const {
    return !(*this < rhs);
}


