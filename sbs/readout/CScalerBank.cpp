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
#include "CScalerBank.h"

using namespace std;

// Utility classes


// Visitor to initialize all children.

class InitializeVisitor : public CScalerBank::CVisitor
{
public:
  virtual void operator()(CScaler* pItem) {
    pItem->initialize();
  }
};

// Visitor to clear children:

class ClearVisitor : public CScalerBank::CVisitor
{
public:
  virtual void operator()(CScaler* pItem) {
    pItem->clear();
  }
};

// Visitor to disable children:

class DisableVisitor : public CScalerBank::CVisitor
{
public:
  virtual void operator()(CScaler* pItem) {
    pItem->disable();
  }
};

// Visitor to read children:

class ReadVisitor : public CScalerBank::CVisitor
{
private:
  vector<uint32_t> m_values;
  
public:
  virtual void operator()(CScaler* pItem) {
    vector<uint32_t> values = pItem->read();
    m_values.insert(m_values.end(),
		    values.begin(), values.end());
  }
  vector<uint32_t> getScalers() const
  {
    return m_values;
  }
};

  // Members inherited from CScaler:

/*!
  Deep initiailization of the modules:
*/
void 
CScalerBank::initialize()
{
  InitializeVisitor i;
  visit(i);
}

/*!
  Deep initialization of the modules
*/
void 
CScalerBank::clear()
{
  ClearVisitor v;
  visit(v);
}
/*!
  Deep disable of the hierarchy:
*/
void 
CScalerBank::disable()
{
  DisableVisitor v;
  visit(v);
}
/*!
  Read the data from the scaler modules.
  Return it as a vector of int32's.
*/
std::vector<uint32_t> 
CScalerBank::read()
{
  ReadVisitor v;
  visit(v);
  return v.getScalers();

}


// Type safe adapter to CComposite:


/*!
  Add a scaler module to the collection:
  \param pScaler - Pointer to the module to add.

  \note The clients are resposible for any storage management
        if pScaler points to a dynamically allocated object.

*/
void 
CScalerBank::AddScalerModule(CScaler* pScaler)
{
  CComposite::addItem(pScaler);
}
/*!
   Delete a scaler from the collection.
   \param pScaler - Points to the item delete.

   \note It is up to upper layers of software to worry
        about handling storage management in the case when
	pScaler was dynamically allocated.
*/
void 
CScalerBank::DeleteScaler(CScaler* pScaler)
{
  CComposite::deleteItem(pScaler);
}

/*!
   Returns a begin iteration to the container.
*/
CScalerBank::ScalerIterator 
CScalerBank::begin()
{
  CScalerBank::ScalerIterator p = CComposite::begin();
  return p;
}
/*!
  Returns an end iteration to the container of scalers:
*/
CScalerBank::ScalerIterator 
CScalerBank::end()
{
  CScalerBank::ScalerIterator p = CComposite::end();
  return p;
}


/*!
   This is a composite:
*/
bool
CScalerBank::isComposite() const
{
  return true;
}


/*!
  Vists every element in the collection.  Does not recurse.
  It's up tothe caller to arrange recursion if needed.
  Note that usually the member functions of the collections
  handle recursion.

  \param visitor - A reference to a function object that is called once
                   for each element of the collection. The function object
		   is passed a pointer to the object being visited as a
		   sole parameter.  The pointer will be of type CScaler*.

Here's a simple example of counting elements in the collection with recursion;
Using this is equivalent to fullCount():


\verbatim
class counter : public CScalerBank::Visitor {
  int  m_count;
public:
  CScalerBank() : m_count(0) {}
  int getCount() {return m_count; }
  virtual void operator()(CScaler* pObject) {
     m_count++;
     if (pObject->isComposite()) {
       CScalerBank* pBank = dynamic_cast<CScalerBank*>(pObject);
       counter nextDown;
       pBank->visit(nextDown);
       m_count += nextDown.getCount();
     }
  }
};

...

// Count the number of items in CScalerBank bank:

counter totalCount;
bank.visit(totalCount);
cerr << "There are " << totalCount.getCount() << "elements\n";

\endverbatim
*/
void 
CScalerBank::visit(CScalerBank::CVisitor& visitor)
{
  ScalerIterator p = begin();
  while(p != end()) {
    CScaler* pScaler = dynamic_cast<CScaler*>(*p);
    visitor(pScaler);

    p++;
  }
}
