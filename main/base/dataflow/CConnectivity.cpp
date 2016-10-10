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
#include <set>
#include <unistd.h>

#include<iostream>

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


/**
 * dumpStrings
 *   Debugging utility to dump a vector of strings to an output file:
 */
static void
dumpStrings(std::ostream& o, const std::vector<std::string>& s)
{
    for (auto p = s.begin(); p != s.end(); p++) {
        std::cout << *p << std::endl;
    }
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
/**
 * getConsumers
 *   Gets the names of hosts that consume data from us.  This is done by 
 *   looking at the consumers for all rings and finding those with
 *   consumers of the form ..../ringtostdout   hostname
 *   The hostname on the back end of ringtostdout... those are
 *   The hosts to which data are being sent.
 *
 *  @return std::vector<std::string> - The (possibly empty) list
 *        of consumer nodes.
 */
std::vector<std::string>
CConnectivity::getConsumers()
{
  std::string rawUsage = m_pRingMaster->requestUsage();
  std::vector<std::string> rings = usageToRings(rawUsage);
  
  // The loop below makes a vector of all consumers of all rings.
  // for now we don't really care which ring is actually
  // supplying the remote guys.

  std::vector<CRingMaster::commandWords> consumers;
  for (size_t i = 0; i < rings.size(); i++) {
    CRingMaster::ClientCommands clients = m_pRingMaster->listClients(rings[i]);
    consumers.insert(
        consumers.end(), 
        clients.s_consumers.begin(), clients.s_consumers.end());
  }
  // Filter that by clients that are ringtostdout.

  consumers = listRing2Stdout(consumers);
  std::vector<std::string> hosts = extractHosts(consumers);

  //  Now use a set to uniquify the hosts and then extract
  // those back out into a vector of hosts

  std::set<std::string> uniqueHosts;
  for(int i = 0; i < hosts.size(); i++) {
    uniqueHosts.insert(hosts[i]);
  }
  // Pull the set keys back out into the result vector:

  std::vector<std::string> result;
  for (auto p = uniqueHosts.begin(); p != uniqueHosts.end(); p++) {
    result.push_back(*p);
  }
  return result;
}
/**
 * getAllParticipants
 *   Returns the set of all nodes that are participating in the
 *   dataflow.  This is done by starting here and crawling the
 *   graph of connected nodes.  Note that any exceptions in attempts
 *   to connect to remote ring masters are just treated like the
 *   remote node is not a participant.
 *
 *   @note - what we really do is just invoke the private
 *           static metnhod getAllParticipants handing off us
 *           as the parameter.  That method recursively walks the
 *           connectivity graph.
 *   @return std::vector<std::string> - The nodes involved.
 */
std::vector<std::string>
CConnectivity::getAllParticipants()
{
  std::vector<std::string> result = getAllParticipants(*this);

  return result;
}
/*---------------------------------------------------------------------------
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
/**
 * listRing2Stdout
 *   List the consumers that are ring2stdout.  This the same
 *   As returning the list of consumers whose first word has the 
 *   tail ringtostdout
 *
 *  @param consumers - Vector of consumers.
 *  @return std::vector<CRingMaster::commandWords> -
 *          Vector of commands that are ringtostdout commands.
 */
std::vector<CRingMaster::commandWords>
CConnectivity::listRing2Stdout(
    const std::vector<CRingMaster::commandWords>& consumers
)
{
  std::vector<CRingMaster::commandWords> result;
  for (int i = 0; i < consumers.size(); i++) {
    CRingMaster::commandWords c = consumers[i];
    if (tailIs(c[0], "ringtostdout")) {
      result.push_back(c);
    }
  }
  return result;
}
/**
 * extractHosts
 *   The host of a ringtostdout command is the final element of
 *   the command line which must have at least three elements: 
 *   command, ringname, hostname.
 *
 * @param consumers - vector of consumer command word vectors.
 * @return std::vector<std::string> host that were encountered.
 * @note - duplicates are not removed.
 */
std::vector<std::string>
CConnectivity::extractHosts(
    const std::vector<CRingMaster::commandWords>& consumers
)
{
  std::vector<std::string> result;
  for (size_t i = 0; i < consumers.size(); i++) {
    std::vector<std::string> command = consumers[i];
    if (command.size() >= 3) {
      result.push_back(command[command.size()-1]);
    }
    return result;
  }
}
/**
 * getAllParticipants
 *   Returns all of the participants in the data flow relative to a 
 *   starting point:
 *     -  This host is put in the set of participants.
 *     -  The connections here are gotten and put in an std::set.
 *     -  For each participant in the set, a new bunch of participants
 *        is gotten (recursive) and the new unique ones added to the set.
 *     -  vector of set keys is then returned.
 *
 *  @param initial - an initial location from which to start probing.
 *  @return std::vector<std::string> - the unique hosts involved in the data flow.
 */
std::vector<std::string>
CConnectivity::getAllParticipants(CConnectivity& initial)
{
  std::set<std::string> allClients;
  
  // If starting host is localhost  - map it to its canonical
  // hostname:
  
  std::string startingHost = initial.m_pRingMaster->getHost();
  
  if (startingHost == "localhost") {
    char canonicalHost[100];
    gethostname(canonicalHost, sizeof(canonicalHost));
    startingHost = Os::getfqdn(canonicalHost);
  } else {
    startingHost = Os::getfqdn(startingHost.c_str());
  }
  allClients.insert(startingHost);    // Initial's host is always in the set.

  // Get the clients:

  std::cout << "Probing participants starting at " << startingHost << std::endl;
  std::vector<std::string> clients = initial.getProducers();
  std::cout << "Producers: ";
  dumpStrings(std::cout, clients);
  std::cout << std::endl;
  
  std::cout << "Consumers: ";
  std::vector<std::string> consumers = initial.getConsumers();
  dumpStrings(std::cout, consumers);
  std::cout << std::endl;
  
  
  clients.insert(clients.end(), consumers.begin(), consumers.end());
    std::cout<< " Combined: ";
    dumpStrings(std::cout, clients);  
  
  for (size_t i = 0; i < clients.size(); i++) {
    if(!allClients.count(clients[i])) {
      try {
	CConnectivity next(clients[i].c_str());
	std::vector<std::string> additionalHosts = getAllParticipants(next);
	for (size_t h =0; h < additionalHosts.size(); h++) {
	  allClients.insert(additionalHosts[h]);
	}
	allClients.insert(clients[i]); // Include the host we just checked.
      }
      catch(...) {}                 
    }
    
  }
  std::cout << "Returning: ";
  dumpStrings(std::cout, std::vector<std::string>(allClients.begin(), allClients.end()));
  std::cout << std::endl;
  return std::vector<std::string>(allClients.begin(), allClients.end());

}    
