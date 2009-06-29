/*=========================================================================*/
// BagItem.h:
//
// Author:
//		Eric Kasten
//		NSCL
//		Michigan State University
//		East Lansing, MI 48824-1321
//		mailto:kasten@nscl.msu.edu
//
// Copyright NSCL 1998, All rights reserved.

#ifndef BAGITEM_H
#define BAGITEM_H 

#include <daqconfig.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <mainexterns.h>
#include <DAQObject.h>

/*=====================================================================*/
template <class DAQOBJ,int ID>
class BagItem: public DAQObject {
  public:
    BagItem(DAQOBJ *b) {SetType(ID);myitem = b;nxt = NULL,prv = NULL;}; // Constructor
    ~BagItem() {};  // Destructor
    DAQOBJ *Get() {return(myitem);}; // Get item encapsulated by this item 
    BagItem *GetNext() {return(nxt);};  // Get next bag item
    BagItem *GetPrev() {return(prv);};  // Get prev bag item
    void *SetNext(BagItem *b) {nxt = b;};  // Set the next bag item
    void SetPrev(BagItem *b) {prv = b;};   // Set the prev bag item
  protected:
    BagItem() {SetType(ID);};  // Default constructor
  private:
    DAQOBJ *myitem;  // Object encapsulated by this bag item
    BagItem *nxt;  // Next bag item
    BagItem *prv;  // Prev bag item
};

#endif
