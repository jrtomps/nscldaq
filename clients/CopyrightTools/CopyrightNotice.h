#ifndef __COPYRIGHTNOTICE_H
#define __COPYRIGHTNOTICE_H

#ifndef __CPP_IOSTREAM_H
#include <iostream.h>
#define __CPP_IOSTREAM_H
#endif

/*!
  Generate simple copyright and authorship notices.
  Copyright notices are intended for interactive output.
  Authorship notices are intended to acknowledge and ego boost.
 */  
class CopyrightNotice
{
public:
  static void  Notice(ostream& out,  const char* program, 
		      const char* version,  const char* year);
  static void  AuthorCredit(ostream& out, char* program, ...);

}; 

#endif
