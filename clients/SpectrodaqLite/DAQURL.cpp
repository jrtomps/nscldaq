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

#include <config.h>
#include <string.h>
#include <spectrodaq.h>
#include <stdlib.h>

using namespace std;

// Implements the DAQURL deriviation of URL
// to give us constructors that are like the
// old system.

/*!
   Construct a URL using an std::string:

*/
DAQURL::DAQURL(string url) :
  URL(host(url.c_str()), port(url.c_str()))
{}
/*!
   Same but use a const char* to a null terminated
   string:
*/

DAQURL::DAQURL(const char* url) :
  URL(host(url), port(url))
{}

DAQURL::DAQURL() :
  URL(host("tcp://localhost:2602"), port("tcp://localhost:2602"))
{
}

//  Pull the port out of a string of the form:
//    tcp://host:port[/]
// We're going to assume the URL is well formed for now

int
DAQURL::port(const char* url)
{
  char* port = strrchr(url, ':');
  port++;

  return atoi(port);
}

/*!  Get the hostname from the URL
 */
String
DAQURL::GetHostName()
{

  return String(getHostName());
}

//!  Get the path part of the URL
String
DAQURL::GetPath()
{
  return String(getPath());
}

// Pull the host part from the string.
// We again assume the URL is well formed;

// Very not threadsafe:

daqhwyapi::String result;

daqhwyapi::String&
DAQURL::host(const char* url)
{
  char* start = strchr(url, ':');
  start += 3; 			// Skip over "://".

  char* end   = strrchr(url, ':');

  size_t finalLength = end-start; // Chars + null terminator.

  char* host = new char[finalLength];
  memset(host, 0, finalLength);
  strncpy(host, start, finalLength-1);

  delete []host;

  return result;
}
