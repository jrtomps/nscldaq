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




#ifndef __CBUFFERMONITOR_CPP
#define __CBUFFERMONITOR_CPP
#include <config.h>
#include <CBufferMonitor.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
  \fn CEventMonitor::result CBufferMonitor::operator()() 

 Operation Type:
    override
 
 Purpose:
    Waits for a buffer to be received. Returns one of
    the following: 
    Occurred - a buffer was received into m_Buffer
    TimedOut - Timeouts were enabled and no buffer was
    received during the timeout interval.

*/
template<class T>
CEventMonitor::result
CBufferMonitor<T>::operator() ()
{ 
  m_Buffer.SetTag(m_nTag);

  // If timeouts were enabled:
  if(getTimedWait()) {
    struct timeval timeout = getTimeout();
    // Accept a buffer
    m_Buffer.Accept(&timeout);
    if(m_Buffer.GetLen() != 0)
      return CEventMonitor::Occurred;
    else
      return CEventMonitor::TimedOut;
  }
  // Timeouts disabled
  else {
    m_Buffer.Accept();
    if(m_Buffer.GetLen() != 0)
      return CEventMonitor::Occurred;
    else
      return CEventMonitor::TimedOut;
  }
}

/*!
  \fn int CBufferMonitor::AddLink (const string& URL, int tag = COS_ALLBITS,
                                   int mask = COS_ALLBITS, bool fReliable=true)

 Operation Type:
    Mutator

 Purpose:
    Adds a link to the link manager. The link id is returned.
    On failure, a CLinkFailedException is thrown.
*/
template<class T>
int
CBufferMonitor<T>::AddLink (const string& URL, int tag, int mask, 
			    bool fReliable)
{
  LinkInfo info;
  info.Tag = tag;
  info.Mask = mask;
  info.URL = URL;
  DAQURL daqurl = DAQURL(URL.c_str());
  info.linkid = daq_link_mgr.AddSink(daqurl, tag, mask, 
				     fReliable ? COS_RELIABLE 
				     : COS_UNRELIABLE);
  if(info.linkid == 0) {
    throw CLinkFailedException
      ("CBufferMonitor<T>::AddLink Adding link to list", info.linkid);
  }
  m_lLinks.push_back(info);
  return info.linkid;
}

/*!
  \fn void CBufferMonitor::RemoveLink (int linkid)

 Operation Type:
    Mutator

 Purpose:
    If the specified link exists, it is removed from the
    link list and deleted from the spectrodaq link manager.
    If the link does not exist, a CNoSuchLinkException
    is thrown.
*/
template<class T>
void
CBufferMonitor<T>::RemoveLink (int linkid)
{
  LinkIterator linkIt;
  for(linkIt = m_lLinks.begin(); linkIt != m_lLinks.end(); linkIt++) {
    if ((*linkIt).linkid == linkid) {
      daq_link_mgr.DeleteSink(linkid);

      m_lLinks.erase(linkIt);
    }
  }
  if(linkIt == m_lLinks.end())
    throw CNoSuchLinkException
      ("CBufferMonitor<T>::RemoveLink Removing link from list", linkid);
}

/*!
  \fn void CBufferMonitor::RemoveLink (LinkIterator link)

 Operation Type:
    Mutator
 
 Purpose:
    Removes a link given the iterator to its link structure
    in the link list. If the iterator is end(), a
    CNoSuchLinkException is thrown.
*/
template<class T>
void
CBufferMonitor<T>::RemoveLink (LinkIterator link)
{
  if(link != m_lLinks.end()) {
    daq_link_mgr.DeleteSink((*link).linkid);
    m_lLinks.erase(link);
  }
  else {
    throw CNoSuchLinkException
      ("CBufferMonitor<T>::Remove Removing link from list", (*link).linkid);
  }
}

/*!
  \fn LinkIterator CBufferMonitor::FindLink (LinkMatchPredicate& rPredicate,
                                             LinkIterator startat)

 Operation Type:
    Selector
 
 Purpose:
    Locates the first link that satisfies a given
    predicate. Predefined predicates include:
    MatchURL - matches URL only
    MatchAll - Matches URL, tag and mask.
    A LinkMatchPredicate is a function object implementing:
    bool operator()(LinkInfo) which returns TRUE if the link
    satisfies the predicate. Returns an iterator 'pointing'
    to the first match, or end() if there is no match.
*/
template<class T>
template<class LinkMatchPredicate>
LinkIterator
CBufferMonitor<T>::FindLink (LinkMatchPredicate& rPredicate, 
			     LinkIterator startat)
{
  for(LinkIterator link = startat; link != m_lLinks.end(); link++) {
    if(rPredicate(*link))
      return link;
  }
  return m_lLinks.end();
}

