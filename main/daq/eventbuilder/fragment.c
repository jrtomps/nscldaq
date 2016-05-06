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
#include "fragment.h"
#include <stdlib.h>
#include <string.h>

/**
 * Free a fragment.  The assumption is that  both the header and the body 
 * are dynamically allocated.
 *
 * @param p - ppointer to the fragment.
 */
void freeFragment(pFragment p) 
{
  free(p->s_pBody);
  p->s_pBody = 0;
  free(p);

}
/**
 * Create a new dynamically allocated fragment from an existing
 * header that has been filled in .k
 * - Ther etruned fragment can be freed via freeFragment.
 * - We need the s_size field filled in because that's what determines
 *  the full size of the storage.
 *
 * @para pHeader - Pointer to the fragment header.
 *
 *  TODO: malloc failure handling.
 */
pFragment allocateFragment(pFragmentHeader pHeader)
{
  pFragment p = malloc(sizeof(Fragment));
  memcpy(&(p->s_header), pHeader, sizeof(FragmentHeader));

  p->s_pBody = malloc(pHeader->s_size);

  return p;
}
/**
 * Create a new dynamically allocated fragment from 
 * the parameters that go in the header. The body is
 * allocated but nothing put in it.
 *
 * @param timestamp - The fragment timestamp.
 * @param sourceId  - The fragment source id.
 * @param size      - The size of the fragment body.
 */
pFragment newFragment(uint64_t timestamp, uint32_t sourceId, uint32_t size)
{
  FragmentHeader h = {timestamp, sourceId, size};
  return allocateFragment(&h);
}
/**
 * Determine the total number of bytes needed to flatten a fragment chain out into
 * memory (e.g. a message payload).
 *
 * @param p - pointer to the fragment chain.
 */
size_t
fragmentChainLength(pFragmentChain p)
{
  size_t result = 0;
  while (p) {
    result += sizeof(FragmentHeader) + p->s_pFragment->s_header.s_size;
    p = p->s_pNext;
  }

  return result;
}
