/*  Implement the Copyright notice members:
 */
#include <config.h>
#include "CopyrightNotice.h"
#include <stdarg.h>

void
CopyrightNotice::Notice(ostream& out, const char* program, 
			const char* version, const char* year)
{
  out << program << " version " << version;
  out << " (C) Copyright 2002 Michigan State University all rights reserved\n";
  out << program << " comes with ABSOLUTELY NO WARRANTY;\n";
  out << "This is free software, and you are welcome to redistribute it\n";
  out << "under certain conditions; See http://www.gnu.org/licenses/gpl.txt\n";
  out << "for details\n";
  
}

/*!
    Puts out an author credit message.  The variable list of arguments
    is a set of char* strings that must be terminated by a NULL pointer.
*/
void
CopyrightNotice::AuthorCredit(ostream& out, char* program, ...)
{
  va_list ap;
  va_start(ap, program);
  char* pAuthor;

  out << program << " was written by:\n\n";
  pAuthor = (char*)va_arg(ap, char*);
  //  while(pAuthor != (char*)0) {
    out << pAuthor << endl;
    pAuthor = (char*)va_arg(ap, char*);
    //  }
  va_end(ap);
}
