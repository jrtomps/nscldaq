#include <iostream.h>
#include "CNamedObject.h"
#include "CObjectRegistry.h"
#include "CClassifiedObjectRegistry.h"
#include "CTestClasses.h"
#include "CDuplicateNameException.h"
#include "CNoSuchObjectException.h"
#include <assert.h>
#include <map>

int main()
{
  typedef map<string, CNamedObject*>::iterator ObjectIterator;

  cout << "Testing Registration class hierarchy operations..." << endl;
  // Create some CNamedObject's to register
  CNamedObject Obj1("Object1"),
    Obj2("Object2"),
    Obj3("Object3"),
    Obj4("Object4"),
    Obj5("Object5"),
    Obj6("Object6"),
    Obj7("Object7"),
    Generic0, Generic1;

  cout << "Testing generic object naming:" << endl;
  cout << Generic0.DescribeSelf() << endl;
  cout << Generic1.DescribeSelf() << endl;

  // Create a CObjectRegistry
  CObjectRegistry registry("Registry1");
  cout << "ObjectRegistry has the following attributes:" << endl;
  cout << "Name: " << registry.getName() << endl << endl;
  // Add some NamedObject's to the registry
  try {
    registry.Add(Obj1);
    registry.Add(Obj2);
  }
  catch(CDuplicateNameException& e) {
    cerr << "Failed to add object to registry" << endl;
    cerr << e.ReasonText() << endl;
  }

  // Try the Find() function
  ObjectIterator It;
  try {
    It = registry.Find("Object1");
  }
  catch(CNoSuchObjectException& e) {
    cerr << "Failed to find object in registry" << endl;
    cerr << e.ReasonText() << endl;
  }
  cout << "Found object named: " << ((*It).second)->getName() << endl;
  cout << "Description is:     " << ((*It).second)->DescribeSelf() << "\n\n";

  // Add some more named objects to the registry
  try {
    registry.Add(Obj3);
    registry.Add(Obj4);
  }
  catch (CDuplicateNameException& dne) {
    cerr << "Failed to add object to registry" << endl;
    cerr << dne.ReasonText() << endl << endl;
  }

  // Try the Remove() function
  try {
    registry.Remove(Obj3);
    registry.Remove("Object8");
  }
  catch (CNoSuchObjectException& nsoe) {
    cerr << "Failed to remove object from registry" << endl;
    cerr << nsoe.ReasonText() << endl;
  }
  registry.Add(Obj3);

  // Demonstrate the CObjectRegistry::DescribeSelf() function
  cout << "\nDemonstrating CObjectRegistry::DescribeSelf():\n";
  cout << registry.DescribeSelf() << endl;

  // Create a CClassifiedObjectRegistry
  CClassifiedObjectRegistry classifiedRegistry("myClassified");
  
  string reg2 = "Registry2",
    reg3 = "Registry3",
    reg4 = "Registry4";

  // Add some registries to the ClassifiedObjectRegistry using Create()
  classifiedRegistry.CreateRegistry(reg2);
  classifiedRegistry.CreateRegistry(reg3);
  classifiedRegistry.CreateRegistry(reg4);

  // Try adding some objects to registry reg2
  try {
    classifiedRegistry.Add(reg2, Obj2);
    classifiedRegistry.Add(reg2, Obj3);
    classifiedRegistry.Add(reg2, Obj4);
  }
  catch (CDuplicateNameException& dne) {
    cerr << "Failed to add object to registry" << endl;
    cerr << dne.ReasonText() << endl << endl;
  }

  // Try adding some objects to registry reg3
  try {
    classifiedRegistry.Add(reg3, Obj3);
    classifiedRegistry.Add(reg3, Obj5);
    classifiedRegistry.Add(reg3, Obj7);
  }
  catch (CDuplicateNameException& dne) {
    cerr << "Failed to add object to registry" << endl;
    cerr << dne.ReasonText() << endl << endl;
  }

  // Try adding some objects to registry reg4
  try {
    classifiedRegistry.Add(reg4, Obj1);
    classifiedRegistry.Add(reg4, Obj6);
    classifiedRegistry.Add(reg4, Obj7);
    classifiedRegistry.Add(reg4, Obj3);
    classifiedRegistry.Add(reg4, Obj1);
  }
  catch (CDuplicateNameException& dne) {
    cerr << "Failed to add object to registry" << endl;
    cerr << dne.ReasonText() << endl;
  }

  // Demonstrate the CClassifiedObjectRegistry::DescribeSelf() function
  cout << "\nDemonstrating CClassifiedObjectRegistry::DescribeSelf():\n";
  cout << classifiedRegistry.DescribeSelf() << endl;

  // Try removing an object from the ClassifiedObjectRegistry using Remove()
  try {
    classifiedRegistry.Remove(reg4, Obj6);
  }
  catch (CNoSuchObjectException& nsoe) {
    cerr << "Failed to remove object from registry" << endl;
    cerr << nsoe.ReasonText() << endl << endl;
  }

  // Try finding some objects in specific registries using
  // Find(string& RegistryName, const string& RegistryName)
  try { 
    ObjectIterator It1 = classifiedRegistry.Find("Registry2", "Object1");
    ObjectIterator It2 = classifiedRegistry.Find("Registry2", "Object7");
  }
  catch (CNoSuchObjectException& nsoe) {
    cerr << "Failed to find object in registry" << endl;
    cerr << nsoe.ReasonText() << endl;
  }

  /////////////////////////////////////////////////////////////
  //  Testing the test classes derived from CNamedObject
  //  All of these classes have either been derived from
  //  CNamedObject, or from some other class which was
  //  derived from CNamedObject. This section tests the
  //  DescribeSelf function, and the ability of objects
  //  to register themselves at construction.
  //

  cout << "\nPress return to test some contrived classes derived ";
  cout << "from CNamedObject...";
  for(int i = 0; i == 0; i++) {
    cin.ignore(1024, '\n');
    if(cin.eof()) break;
  }
  cout << endl;
  CClassifiedObjectRegistry newClassRegistry("newClassRegistry");
  
  // Note that objects are automatically registered upon creation
  CTestEvent testevent("myTestEventObject", newClassRegistry);
  CTestEventMonitor testeventmonitor
    ("myTestEventMonitorObject", newClassRegistry);
  CDerivedFromTestEvent dftestevent("mydfTestEventObject", newClassRegistry);
  CDerivedFromTestEventMonitor dftesteventmonitor
    ("mydfTestEventMonitorObject", newClassRegistry);
  CDerivedFromDerivedFromTestEvent dfdftestevent
    ("mydfdfTestEventObject", newClassRegistry);

  cout << "Demonstrating CClassifiedObjectRegistry::DescribeSelf()" << endl;
  cout << newClassRegistry.DescribeSelf() << endl;

  // Test the DeleteRegistry function
  try {
    newClassRegistry.DeleteRegistry(testevent.getRegistry());
    newClassRegistry.DeleteRegistry(testeventmonitor.getRegistry());
  }
  catch (CNoSuchObjectException& nsoe) {
    cerr << "Failed to find object in registry" << endl;
    cerr << nsoe.ReasonText() << endl;    
  }

  cout << "After deleting registries, DescribeSelf():" << endl;
  cout << newClassRegistry.DescribeSelf() << endl;
  cout << endl;
}
