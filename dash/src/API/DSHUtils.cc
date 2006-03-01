/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <time.h>
#include <ctype.h>

#ifndef DAQHWYAPI_DSHUTILS_H
#include <dshapi/DSHUtils.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_STRINGARRAY_H
#include <dshapi/StringArray.h>
#endif

#ifndef DAQHWYAPI_INTARRAY_H
#include <dshapi/IntArray.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

#ifndef DAQHWYAPI_STRINGUTILS_H
#include <dshapi/StringUtils.h>
#endif

#ifndef DAQHWYAPI_FSUTILS_H
#include <dshapi/FSUtils.h>
#endif

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

namespace daqhwyapi {
/**
* @var dshutils_rtexception
* @brief Exception to throw for runtime exceptions.
*
* Exception to throw for runtime exceptions.
*/
static RuntimeException dshutils_rtexception;

#define RUN_FILE_REGX "run*_0000.evt"
#define ARRAY_DUMP_CHARCOUNT 29

// Is a character an octal character?
static inline bool isoctal(char c) {
  return ((c >= '0')&&(c <= '7'));
}

// Is a character a hex character?
static inline bool ishex(char c) {
  if (((c >= '0')&&(c <= '9'))||
      ((c >= 'a')&&(c <= 'z'))||
      ((c >= 'A')&&(c <= 'Z'))) {
    return true;
  }
  return false;
}

// Convert a octal character to an octal value
static inline int octalvalue(char c) {
  return (c - '0');
} 

// Convert a hex character to a hex value
static inline int hexvalue(char c) {
  if ((c >= '0')&&(c <= '9')) return (c - '0');
  if ((c >= 'a')&&(c <= 'f')) return (c - 'a' + 10);
  if ((c >= 'A')&&(c <= 'F')) return (c - 'A' + 10);
  return 0;
} 

// Index and value structure for sorting
typedef struct indexandvalue {
  int val;
  int idx;
} indexandvalue_t;

// Index and time_t structure for sorting
typedef struct indexandtime {
  time_t val;
  int idx;
} indexandtime_t;

// Comparison function for passing to qsort and 
// sorting indexandvalue arrays.
static int indexandvalue_compare(const void *v1,const void *v2) {
  indexandvalue_t *nv1 = (indexandvalue_t*)v1;
  indexandvalue_t *nv2 = (indexandvalue_t*)v2;
  return (nv1->val - nv2->val); 
}

// Comparison function for passing to qsort and 
// sorting indexandtime arrays.
static int indexandtime_compare(const void *v1,const void *v2) {
  indexandtime_t *nv1 = (indexandtime_t*)v1;
  indexandtime_t *nv2 = (indexandtime_t*)v2;
  return (nv1->val - nv2->val); 
}

// Comparison function for passing to qsort and 
// sorting int arrays.
static int int_compare(const void *v1,const void *v2) {
  return ((*((int*)v1)) - (*((int*)v2))); 
}

} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn DSHUtils::DSHUtils()
* @brief Default constructor.
*                                        
* Default concstructor.
*                                         
* @param None
* @return this
*/      
DSHUtils::DSHUtils() { }

/*===================================================================*/
/** @fn bool DSHUtils::sortFilesByRunNumber(StringArray& files)
* @brief Sort a list of run files by run number.
*                                        
* Sort a list of run files by run number in ascending order.
*                                         
* @param files The files names to sort.
* @return True if sorting was successful, false otherwise.
* @throw RuntimeException If the run number cannot be parsed.
*/      
bool DSHUtils::sortFilesByRunNumber(StringArray& files) { 
  if (files.length == 0) return false;

  // First create and value/index set for sorting
  indexandvalue_t *validx = new indexandvalue_t[files.length];
  for (int i = 0; i < files.length; i++) {
    char *pstart = (char*)(files.elements[i]->c_str());  
    pstart += 3;  // skip over "run"
    char *pend = strchr(pstart,'_'); // Find the end of the run number
    if (pend == NULL) return false;  // Does not look like a run file 

    // Get rid of leading zeros so we do not parse it as octal
    while (pstart != pend) {
      if (*pstart != '0') break;
      pstart++;
    }

    // Construct a string for parsing
    String rstr;
    rstr.append(pstart,(pend-pstart));

    // Parse the string into an integer.
    int rnum = StringUtils::parseInteger(rstr);
 
    // Add to the index/value array
    validx[i].val = rnum;
    validx[i].idx = i;
  } 

  // Now we can sort the index/value array
  qsort((void*)validx,files.length,sizeof(indexandvalue_t),indexandvalue_compare);

  // Put the files in sorted order.
  StringArray warry(files.length);
  for (int i = 0; i < files.length; i++) {
    warry.elements[i] = files.elements[validx[i].idx];
  } 

  // Put them back in the original array
  for (int i = 0; i < files.length; i++) {
    files.elements[i] = warry.elements[i];
  } 

  // Delete the value/index array
  if (validx != NULL) delete[] validx;
  validx = NULL;

  // Everythings is Ok.
  return true;
}

