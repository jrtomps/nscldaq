/*
   This file is a 'null' vme interface api.  It simply throws
   a string exception indicating that there is no VME interface on the
   system whenever any entry point is called.
*/

#include <config.h>
#include <CVMEInterface.h>
#include <string>



#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

const char* CVMEInterface::m_szDriverName = "NULL";

/// No comments required since none of these really do anything useful.. just
// blow the caller away by throwing exceptions.

void*
CVMEInterface::Open(AddressMode nMode,
		    unsigned short crate)
{
  throw string("NULL VME device driver Open called");
}

void CVMEInterface::Close(void* pDeviceHandle)
{
  throw string("NULL VME Device driver Close called");

}

void* CVMEInterface::Map(void* pDeviceHandle,
	    unsigned long nBase, 
	    unsigned long nBytes)
{
  throw string("NULL VME Device driver Map called");
}

void CVMEInterface::Unmap(void* pDeviceHandle,
	     void*  pBase,
	     unsigned long lBytes)
{
  throw string("NULL VME Device driver Unmap called");
}

int CVMEInterface::Read(void* pDeviceHandle,
			unsigned long nOffset,
			void*  pBuffer,
			unsigned long nBytes)
{
  throw string("NULL VME Device driver Read called");
}

int CVMEInterface::Write(void* pDeviceHandle,
		   unsigned long nOffset,
		   void* pBuffer,
		   unsigned long nBytes)
{
  throw string("NULL VME Device driver Write called");
}
