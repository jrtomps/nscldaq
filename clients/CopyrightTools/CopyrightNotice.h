#ifndef __COPYRIGHTNOTICE_H
#define __COPYRIGHTNOTICE_H

/// Requires include of config.h by parent.

#ifndef __OSTREAM_DAQH
#include <Ostream.h>
#endif


/*!
  Generate simple copyright and authorship notices.
  Copyright notices are intended for interactive output.
  Authorship notices are intended to acknowledge and ego boost.
 */  
class CopyrightNotice
{
public:
  static void  Notice(STD(ostream)& out,  const char* program, 
		      const char* version,  const char* year);
  static void  AuthorCredit(STD(ostream)& out, char* program, ...);

}; 

#endif