/*===================================================================*/
/** @fn bool DSHUtils::getDirectoryRunNumbers(IntArray& runnumbers,String& dir)
* @brief Scan a directory for event files and return the run numbers.
*                                        
* Scan a directory for event files and return the run number.  The
* run numbers will be sorted in ascending order.
*                                         
* @param runnumbers Output.  The run numbers found in the directory.
* @param dir The directory to scan.
* @return True if scanning was successful, false otherwise.
* @throw RuntimeException If the run number cannot be parsed.
*/      
bool DSHUtils::getDirectoryRunNumbers(IntArray& runnumbers,String& dir) { 
  // We want to get the first segment of each run in the directory
  String rx(RUN_FILE_REGX);

  // A work array for examining the directory list
  StringArray wlist;

  // Get a filtered directory list.  This list is returned
  // alpha sorted.
  FSUtils::directoryList(wlist,dir,rx);

  // Resive the output array.
  runnumbers.renew(wlist.length);

  // Extract the run numbers from the file list.
  for (int i = 0; i < wlist.length; i++) {
    char *pstart = (char*)(wlist.elements[i]->c_str());  
    pstart += 3;  // skip over "run"
    char *pend = strchr(pstart,'_'); // Find the end of the run number
    if (pend == NULL) return false;  // Does not look like a run file 

    // Get rid of leading zeros so we do not parse it as octal
    while (pstart != pend) {
      if (*pstart != '0') break;
      pstart++;
    }

    // Construct a string for parsing
    String rstr;
    rstr.append(pstart,(pend-pstart));

    // Parse the string into an integer.
    int rnum = StringUtils::parseInteger(rstr);
 
    // Add to the output array
    runnumbers.elements[i] = rnum;
  } 

  // Clear out the work list
  wlist.clearAndDelete();

  // Now we can sort the index/value array
  qsort((void*)(runnumbers.elements),runnumbers.length,sizeof(int),int_compare);

  // Everythings is Ok.
  return true;
}

/*===================================================================*/
/** @fn bool DSHUtils::getDirectoryRunCTimes(IntArray& runnumbers,String& dir)
* @brief Scan a directory for event files and return the run numbers.
*                                        
* Scan a directory for event files and return the run numbers.  The
* run numbers will be sorted in ascending order by run file creation
* time..
*                                         
* @param runnumbers Output.  The run numbers found in the directory.
* @param dir The directory to scan.
* @return True if scanning was successful, false otherwise.
* @throw RuntimeException If the run number cannot be parsed.
*/      
bool DSHUtils::getDirectoryRunCTimes(IntArray& runnumbers,String& dir) { 
  // We want to get the first segment of each run in the directory
  String rx(RUN_FILE_REGX);

  // A work array for examining the directory list
  StringArray wlist;

  // Get a filtered directory list.  This list is returned
  // alpha sorted.
  FSUtils::directoryList(wlist,dir,rx);

  // First create and value/index set for sorting
  int fcnt = wlist.length;
  indexandtime_t *validx = new indexandtime_t[fcnt];

  // Extract the run numbers from the file list.
  for (int i = 0; i < fcnt; i++) {
    char *pstart = (char*)(wlist.elements[i]->c_str());  
    pstart += 3;  // skip over "run"
    char *pend = strchr(pstart,'_'); // Find the end of the run number
    if (pend == NULL) return false;  // Does not look like a run file 

    // Get rid of leading zeros so we do not parse it as octal
    while (pstart != pend) {
      if (*pstart != '0') break;
      pstart++;
    }

    // Construct a string for parsing
    String rstr;
    rstr.append(pstart,(pend-pstart));

    // Parse the string into an integer.
    int rnum = StringUtils::parseInteger(rstr);
 
    // Add to the sortable array
    String fnam(dir);
    if (fnam.size() <= 0) fnam = "./";
    else if ((fnam.c_str())[fnam.size()-1] != '/') fnam += '/';
    fnam += (*(wlist.elements[i]));
    validx[i].idx = rnum;
    validx[i].val = FSUtils::fileCTime(fnam);
  } 

  // Clear out wlist
  wlist.clearAndDelete();

  // Now we can sort the index/value array
  qsort((void*)(validx),fcnt,sizeof(indexandtime_t),indexandtime_compare);

  // Resive the output array.
  runnumbers.renew(fcnt);

  // Add the run number is time ascending order
  for (int i = 0; i < fcnt; i++) {
    runnumbers.elements[i] = validx[i].idx;
  }

  // Delete the value/index array
  if (validx != NULL) delete[] validx;
  validx = NULL;

  // Everythings is Ok.
  return true;
}

