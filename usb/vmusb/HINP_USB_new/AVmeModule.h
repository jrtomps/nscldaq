#ifndef __AVMEMODULE_H
#define __AVMEMODULE_H
                                      
#include <string>

#include <CVMEInterface.h>
#include <SBSBit3API.h>

class AVmeModule      
{ 
private:
  std::string	m_sName;
  int		m_nCrate;
  int		m_nSlot;
  long		m_lBase;
  int*	        m_pDevice;
  
public:
  // Constructors, destructors and other cannonical operations: 
  	AVmeModule ( std::string name, unsigned int slot, long base);
  	AVmeModule ( std::string name, unsigned int slot);
  	AVmeModule ( std::string name, unsigned int crate, unsigned int slot, long base);
  	AVmeModule ( std::string name, unsigned int crate, unsigned int slot);
	AVmeModule(const AVmeModule& rhs); //!< Copy constructor.
	~AVmeModule ( );	//!< Destructor.
  
	AVmeModule& operator= (const AVmeModule& rhs); //!< Assignment
	int           operator==(const AVmeModule& rhs) const; //!< Comparison for equality.
	int         operator!=(const AVmeModule& rhs) const {
	return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  std::string getName() const {
    return m_sName;
  }
  
  int getSlot() const {
    return m_nSlot;
  }
  
  long getBase() const {
  	return m_lBase;
  }
  
  // Mutators:
protected:  
  
  // Class operations:
public:
  
  // Additional useful functions:
  
  static bool	ValidSlot(unsigned int slot);
  long*		Map(long offset, long length);

};


#endif
