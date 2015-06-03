
#ifndef CSYSTEMCONTROL_H
#define CSYSTEMCONTROL_H

#include <config.h>
#include <string>
#include <TCLObject.h>
#include <memory>

class CTCLInterpreter;
struct Tcl_Interp;
struct Tcl_Event;

class CBeginRun;
class CEndRun;
class CPauseRun;
class CResumeRun;
class CInit;
class CExit;

// This class encapsulates and embodies the run control operations.
class CSystemControl
{
  private:
    CTCLInterpreter*     m_pInterpreter;
    Tcl_ThreadId         m_threadId;

    static std::string   m_initScript;
    // various commands that will be registered in the main tcl interpreter
    // we make them unique_ptrs so that their destructors get called at
    // program completion.
    static std::unique_ptr<CBeginRun>  m_pBeginRun;
    static std::unique_ptr<CEndRun>    m_pEndRun;
    static std::unique_ptr<CPauseRun>  m_pPauseRun;
    static std::unique_ptr<CResumeRun> m_pResumeRun;
    static std::unique_ptr<CInit>      m_pInit;
    static std::unique_ptr<CExit>      m_pExit;

  public:
    void run(int argc, char** argv);
    void setInitScript(const std::string& path);

    static int AcquisitionErrorHandler(Tcl_Event* pEvent, int flags);
    static int scheduleExit(int status);
    static int tclExit(Tcl_Event* pEvent, int flags);

  private:
    static int AppInit(Tcl_Interp* interp);
    static std::string makeConfigFile(std::string baseName);
    static std::string destinationRing(const char* pRingName);

    static CTCLObject makeCommand(CTCLInterpreter* pInterp, 
                                  const char* verb, 
                                  std::string argument);
};
#endif