/*===================================================================*/
/** @fn bool DSHUtils::isNumericRange(String& rangestr)
* @brief Check if a string can be parsed as a numeric range.
*                                        
* Check if a string can be parsed as a numeric range.  That is,
* given a string representation of a numeric range (e.g., 5-10 or 5,10),
* this method will return true.  Otherwise false is returned.
*                                         
* @param rangestr The string to check.
* @return True if the rangestr can be parsed as a numeric range.
*/      
bool DSHUtils::isNumericRange(String& rangestr) { 
  int len = rangestr.size();
  int pos = 0;
  const char *p = rangestr.c_str();

  // Check for length
  if (len < 3) return false; // need digit-digit format

  // Is the first character a digit?
  if (!isdigit((int)(p[pos]))) return false; 
  pos++;

  // Is everything a digit up to a '-' or ','?
  for (int i = pos; i < len; i++) {
    if (!isdigit((int)(p[pos]))) break;
    pos++;
  }

  // If we have some characters left, the current character should be 
  // a '-' or a ','
  if (pos >= (len-1)) return false;
  if ((p[pos] != '-')&&(p[pos] != ',')) return false;
  pos++;
 
  // The rest of the string should now be digits.
  for (int i = pos; i < len; i++) {
    if (!isdigit((int)(p[pos]))) return false;
    pos++;
  }

  // Yep, looks like a range.
  return true;
}

/*===================================================================*/
/** @fn bool DSHUtils::parsePacketParam(String& paramstr,uint32_t range[])
* @brief Parse the parameter string for the packet flag.
*                                        
* Parse the parameter string for the --packet command line flag.
* This method is designed to parse a single packet integer
* (--packet=5), a single packet symbolic (--packet=physics) or
* a packet integer range (--packet=5,10).  The range is returned
* in the range parameter.  If only a single packet is specified, then
* both ends of the range will be identical.
*                                         
* @param paramstr The paramter string to parse.
* @param range A int array of length 2 for returning the parsed range.  Output.
* @return None.
* @throw RuntimeException If the parameter cannot be parsed.
* @throw RuntimeException If a range contains a reserved system type.
*/      
void DSHUtils::parsePacketParam(daqhwyapi::String& paramstr,uint32_t range[]) {
  // Set a default range
  range[0] = 0; range[1] = 0;

  int len = paramstr.size();
  const char *p = paramstr.c_str();

  if (!isdigit(*p)) {  // Must be a symbolic value 
    range[0] = range[1] = Record::stringToPacketType(paramstr);
    range[1] = range[0];

    // Value must not be one of those reserved for use by the system
    if (Record::isReservedType(range[0])) {
      throw dshutils_rtexception.format(CSTR("DSHUtils::parsePacketParam() packet type specified (%d) indicates a type reserved for use ONLY by the system."),range[0]);
    }
  } else if (isNumericRange(paramstr)) { // Parse as a range
    char tstr[256]; // Should be large enough

    // Create a nonconst working copy
    int l = (len <= 255) ? len : 255;
    memcpy(tstr,p,l);

    // Find the separator
    char *sep = strchr(tstr,'-');
    if (sep == NULL) sep = strchr(tstr,',');
    if (sep == NULL) throw dshutils_rtexception.format(CSTR("DSHUtils::parsePacketParam() cannot parse paramstr \"%s\" as a numeric range"),tstr);

    // Add a null and increment to beginning of the second value
    (*sep) = '\0';
    sep++;

    // Parse the values
    String r1(tstr);
    String r2(sep);
    range[0] = StringUtils::parseInteger(r1);
    range[1] = StringUtils::parseInteger(r2);

    // The entire range must be above the maximum system types
    if (range[0] <= Record::type_maxsystem) {
      throw dshutils_rtexception.format(CSTR("DSHUtils::parsePacketParam() entire range of user types must be greater than the maximum system types = %d"),Record::type_maxsystem);
    }
  } else { // Integer single value
    range[0] = range[1] = StringUtils::parseInteger(paramstr);

    // Value must not be one of those reserved for use by the system
    if (Record::isReservedType(range[0])) {
      throw dshutils_rtexception.format(CSTR("DSHUtils::parsePacketParam() packet type specified (%d) indicates a type reserved for use by the system."),range[0]);
    }
  }

  // Make sure the low end of the range comes first
  if (range[0] > range[1]) {
    uint32_t rtmp = range[0];
    range[0] = range[1];
    range[1] = rtmp;
  }
}


