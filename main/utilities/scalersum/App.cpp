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
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>


/**
 * constructor
 *
 * @param args - Command line arguments processed gengetopt.
 */
App::App(struct gengetopt_args_info& args) :
    m_omitLabels(false),
    m_flip(false)
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


/*---------------------------------------------------------------------------
 * public methods
 */


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


