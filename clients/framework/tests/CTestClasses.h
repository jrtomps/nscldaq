// TestClasses.h
//
// Defines some contrived classes which have
// been derived from CNamedObject, or from
// some other class which was derived from
// CNamedObject. This file is for purposes
// of testing the registration files only.
//
/////////////////////////////////////////////////////////////////////////

#ifndef __CTESTEVENT_H
#define __CTESTEVENT_H

#ifndef __CNAMEDOBJECT_H
#include "CNamedObject.h"
#endif

class CTestEvent : public CNamedObject
{
  static const char* TestEventRegistry("TestEvent");
  static const char* TestEventMonitorRegistry("TestEventMonitor");
  const string m_Registry;

 public:

  CTestEvent(string am_sName, CClassifiedObjectRegistry& rRegistry) :
    CNamedObject(am_sName),
    m_Registry(TestEventRegistry)
    { 
      AppendClassInfo(); 
      rRegistry.CreateRegistry(m_Registry);
      rRegistry.Add(m_Registry, *this);
    }

  ~CTestEvent() { }

public:
  
  string getRegistry() const
    {
      return m_Registry;
    }
};

#endif

#ifndef __CTESTEVENTMONITOR
#define __CTESTEVENTMONITOR

#ifndef __CNAMEDOBJECT_H
#include "CNamedObject.h"
#endif

class CTestEventMonitor : public CNamedObject
{
  const string m_Registry;
  
 public:

  CTestEventMonitor(string am_sName, 
		    CClassifiedObjectRegistry& rRegistry) :
    CNamedObject(am_sName),
    m_Registry(TestEventMonitorRegistry)
    { 
      AppendClassInfo();
      rRegistry.CreateRegistry(m_Registry);
      rRegistry.Add(m_Registry, *this);
    }

  ~CTestEventMonitor() { }

public:
  
  string getRegistry() const
    {
      return m_Registry;
    }
};

#endif

#ifndef __CDERIVEDFROMTESTEVENT_H
#define __CDERIVEDFROMTESTEVENT_H

#ifndef __CNAMEDOBJECT_H
#include "CNamedObject.h"
#endif

class CDerivedFromTestEvent : public CTestEvent
{
  const string m_Registry;

 public: 
  
  CDerivedFromTestEvent(string am_sName,
		        CClassifiedObjectRegistry& rRegistry) :
    CTestEvent(am_sName, rRegistry),
    m_Registry(TestEventRegistry)
    { 
      AppendClassInfo(); 
    }

  ~CDerivedFromTestEvent() { }  

public:
  
  string getRegistry() const
    {
      return m_Registry;
    }
};

#endif

#ifndef __CDERIVEDFROMTESTEVENTMONITOR_H
#define __CDERIVEDFROMTESTEVENTMONITOR_H

#ifndef __CNAMEDOBJECT_H
#include "CNamedObject.h"
#endif

class CDerivedFromTestEventMonitor : public CTestEventMonitor
{
  const string m_Registry;

 public:

  CDerivedFromTestEventMonitor(string am_sName, 
			       CClassifiedObjectRegistry& rRegistry) :
    CTestEventMonitor(am_sName, rRegistry),
    m_Registry(TestEventMonitorRegistry)
    { 
      AppendClassInfo();
    }

  ~CDerivedFromTestEventMonitor() { }

public:
  
  string getRegistry() const
    {
      return m_Registry;
    }
};

#endif

#ifndef __CDERIVEDFROMDERIVEDFROMTESTEVENT_H
#define __CDERIVEDFROMDERIVEDFROMTESTEVENT_H

#ifndef __CNAMEDOBJECT_H
#include "CNamedObject.h"
#endif

class CDerivedFromDerivedFromTestEvent : public CDerivedFromTestEvent
{
  const string m_Registry;

 public:

  CDerivedFromDerivedFromTestEvent(string am_sName, 
				   CClassifiedObjectRegistry& rRegistry) :
    CDerivedFromTestEvent(am_sName, rRegistry),
    m_Registry(TestEventRegistry)
    { 
      AppendClassInfo(); 
    }

  ~CDerivedFromDerivedFromTestEvent() { }

public:
  
  string getRegistry() const
    {
      return m_Registry;
    }
};

#endif