/*==============================================================*/
/** @fn void DSHUtils::hexDump(File *fp,ByteArray& rArray,int aCnt)
* @brief Print in ByteArray in hex and character.
*
* Print a specified number of ByteArray values using a hex and
* character representation.
*
* @param fd An output file descriptor.
* @param rArray The array to print.
* @param aCnt The number of values to print.
* @return None
* @throw RuntimeException If fp is NULL.
*/                                                             
void DSHUtils::hexDump(FILE *fp,ByteArray& rArray,int aCnt) {
  unsigned char ch;
  unsigned short v;
  int i,j,cnt = 0,len;
  unsigned char *p,tmp[ARRAY_DUMP_CHARCOUNT+1];
  int scnt = rArray.length;
  char *vals = (char*)(rArray.elements);

  if (fp == NULL) {
    throw dshutils_rtexception.format(CSTR("DSHUtils::hexDump() fp cannot be NULL."));
  }
   
  len = aCnt < scnt ? aCnt : scnt;
  p = (unsigned char *)vals;
  fprintf(fp,"%08x: ",0);
    
  for (i = 0; i < len; i++) {
    if (cnt > ARRAY_DUMP_CHARCOUNT) {
      fprintf(fp,"\n          ");
    
      for (j = 0; j < cnt; j++) {
        ch = tmp[j];
        if (isprint(ch)&&(!iscntrl(ch))) fprintf(fp," %c",ch);
        else fprintf(fp,"  ");
      }
   
      fprintf(fp,"\n%08x: ",i);
      cnt = 0;
    }
  
    v = (*p);  
    fprintf(fp,"%02x",v);
    tmp[cnt] = (*p);
    cnt++; p++;
  }
    
  if (cnt > 0) fprintf(fp,"\n          ");

  for (j = 0; j < cnt; j++) {
    ch = tmp[j];
    
    if (isprint(ch)&&(!iscntrl(ch))) fprintf(fp," %c",ch);
    else fprintf(fp,"  ");
  }
   
  fprintf(fp,"\n");
  fflush(fp);
}


/*==============================================================*/
/** @fn void DSHUtils::convertEscapeCharacters(String& rStr)
* @brief Convert escape characters in a string.
*
* Convert escape characters (such as \f or \n) into their
* proper byte values.
*
* @param rStr The string to convert.
* @return None
*/                                                             
void DSHUtils::convertEscapeCharacters(String& rStr) {
  String wstr; 
  bool isescape = false;
  int siz = rStr.size();
  char c = '\0';

  for (int i = 0; i < siz; i++) { 
    c = rStr[i];
    if (!isescape) {
      if (c != '\\') wstr.append(c); 
      else isescape = true;
      continue;
    } else { // Escape character
      isescape = false;
      
      // Octal escape
      if (isoctal(c)) { // Is an octal escape
        // Format is \NNN (1 to 3 octal digits)
        int mlen = ((siz-i) >= 3) ? 3 : siz - i -1;
        int val = 0;
        for (int j = 0; j < mlen; j++) {
          if (isoctal(c)) {
            val = val << 3;
            val += octalvalue(c);
            i++; if (i < siz) c = rStr[i];
          } else {
            i--; c = rStr[i];
            break;
          }
        }
        wstr.append((char)val);
      } else if (c == 'x') { // Is a hex escape
        // Move forward if we can
        if (i < (siz-1)) {
          i++; if (i < siz) c = rStr[i];
          if (!ishex(c)) { // If not a hex digit, treat as escaped x
            wstr.append('x');
            i--; c = rStr[i];
            break;
          }
        } else { // At the end of the string, treas as excaped x
          wstr.append('x');
          break;
        }

        // Format is \xNN (1 to 2 hex digits)
        int mlen = ((siz-i) >= 2) ? 2 : siz - i - 1;
        int val = 0;
        for (int j = 0; j < mlen; j++) {
          if (ishex(c)) {
            val = val << 4;
            val += hexvalue(c);
            i++; if (i < siz) c = rStr[i];
          } else {
            i--; c = rStr[i];
            break;
          }
        }
        wstr.append((char)val);
      } else  { // formfeed, newlines, etc.
        switch (c) {
          case 'a': wstr.append((char)7); break;
          case 'b': wstr.append((char)8); break; 
          case 't': wstr.append((char)9); break;
          case 'n': wstr.append((char)10); break;
          case 'v': wstr.append((char)11); break;
          case 'f': wstr.append((char)12); break;
          case 'r': wstr.append((char)13); break;
          default: wstr.append(c); break;
        }
      }
    }
  }

  rStr = wstr; // Set the new values
}

