#ifndef __TCLEXTENSION_H
#define __TCLEXTENSION_H

#ifndef __CDAQTCLPROCESSOR_H
#include <CDAQTCLProcessor.h>
#endif


class TCLExtension : public CDAQTCLProcessor
{

public:
  TCLExtension(CTCLInterpreter* pInterp);
  virtual int  operator() (CTCLInterpreter &rInterpreter, CTCLResult &rResult, 
			   int nArguments, char *pArguments[]);

};

#endif
