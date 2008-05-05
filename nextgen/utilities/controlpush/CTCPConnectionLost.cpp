/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";//
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//

// Header files:
#include <config.h>
#include <CTCPConnectionLost.h>
#include <CSocket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <CApplicationSerializer.h>
#include <stdio.h>


using namespace std;


// Constructors and related operations.

/*!
  'Normal constructor'  This is called prior to throwing a connection
  lost exception.  The host and port information are gotten from the 
  socket.  The reason the connection was lost is currently in errno.

  \param pSocket - the socket which just lost connection.
  \param pDoing -Describes what the socket was doing when the exceptional
     condition occured.
  */
CTCPConnectionLost::CTCPConnectionLost(CSocket* pSocket, const char* pDoing) :
  CErrnoException(pDoing)
{
  Host(pSocket);
  Port(pSocket);
}
/*!
  Copy constructor.. used by the compiler to generate termporary variables and
  of course, by throw to create a scope-safe copy of the exception being thrown.
  \param rhs - the reference object being copied.
  */
CTCPConnectionLost::CTCPConnectionLost(const CTCPConnectionLost& rhs) :
  CErrnoException(rhs),
  m_Host(rhs.m_Host),
  m_Port(rhs.m_Port)
{}

/*!
  Assignment operator.  Only slightly different than copy construction:
  - Protect against self assign.
  - Return *this.
  - this is already fully constructed.
  
  \param rhs - Reference to rhs of assignment operation.
  */
CTCPConnectionLost&
CTCPConnectionLost::operator=(const CTCPConnectionLost& rhs)
{
  if(&rhs != this) {
    CErrnoException::operator=(rhs);
    m_Host   = rhs.m_Host;
    m_Port   = rhs.m_Port;
  }
  return *this;
}
/*!
  Equality comparison.

  \param rhs The right hand side of the ==.
  */
int
CTCPConnectionLost::operator==(const CTCPConnectionLost& rhs) 
{
  return ( CErrnoException::operator==(rhs)       &&
	   (m_Host     == rhs.m_Host)             &&
	   (m_Port     == rhs.m_Port));
}


// Class operations.

/*!
  Return the reason for the exception as a textual string of the form:

  Connection with host m_Host at m_Port was lost: CErrnoException::ReasonText()

  */

const char*
CTCPConnectionLost::ReasonText() const
{
  m_ReasonText  = "Connectionwith host ";
  m_ReasonText += m_Host;
  m_ReasonText += " at ";
  m_ReasonText += m_Port;
  m_ReasonText += " was lost: ";
  m_ReasonText += CErrnoException::ReasonText();

  return m_ReasonText.c_str();
}

// Utility members.

/*!
  Extract the peer's hostname from the socket.
  This is done as follows:
  - The fd of the socket is gotten.
  - getpeername(2) is called to get the peer address.
  - gethostbyaddr(3) is called to translate this (if possible) to a hostname.
  - If gethostbyaddr(3) failed to return a useful hostname, inet_addr(3)
    is used to translate the peer address into a dotted ip string.

  \param pSocket - Pointer to the socket for which this information is to be
                   retrieved.

  NOTE:  We assume gethostbyaddr(3) and inet_addr(3) are not thread-safe.
  */

void
CTCPConnectionLost::Host(CSocket* pSocket)
{
  int         fd = pSocket->getSocketFd();
  sockaddr_in Peer;
  socklen_t   addrlen(sizeof(sockaddr_in));

  if(getpeername(fd, (sockaddr*)&Peer, &addrlen) < 0) {
    m_Host = "-unavailable-";	// This indicates we couldn't figure out host.
    return;
  }

  CApplicationSerializer::Lock(); //<-- Start critical region.
  {
    hostent* pEntry;
    pEntry = gethostbyaddr((const char*)&(Peer.sin_addr), 
			   sizeof(in_addr), AF_INET);
    if(pEntry) {
      m_Host = pEntry->h_name;	// Reverse DNS provides hostname.
    }
    else {
      m_Host = inet_ntoa(Peer.sin_addr); // Dotted IP.
    }
  }
  CApplicationSerializer::Unlock(); //--> End critical region.
}

/*!
  For a given CSocket* fills in m_Port.  This is done by:
  - Retreiving the socket fd.
  - Asking getpeername(2) who is attached to the socket.
  - Asking getservbyport(3) to return the service name from the port.
  - If getservbyport(3) failed, the numeric service name is encoded.
  - If getpeername(2) failed, the port -unavailable- is returned.

  \param pSocket - Pointer to the soket about which we're asking.

  NOTE:  We assume that getservbyport(3) is not thread safe.
  */

void
CTCPConnectionLost::Port(CSocket* pSocket)
{
  int         fd(pSocket->getSocketFd());
  sockaddr_in Peer;
  socklen_t   addrlen(sizeof(sockaddr_in));

  if(getpeername(fd, (sockaddr*)&Peer, &addrlen) < 0) {
    m_Port = "-unavailable-";
    return;

    CApplicationSerializer::Lock(); //<-- Start critical region.
    {
      servent* pEntry = getservbyport(Peer.sin_port, "tcp");
      if(pEntry) {
	m_Port = pEntry->s_name;
      }
      else {
	char port[100];
	sprintf(port, "%d", ntohs(Peer.sin_port));
	m_Port = port;
      }
      
    }
    CApplicationSerializer::Unlock(); // --> End critical region
  }
}
