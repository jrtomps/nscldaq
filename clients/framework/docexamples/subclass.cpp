#include <iostream.h>
#include <stdio.h>
#include <spectrodaq.h>
#include <SpectroFramework.h>

class Echo : public CFileEvent
{
public:
  Echo(int fd, const char* pName);
  virtual void OnReadable(istream& rin);
};

Echo::Echo(int fd, const char* pName):
  CFileEvent(fd, pName)
{
  AppendClassInfo();
}

void
Echo::OnReadable(istream& rin)
{
  CFileEvent::OnReadable(rin);
  string word;
  rin >> word;
  cout << word << endl;
}

class MyApp : public DAQROCNode
{
protected:
  virtual int operator()(int argc, char** argv);
};

int
MyApp::operator()(int argc, char** argv)
{
  Echo echo(fileno(stdin), "EchoProcessor");

  echo.Enable();
  DAQThreadId id = echo.getThreadId();

  Join(id);			// Wait for echo to exit.
}


MyApp theapp;
