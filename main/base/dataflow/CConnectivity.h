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
 * @file CConnectivity.h
 * @brief Determine which systems connect to our rings.
 *
 */

#ifndef CCONNECTIVITY_H
#define _CCONNECTIITY_H

#include <vector>
#include <set>
#include <string>
#include <CRingMaster.h>


/**
 * CConnectivity
 *   This class provides a mechanism to determine which hosts
 *   get or put data into a specified host.  It does this
 *   by examining the ringbuffers in that system.  
 *      Ringbuffers named something@something.else represent 
 *   proxy rings that indicate the system is getting data 
 *   from the ring 'something' located in the host something.else.
 *     Ringbuffers that have a consumer that looks like
 *   ringtostdout that also have a parameter indicate data being
 *   sent from that ring to the host indicated by the parameter.
 * 
 *   Note that we need help to look up the command associated
 *   with a ring buffer's clients.  The ringmaster has been
 *   modified to support that with the CLIENTS operation.
 *
 *  *  getProducers therefore gets the hosts that produce data into
 *     this node.
 */

class CConnectivity {
private:
  CRingMaster*  m_pRingMaster;
public:
  CConnectivity(const char *pHost);
  virtual ~CConnectivity();
  
  std::vector<std::string> getProducers();
  std::vector<std::string> getConsumers();
  std::vector<std::string> getAllParticipants();
  
  // Utilities:
  
  std::vector<std::string> usageToRings(std::string rawUsage);
  std::vector<std::string> proxyRingNames(const std::vector<std::string>& rings);
  std::vector<std::string> ringToStdinProducers(const std::vector<std::string>& rings);
  std::vector<std::string>  proxyRingsToHostnames(const std::vector<std::string>& rings);

  std::vector<CRingMaster::commandWords> listRing2Stdout(
      const std::vector<CRingMaster::commandWords>& consumers);
  std::vector<std::string> extractHosts(
      const std::vector<CRingMaster::commandWords>& consumers); 

  static void getAllParticipants(CConnectivity& aHost, std::set<std::string>& seen);
};


#endif
