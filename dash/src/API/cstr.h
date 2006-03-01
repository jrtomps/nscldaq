#ifndef DAQHWYAPI_CSTR_H
#define DAQHWYAPI_CSTR_H 

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

namespace daqhwyapi {

#define MAX_SCRATCHPAD_AREA 1024
#define MAX_CSTR MAX_SCRATCHPAD_AREA

extern char daqhwyapi_work_area[];

#ifndef __LINE__
#ifndef __FILE__
  #define _CREATE_CSTR daqhwyapi_create_cstr(char *str,char *msg)
  #define CSTR(s) #s
  #ifdef HAVE_SNPRINTF
    #define CSTR_FMT snprintf(str,MAX_CSTR,"%s (pid=%d)" ,msg,getpid())
  #else
    #define CSTR_FMT sprintf(str,"%s (pid=%d)",msg,getpid())
  #endif
#endif /* ifndef __FILE__ */ 
  #define _CREATE_CSTR daqhwyapi_create_cstr(char *str,char *msg,char *fnam)
  #define CSTR(s) daqhwyapi_create_cstr(daqhwyapi_work_area,s,__FILE__)
  #ifdef HAVE_SNPRINTF
    #define CSTR_FMT snprintf(str,MAX_CSTR,"%s (\"%s\" pid=%d)",msg,fnam,getpid())
  #else
    #define CSTR_FMT sprintf(str,"%s (\"%s\" pid=%d)",msg,fnam,getpid())
  #endif
#else  /* ifndef __LINE__ */
  #define _CREATE_CSTR daqhwyapi_create_cstr(char *str,char *msg,char *fnam,int lnum)
  #define CSTR(s) daqhwyapi_create_cstr(daqhwyapi_work_area,s,__FILE__,__LINE__)
  #ifdef HAVE_SNPRINTF
    #define CSTR_FMT snprintf(str,MAX_CSTR,"%s (line %d of \"%s\" pid=%d)",msg,lnum,fnam,getpid())
  #else
    #define CSTR_FMT sprintf(str,"%s (line %d of \"%s\" pid=%d)",msg,lnum,fnam,getpid())
  #endif
#endif  /* ifndef __FILE__ */

static inline char * _CREATE_CSTR
{
  static char *garbage = " <!!NULL string passed to daqhwyapi_create_cstr()!!> ";
  if (str == NULL) return(garbage);
  CSTR_FMT;
  return(str);
}

} // namespace daqhwyapi

#endif
