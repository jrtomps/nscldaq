/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CConnectivity.cpp
 * @brief Determine which systems connect to a hosts rings.
 *
 */
#include "CConnectivity.h"
#include "CRingMaster.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

/**
 *  Static utilities:
 */

/**
 * tailIs:
 *   @param longString - a long string.
 *   @param shortString - A string that is at most as long as longstring.
 *   @return bool - true if the last characters of longString are shortString.
 *   @note if longString is shorter than shortString, automatically this is false.
 */
static bool
tailIs(std::string longString, std::string shortString)
{
    return
        (longString.size() >= shortString.size()) &&
        (longString.substr(longString.size() - shortString.size()) == shortString);

}


/*--------------------------------------------------------------------------
 * Public methods:
 */

/**
 * constructor
 *    Just produces a ring master object or throws an exception
 *    if that's not possible.
 *
 *    @param pHost - the host whose ring master we'll connect with.
 */
CConnectivity::CConnectivity(const char* pHost) :
    m_pRingMaster(0)
{
    m_pRingMaster = new CRingMaster(std::string(pHost));        
}
/**
 * destructor
 *     Destroys the ring master object.
 */
CConnectivity::~CConnectivity()
{
    delete m_pRingMaster;
}

/**
 * getProducers
 *   Returns a vector of hosts that produce data into this system.
 *   This is determined by locating rings that have names like proxy rings
 *   and that have producer processes that are stdintoring with a matching
 *   host as their last command line elements.
 *
 * @return std::vector<std::string>  - vector (possibly empty) of remote hosts.
 * @note The CRemotAccess class is such that the members of the vector will be
 *       fully qualified domain names.
 */
std::vector<std::string>
CConnectivity::getProducers()
{
    // Get usage information...that gives us the ring names to look through.
    // these are then filtered by names that look like proxy ring names.
    
    std::string rawUsage = m_pRingMaster->requestUsage();
    std::vector<std::string> rings = usageToRings(rawUsage);
    rings = proxyRingNames(rings);
    rings = ringToStdinProducers(rings);
    
    return proxyRingsToHostnames(rings);
}

/*----------------------------------------------------------------------------
 * private utilities.
 */

/**
 * usageToRings
 *    Turns the usage string into a vector of ring names.
 *
 *  @param rawUsage - the raw usage string from CRingMaster::requestUsage().
 *  @return std::vector<std::string>  - the names of the rings in the usage string.
 */
std::vector<std::string>
CConnectivity::usageToRings(std::string rawUsage)
{
    std::vector<std::string> result;
    
    // Use a captive intpereter to bust the Tcl list into its chunks.
    // We have a list of lists, element 0 of each sublist is a ringname.
    
    CTCLInterpreter interp;
    CTCLObject      usage;
    usage.Bind(interp);
    usage = rawUsage;
    
    for (auto i = 0; i < usage.llength(); i++) {
        CTCLObject ringUsage = usage.lindex(i);
        ringUsage.Bind(interp);
        result.push_back((std::string)(ringUsage.lindex(0)));
    }
    
    return result;
}
/**
 * proxyRingNames
 *   Filters a list of ringbuffers by ring names that look like they may be
 *   proxy rings.  This is done by returning only those ringnames that have
 *   an @ in their names where the @ is not the leading character: e.g.
 *   fox@spdaq22.nscl.msu.edu is likely a proxy ring for the ring fox
 *   in the host spdaq22.nscl.msu.edu.
 *
 *  @param rings - input ring names.
 *  @return std::vector<std::string>  Resulting rings that look like proxies.
 */
std::vector<std::string>
CConnectivity::proxyRingNames(const std::vector<std::string>& rings)
{
    std::vector<std::string>   result;
    
    for (auto i = 0; i < rings.size(); i++) {
        std::string r = rings[i];
        if (r[0] != '@') {             // Not a leading @
            
            // Check for an embedded @.
            
            size_t atPos = r.find("@");
            if (atPos != std::string::npos) {
                result.push_back(r);    
            }
        }
    }
    
    return result;
}
/**
 * ringToStdinInProducers
 *
 *   Filters a list of rings to those for whom the producer is
 *   ringtostdin with a final parameter that matches the trailing part of
 *   the ring name string.
 *
 *   @param rings - Set of rings to filter.
 *   @return std::vector<std::string> - Names of rings that survive the filter.
 */
std::vector<std::string>
CConnectivity::ringToStdinProducers(const std::vector<std::string>& rings)
{
    std::vector<std::string> result;
    
    // Iterate through the rings examinine their producers:
    
    for (auto i = 0; i < rings.size(); i++) {
        CRingMaster::ClientCommands clients = m_pRingMaster->listClients(rings[i]);
        
        // we need at least three command words - one ending in ringtostdout, the
        // last two the ring name and the come-from host.
        
        std::vector<std::string> p = clients.s_producer;
        if (p.size() >= 3) {
            std::string command = p[0];
            std::string ring    = p[p.size()-2];   // Should match ringname.
            std::string host    = p[p.size()-1];   // Should match hostname.
            
            // Command must be as long as "stdintoring" or longer and tail must equal it:
            
            std::string stdintoring("stdintoring");
            if (tailIs(command, stdintoring)) {
                // ring must be the same as rings[i]:
                
                if (ring == rings[i]) {
                    // Last part of rings[i] must be the same as host:
                    
                    if (tailIs(rings[i], host)) {
                        result.push_back(host);
                    }
                }
            }
        }
    }
    return result;
}
/**
 *  proxyRingsToHostnames
 *     Given a vector of ringnames that are strongly believed to be proxiesm,
 *     returns the hosts their data comes from.
 *
 *   @param rings - names of rings that are proxies.
 *   @return std::vector<std::string>
 */
std::vector<std::string>
CConnectivity::proxyRingsToHostnames(const std::vector<std::string>& rings)
{
    std::vector<std::string> result;
    
    for (auto i = 0; i < rings.size(); i++) {
        // the hostname is what's past the "@".
        
        const std::string& ring = rings[i];
        result.push_back(ring.substr(ring.find("@") + 1));
    }
    
    return result;
}
