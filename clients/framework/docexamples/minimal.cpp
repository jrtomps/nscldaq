#include <iostream.h>
#include <spectrodaq.h>
#include <SpectroFramework.h>

class MyClass : public DAQROCNode
{
protected:
  int operator()(int argc, char** pargv);
};

int
MyClass::operator()(int argc, char** pargv)
{
  cout << "Hello World." << endl;
}

MyClass theApp;
