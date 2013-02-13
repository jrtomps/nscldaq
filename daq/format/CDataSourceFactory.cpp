/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CDataSourceFactory.h"

#include "CFileDataSource.h"
#include "CRingDataSource.h"

#include <URL.h>

/**
 * makeSource
 *
 *  Creates a dynamically allocated ring item data source and returns a pointer to it
 *  to the caller.  The caller must at some point delete the data source.
 *
 * @param uri  - Uniform resource identifier of the source. 
 * @param sample - Vector of data types that are sampled.  Note that not all data sources
 *                 support sampling (specifically file:/// URI's will ignore this).
 * @param exclude - Vector of data types not to be returned from the source.
 *
 * @return CDataSource* - Pointer to the returned data  source.
 *
 * @throw std::string - error message that describes why the data source could not be
 *                      created (normally due to a malformed URI or a URI protocol that
 *                      isn't supported (e.g. grumpy://thing).
 * @throw - whatever URI methods throw.
 */
CDataSource* 
CDataSourceFactory::makeSource(std::string uri, 
				   std::vector<uint16_t> sample , std::vector<uint16_t>exclude)
{
  CDataSource *pSource;
  URL parsedURI(uri);
  if (parsedURI.getProto() == std::string("file")) {
    // File data source:

    pSource = new CFileDataSource(parsedURI, exclude);

  } else if (parsedURI.getProto() == std::string("tcp")) {
    // ringbuffer (local or remote):

    pSource = new CRingDataSource(parsedURI, sample, exclude);
  } else {
    std::string msg = "Invalid URL protocol ";
    msg            += parsedURI.getProto();
    msg            += " in Ring data source URI: ";
    msg            + uri;
    throw msg;
  }

  return pSource;
}
