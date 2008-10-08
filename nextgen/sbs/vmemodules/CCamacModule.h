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

   

//////////////////////////CCamacModule.h file//////////////////////////////////

#ifndef __CCAMACMODULE_H  
#define __CCAMACMODULE_H
                                      
                               
/*!
   Encapsulates a generic camac module.
   CAMAC modules are assumed to live inside of
   the VME space through the CES/CBD 8210.
   A Camac module is geographicall defined in terms of
   its Branch Crate and Slot.
   More specialized modules can be generated and can
   cache pointers to time critical functions using the MakePointer
   member function.
 */		
class CCamacModule      
{ 
private:
  unsigned int   m_nBranch;	//!< Branch module lives in (0-7)
  unsigned short m_nCrate;	//!< Crate module lives in (1-7)
  int            m_nSlot;	//!< Slot module lives in. 24-31 are controller
  unsigned long* m_pBase;	//!< Base address of module.
  
public:
  // Constructors, destructors and other cannonical operations: 
  
  CCamacModule (unsigned int branch,
		unsigned int crate,
		unsigned int slot); //!< Default constructor.
  CCamacModule(const CCamacModule& rhs); //!< Copy constructor.
  ~ CCamacModule ( ) { }	//!< Destructor.
  
  CCamacModule& operator= (const CCamacModule& rhs); //!< Assignment
  int           operator==(const CCamacModule& rhs) const; //!< Comparison for equality.
  int         operator!=(const CCamacModule& rhs) const {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  unsigned int getBranch() const {
    return m_nBranch;
  }
  
  unsigned short getCrate() const {
    return m_nCrate;
  }
  
  int getSlot() const {
    return m_nSlot;
  }
  
  unsigned long* getBase() const {
    return m_pBase;
  }
  
  // Mutators:
protected:  
  
  // Class operations:
public:
  
  unsigned long Read (unsigned int f, unsigned int a)  const;
  void          Write (unsigned int f, unsigned int a, unsigned long d)  const;
  unsigned short Control (unsigned int f, unsigned int a)  const;
  unsigned long* MakePointer (unsigned int f, unsigned int a, 
			      bool isshort=false)  const;
  unsigned long* MakePointer (unsigned int c, unsigned int n, unsigned int a,
			      unsigned int f, bool isshort = false)  const;

  // Additional useful functions:
  
  static bool ValidBranch(unsigned int branch);
  static bool ValidCrate(unsigned int crate);
  static bool ValidSlot(unsigned int slot);
  static bool ValidSubaddress(unsigned int a);
  static bool ValidFunction(unsigned int f);
  static bool isRead(unsigned int f);
  static bool isWrite(unsigned int f);
  static bool isControl(unsigned int f);
  
};

#endif
