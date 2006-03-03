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

#include "CDocumentedPacketManager.h"  
#include "CDocumentedPacket.h"
#include <CDuplicateSingleton.h>
#include <CNoSuchObjectException.h>
#include <algorithm>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


    
CDocumentedPacketManager* CDocumentedPacketManager::m_ptheInstance = 0; //Static data member initialization

// Local classes 

/*!
    matches a packet given its name:
   */
class CPacketMatchPredicate 
{
   string m_sName;
public:
   CPacketMatchPredicate(const string& rName) :
      m_sName(rName)
   {}
   int operator()(CDocumentedPacket* lhs) 
   {
      return (*lhs == m_sName);
   }
};

	//Default constructor alternative to compiler provided default constructor
	//Association object data member pointers initialized to null association object 
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CDocumentedPacketManager  object are performed.
   We don't allow construction if m_ptheInstance is already defined.  In that
   case a duplicate singletonexception is thrown.
   
*/
CDocumentedPacketManager::CDocumentedPacketManager ()
 
{
   if(m_ptheInstance) {
      throw CDuplicateSingleton("Constructing the documened packet manager",
			      "CDocumentedPacketManager::");
   }
   m_ptheInstance = this;
} 


// Functions for class CDocumentedPacketManager

/*!
    Adds a packet definition to the list of defined packets.

	\param CDocumentedPack& rPacket

*/
void 
CDocumentedPacketManager::AddPacket(CDocumentedPacket& rPacket)  
{
   m_PacketDefinitions.push_back(&rPacket);
}  

/*!
    Removes a packet from the list.
    
    \param  rName - const string& [in] Name of the packet to remove.

*/
void 
CDocumentedPacketManager::DeletePacket(const string& rName)  
{
   DocumentationPacketIterator p = find_if(m_PacketDefinitions.begin(),
				        m_PacketDefinitions.end(),
				        CPacketMatchPredicate(rName));
   if(p == m_PacketDefinitions.end()) {
      throw CNoSuchObjectException("Deleting documented packet from manager",
				 rName);
   }
   m_PacketDefinitions.erase(p);
}  

/*!
    Returns a loop iterator pointing at the first documented packet registered.
    

*/
DocumentationPacketIterator
CDocumentedPacketManager::begin()  
{
   return m_PacketDefinitions.begin();
}  

/*!
    Returns an end loop iterator to for the
    set of packets defined.


*/
DocumentationPacketIterator
CDocumentedPacketManager::end()  
{
   return m_PacketDefinitions.end();
 
}  

/*!
    Returns the number of documented packets currently registered.


*/
unsigned int 
CDocumentedPacketManager::size()  
{
   return m_PacketDefinitions.size();
}  

/*!
    Returns the set of descriptions of each registered
    packet in a format suitable for inclusion in a data
    buffer.
    

*/
vector<string> 
CDocumentedPacketManager::Format()  
{
   vector<string> Result;
   DocumentationPacketIterator i = begin();
   while(i != end()) {
      Result.push_back((*i)->Format());
      i++;
   }
   return Result;
}  

/*!
    Returns the singleton instance pointer.
*/
CDocumentedPacketManager* 
CDocumentedPacketManager::getInstance()  
{
   // If there is no instance, it must be created:
   
   if(!m_ptheInstance) 
      m_ptheInstance = new CDocumentedPacketManager;
   
   return m_ptheInstance;
}