/*!
  \fn LinkIterator CBufferMonitor::beginLinks()

 Operation Type:
    Selector
 
 Purpose:
    Returns an iterator to the beginning of the
    link list.
*/
template<class T>
LinkIterator
CBufferMonitor<T>::beginLinks()
{
  return m_lLinks.begin();
}

/*!
  \fn LinkIterator CBufferMonitor::endLinks()

 Operation Type:
    Selector
 
 Purpose:
    Returns an iterator suitable for determining
    end of iteration through the link list.
*/
template<class T>
LinkIterator
CBufferMonitor<T>::endLinks()
{
  return m_lLinks.end();
}

/*!


 Operation Type:
    Selector
    
 Purpose:
    Returns a pointer to the DAQ Buffer.

*/
template<class T>
T*
CBufferMonitor<T>::getBufferPointer (int nOffset)
{
  char* p   = m_Buffer.GetPtr();
  p        += nOffset*sizeof(T);
  return static_cast<T*>(p);

}

/*!
  \fn void CBufferMonitor::SetBufferTag (int tag = COS_ALLBITS)

 Operation Type:
    Mutator
 
 Purpose:
    Sets the tag matched on receives into the buffer.
    For each link/buffer pair, tags are used to determine
    which buffers routed through SpectroDaq will be
    received by a link or buffer.  The logic is that
    at each stage, the routed buffer's tag is anded with
    the receiving entity's mask.  If this is equal to the
    receiving entity's tag, the buffer is accepted.
    So, for a given link (link.mask, link.tag), and
    our buffer (buffer.mask, buffer.tag):
    A routed buffer rbuffer.tag is received when:
    
    ((rbuffer.tag & link.mask) == link.tag) &&
    ((rbuffer.tag & buffer.mask) == buffer.tag)
*/
template<class T>
void 
CBufferMonitor<T>::SetBufferTag (int tag)
{
  m_Buffer.SetTag(tag);
  m_nTag = tag;

}

/*!
  \fn void CBufferMonitor::SetBufferMask (int nMask)

 Operation Type:
    Mutator
 
 Purpose:
    Sets the receive mask associated with the buffer.
    See SetBufferTag for an explanation of tags and masks
    and how they interact with link tags and masks and the tag
    of the incomming buffer to determine receipt.
*/
template<class T>
void
CBufferMonitor<T>::SetBufferMask (int nMask)
{

  m_nMask = nMask;

}

/*!
  \fn string CBufferMonitor::DescribeSelf()

 Operation Type:
    Selector
 
 Purpose:
    Produces a desciprtion string of the object.  This includes
    1. Calling CEventManager::DescribeSelf()
    2. Putting out the tag and mask of the buffer.
    3. Listing the links and their information.
*/
template<class T>
string
CBufferMonitor<T>::DescribeSelf()
{
  string Result;
  int count = 0;
  Result = CEventMonitor::DescribeSelf();
  if(m_Buffer.GetLen() > 0) {
    Result += "\n  Tag = ";
    char tagBuffer[20];
    sprintf(tagBuffer, "%d", m_Buffer.GetTag());
    Result += tagBuffer;

    Result += "\n  Link info:";
    if(m_lLinks.size() > 0) {
      for(LinkIterator It = beginLinks(); It != endLinks(); It++) {
	Result += "\n    Link #";
	char countBuffer[256];
	sprintf(countBuffer, "%d", ++count);
	Result += countBuffer;
	Result += "\n      Tag =    ";
	char tagbuf[256], maskbuf[256], linkbuf[256];
	sprintf(tagbuf, "%d", (*It).Tag);
	Result += tagbuf;
	Result += "\n      Mask =   ";
	sprintf(maskbuf, "%d", (*It).Mask);
	Result += maskbuf;
	Result += "\n      URL =    ";
	Result += (*It).URL;
	Result += "\n      Linkid = ";
	sprintf(linkbuf, "%d", (*It).linkid);
	Result += linkbuf;
      }
    }
    else
      Result += "\n    No links currently exist";
  }
  else
    Result += "\n  There is no buffer";

  return Result;
}


#endif
