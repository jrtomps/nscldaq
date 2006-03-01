/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <ios>
#include <iosfwd>

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {
/**
* @var string_bad_alloc
* @brief Exception to throw when allocation fails.
*
* Exception to throw when allocation fails.
*/
static BadMemoryAllocationException string_bad_alloc;

/**
* @var string_out_of_bounds
* @brief Exception to throw for indexing errors.
*
* Exception to throw for indexing errors.
*/
static IndexOutOfBoundsException string_out_of_bounds;
} // namespace daqhwyapi

using namespace daqhwyapi;

int String::block_size = 20;

/*===================================================================*/
/** @fn String::String()
* @brief Default constructor.
*                                        
* Default constructor.  
*                                         
* @param None
* @return this                 
*/      
String::String() { 
  initialize();
}

/*===================================================================*/
/** @fn void String::initialize()
* @brief initialize.
*                                        
* initialize.
*                                         
* @param None
* @return None
*/      
void String::initialize() { 
  vals = NULL;
  scnt = 0;
  cmax = 0;
}

/*===================================================================*/
/** @fn String::String(const char *s,int len)
* @brief Constructor with character pointer and length.
*                                        
* Constructor with character pointer and length.
*                                         
* @param s The character pointer.
* @param len The number of characters.
* @return this                 
*/      
String::String(const char *s,int len) { 
  initialize();
  if ((s != NULL)&&(len > 0)) append((char*)s,len);
}

/*===================================================================*/
/** @fn String::String(const char *s)
* @brief Constructor with character pointer.
*                                        
* Constructor with character pointer.
*                                         
* @param s The character pointer.
* @return this                 
*/      
String::String(const char *s) { 
  initialize();
  if (s != NULL) {
    int len = strlen(s);
    append((char*)s,len);
  }
}

/*===================================================================*/
/** @fn String::String(const String& s)
* @brief Copy constructor.
*                                        
* Copy constructor.
*                                         
* @param s The string.
* @return this                 
*/      
String::String(const String& s) { 
  initialize();
  append(s.vals,s.scnt);
}

/*===================================================================*/
/** @fn String::String(const std::string& s)
* @brief Constructor with a std::string.
*                                        
* Constructor with a std::string.
*                                         
* @param s The string.
* @return this                 
*/      
String::String(const std::string& s) { 
  initialize();
  char *ws = (char*)(s.c_str());
  int l = s.size();
  if ((ws != NULL)&&(l > 0)) {
    append(ws,l);
  }
}

/*===================================================================*/
/** @fn String::String(const char v)
* @brief Constructor with char.
*                                        
* Constructor with a char.
*                                         
* @param v The char.
* @return this                 
*/      
String::String(const char v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const int v)
* @brief Constructor with an int.
*                                        
* Constructor with an int.
*                                         
* @param v The int.
* @return this                 
*/      
String::String(const int v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const unsigned int v)
* @brief Constructor with an unsigned int.
*                                        
* Constructor with an unsigned int.
*                                         
* @param v The unsigned int.
* @return this                 
*/      
String::String(const unsigned int v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const short v)
* @brief Constructor with a short.
*                                        
* Constructor with a short.
*                                         
* @param v The short.
* @return this                 
*/      
String::String(const short v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const unsigned short v)
* @brief Constructor with an unsigned short.
*                                        
* Constructor with an unsigned short.
*                                         
* @param v The unsigned short.
* @return this                 
*/      
String::String(const unsigned short v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const long v)
* @brief Constructor with a long.
*                                        
* Constructor with a long.
*                                         
* @param v The long.
* @return this                 
*/      
String::String(const long v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const long long v)
* @brief Constructor with a long long.
*                                        
* Constructor with a long long.
*                                         
* @param v The long long.
* @return this                 
*/      
String::String(const long long v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const long unsigned int v)
* @brief Constructor with a long unsigned int.
*                                        
* Constructor with a long unsigned int.
*                                         
* @param v The long unsigned int.
* @return this                 
*/      
String::String(const long unsigned int v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const long long unsigned int v)
* @brief Constructor with a long long unsigned int.
*                                        
* Constructor with a long long unsigned int.
*                                         
* @param v The long long unsigned int.  
* @return this                 
*/      
String::String(const long long unsigned int v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const float v)
* @brief Constructor with a float.
*                                        
* Constructor with a float.
*                                         
* @param v The float.
* @return this                 
*/      
String::String(const float v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const double v)
* @brief Constructor with a double.
*                                        
* Constructor with a double.
*                                         
* @param v The double.
* @return this                 
*/      
String::String(const double v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const bool v)
* @brief Constructor with a bool.
*                                        
* Constructor with a bool.
*                                         
* @param v The bool.
* @return this                 
*/      
String::String(const bool v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::String(const void *v)
* @brief Constructor with a void pointer.
*                                        
* Constructor with a void pointer.
*                                         
* @param v The void pointer.
* @return this                 
*/      
String::String(const void *v) {
  initialize();
  append(v);
}

/*===================================================================*/
/** @fn String::~String()
* @brief Destructor.
*                                        
* Destructor method for this class. 
*                                         
* @param None
* @return None 
*/      
String::~String() { 
  clear();
}

/*==============================================================*/
/** @fn bool String::append(char c) 
* @brief Append a character to this collection.
*
* Append a character to this collection.
*
* @param c The character.
* @return true if the string has changed. 
*/                                                             
bool String::append(char c) {
  int siz = (scnt <= 0) ? 2 : (scnt+1);
  if (grow(siz) == NULL) throw(string_bad_alloc.format(CSTR("String::append() cannot allocate memory of size = %d"),siz));
  vals[scnt] = c;
  scnt++;
  vals[scnt] = '\0';
  return true;
}

/*==============================================================*/
/** @fn bool String::append(char *s,int len) 
* @brief Append a character pointer to this string.
*
* Append a character pointer to this string.
*
* @param s The character pointer.
* @param len The number of characters to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(char *s,int len) {
  char *p = NULL;
  if (s == NULL) return false;
  if (len <= 0) return false;
  if (grow(scnt+len+1) == NULL) throw(string_bad_alloc.format(CSTR("String::append() cannot allocate memory of size = %d"),scnt+len));
  p = &(vals[scnt]);
  memcpy(p,s,len);
  scnt += len;
  vals[scnt] = '\0';
  return true;
}

/*==============================================================*/
/** @fn bool String::append(char *s) 
* @brief Append a character pointer to this string.
*
* Append a character pointer to this string.
*
* @param s The character pointer.
* @return true if the string has changed. 
*/                                                             
bool String::append(char *s) {
  if (s == NULL) return false;
  else return append(s,strlen(s));
}

/*==============================================================*/
/** @fn bool String::append(const char *s) 
* @brief Append a character pointer to this string.
*
* Append a character pointer to this string.
*
* @param s The character pointer.
* @return true if the string has changed. 
*/                                                             
bool String::append(const char *s) {
  if (s == NULL) return false;
  else return append((char*)s,strlen(s));
}

/*==============================================================*/
/** @fn bool String::append(const String& s) 
* @brief Append a String to this string.
*
* Append a String to this string.
*
* @param s The string to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(const String& s) {
  return append(s.vals,s.scnt); 
}

/*==============================================================*/
/** @fn bool String::append(const std::string& s) 
* @brief Append a std::string to this string.
*
* Append a std::string to this string.
*
* @param s The std::string to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(const std::string& s) {
  char *ws = (char*)(s.c_str());
  int l = s.size();
  if ((ws != NULL)&&(l > 0)) return append(ws,l); 
  return false;
}

/*==============================================================*/
/** @fn bool String::append(int v) 
* @brief Append an int to this string.
*
* Append an int to this string.
*
* @param v The int to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(int v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%d",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(unsigned int v) 
* @brief Append an unsigned int to this string.
*
* Append an unsigned int to this string.
*
* @param v The int to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(unsigned int v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%u",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(short v) 
* @brief Append a short to this string.
*
* Append a short to this string.
*
* @param v The short to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(short v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%hd",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(unsigned short int v) 
* @brief Append an unsigned short to this string.
*
* Append an unsigned short to this string.
*
* @param v The unsigned short to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(unsigned short int v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%hu",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(long v) 
* @brief Append a long to this string.
*
* Append a long to this string.
*
* @param v The long to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(long v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%ld",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(long long v) 
* @brief Append a long long to this string.
*
* Append a long long to this string.
*
* @param v The long long to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(long long v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%lld",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(unsigned long int v) 
* @brief Append an unsigned long to this string.
*
* Append an unsigned long to this string.
*
* @param v The unsigned long to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(unsigned long int v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%lu",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(unsigned long long int v) 
* @brief Append an unsigned long to this string.
*
* Append an unsigned long to this string.
*
* @param v The unsigned long long to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(unsigned long long int v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%llu",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(float v) 
* @brief Append a float to this string.
*
* Append a float to this string.
*
* @param v The float to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(float v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%g",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(double v) 
* @brief Append a double to this string.
*
* Append a double to this string.
*
* @param v The double to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(double v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"%g",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(void *v) 
* @brief Append a void pointer to this string.
*
* Append a void pointer to this string.
*
* @param v The void pointer to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(void *v) {
  int sz = snprintf(wrkspace,STRING_WRKSPACE_SIZE,"0x%p",v);
  return append(wrkspace,sz); 
}

/*==============================================================*/
/** @fn bool String::append(bool v) 
* @brief Append a bool to this string.
*
* Append a bool to this string.
*
* @param v The bool to append.
* @return true if the string has changed. 
*/                                                             
bool String::append(bool v) {
  if (v) return append("true",4);
  else return append("false",5);
}


/*==============================================================*/
/** @fn String& String::operator+=(const char v) 
* @brief Append a char to this string.
*
* Append a char to this string.
*
* @param v The char to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const char v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const char *v) 
* @brief Append a char pointer to this string.
*
* Append a char pointer to this string.
*
* @param v The char pointer to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const char *v) {
  append((char*)v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const String& v) 
* @brief Append a String to this string.
*
* Append a String to this string.
*
* @param v The String to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const String& v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const std::string& v) 
* @brief Append a std::string to this string.
*
* Append a std::string to this string.
*
* @param v The std::string to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const std::string& v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const int v) 
* @brief Append an int to this string.
*
* Append an int to this string.
*
* @param v The int to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const int v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const unsigned int v) 
* @brief Append an unsigned int to this string.
*
* Append an unsigned int to this string.
*
* @param v The unsigned int to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const unsigned int v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const short v) 
* @brief Append a short to this string.
*
* Append a short to this string.
*
* @param v The short to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const short v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const unsigned short v) 
* @brief Append an unsigned short to this string.
*
* Append an unsigned short to this string.
*
* @param v The unsigned short to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const unsigned short v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const long v) 
* @brief Append a long to this string.
*
* Append a long to this string.
*
* @param v The long to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const long v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const long long v) 
* @brief Append a long long to this string.
*
* Append a long long to this string.
*
* @param v The long long to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const long long v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const long unsigned int v) 
* @brief Append a long unsigned int to this string.
*
* Append a long unsigned int to this string.
*
* @param v The long unsigned int to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const long unsigned int v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const long long unsigned int v) 
* @brief Append a long long unsigned int to this string.
*
* Append a long long unsigned int to this string.
*
* @param v The long long unsigned int to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const long long unsigned int v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const float v) 
* @brief Append a float to this string.
*
* Append a float to this string.
*
* @param v The float to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const float v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const double v) 
* @brief Append a double to this string.
*
* Append a double to this string.
*
* @param v The double to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const double v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const bool v) 
* @brief Append a bool to this string.
*
* Append a bool to this string.
*
* @param v The bool to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const bool v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator+=(const void *v) 
* @brief Append a void pointer to this string.
*
* Append a void pointer to this string.
*
* @param v The void pointer to append.
* @return A reference to this.
*/                                                             
String& String::operator+=(const void *v) {
  append(v);
  return *this;
}

/*==============================================================*/
/** @fn String& String::operator=(const char v) 
* @brief Assign a char to this string.
*
* Assign a char to this string.
*
* @param v The char to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const char v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const char *v) 
* @brief Assign a char pointer to this string.
*
* Assign a char pointer to this string.
*
* @param v The char pointer to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const char* v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const String& v) 
* @brief Assign a String to this string.
*
* Assign a String to this string.
*
* @param v The String to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const String& v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const std::string& v) 
* @brief Assign a std::string to this string.
*
* Assign a std::string to this string.
*
* @param v The std::string to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const std::string& v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const int v) 
* @brief Assign an int to this string.
*
* Assign an int to this string.
*
* @param v The int to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const int v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const unsigned int v) 
* @brief Assign an unsigned int to this string.
*
* Assign an unsigned int to this string.
*
* @param v The unsigned int to aassign.
* @return A reference to this.
*/                                                             
String& String::operator=(const unsigned int v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const short v) 
* @brief Assign a short to this string.
*
* Assign a short to this string.
*
* @param v The short to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const short v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const unsigned short v) 
* @brief Assign an unsigned short to this string.
*
* Assign an unsigned short to this string.
*
* @param v The unsigned short to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const unsigned short v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const long v) 
* @brief Assign long to this string.
*
* Assign a long to this string.
*
* @param v The long to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const long v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const long long v) 
* @brief Assign a long long to this string.
*
* Assign a long long to this string.
*
* @param v The long long to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const long long v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const long unsigned int v) 
* @brief Assign a long unsigned to this string.
*
* Assign a long unsigned to this string.
*
* @param v The long unsigned to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const long unsigned int v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const long long unsigned int v) 
* @brief Assign a long long unsigned to this string.
*
* Assign a long long unsigned to this string.
*
* @param v The long long unsigned to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const long long unsigned int v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const float v) 
* @brief Assign a float to this string.
*
* Assign a float to this string.
*
* @param v The float to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const float v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const double v) 
* @brief Assign a double to this string.
*
* Assign a double to this string.
*
* @param v The double to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const double v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const bool v) 
* @brief Assign a bool to this string.
*
* Assign a bool to this string.
*
* @param v The bool to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const bool v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn String& String::operator=(const void *v) 
* @brief Assign a void pointer to this string.
*
* Assign a void pointer to this string.
*
* @param v The void pointer to assign.
* @return A reference to this.
*/                                                             
String& String::operator=(const void *v) {
  clear();
  append(v);
  return(*this);
}

/*==============================================================*/
/** @fn char& String::operator[](const int idx) 
* @brief Subscript this string.
*
* Return the character at a specified index.
*
* @param idx The index.
* @return A reference to the requeste char.
* @throws A reference to the requeste char.
*/                                                             
char& String::operator[](const int idx) {
  if ((idx < 0)||(idx >= scnt)) throw string_out_of_bounds.format(CSTR("String::operator[]() specified index is out-of-bounds index = %d"),idx);
  return vals[idx];
}

/*==============================================================*/
/** @fn void clear() 
* @brief Empty this string.
*
* Empty this string.
*
* @param None
* @return None
*/                                                             
void String::clear() {
  if (vals != NULL) delete[] vals;
  vals = NULL;
  cmax = 0;
  scnt = 0;
}

/*==============================================================*/
/** @fn bool String::equals(const String& s) 
* @brief Check if this string is equal to another.
*
* Check if this string is equal to another. 
*
* @param s The other string.
* @return true if the string is equal to the one specified.
*/                                                             
bool String::equals(const String& s) {
  if (scnt != s.scnt) return false;
  else return strncmp(vals,s.vals,scnt) == 0 ? true : false;
}

/*==============================================================*/
/** @fn bool String::equalsIgnoreCase(const String& s) 
* @brief Check if this string is equal to another.
*
* Check if this string is equal to another (case insensitive). 
*
* @param s The other string.
* @return true if the string is equal to the one specified.
*/                                                             
bool String::equalsIgnoreCase(const String& s) {
  if (scnt != s.scnt) return false;
  else return strncasecmp(vals,s.vals,scnt) == 0 ? true : false;
}

/*==============================================================*/
/** @fn int String::size()
* @brief Return the length of this string.
*
* Return the length of this string.
*
* @param None
* @return The length of this string.
*/                                                             
int String::size() {
  return scnt;
}

/*==============================================================*/
/** @fn Object *String::grow(int siz)
* @brief Grow this String.
*
* Grow this String to the specified size.  
*
* @param siz The requested allocation size.
* @return The allocated array or NULL on failure.
*/                                                             
char *String::grow(int siz) {
  char *ns = NULL;
  int blks = 0;

  if (siz < 0) blks = 1;
  else blks = (siz/block_size)+1;

  if ((vals == NULL)||(cmax < 1)) { // Init
    if (vals != NULL) delete[] vals;
    try {
      vals = new char[(blks*block_size)+1];
      memset(vals,0,(blks*block_size)+1);
    } catch (...) {
      return(NULL);
    }
    cmax = blks * block_size;
  } else if (siz >= cmax) { // Grow 
    try {
      ns = new char[(blks*block_size)+1]; 
      memset(ns,0,(blks*block_size)+1);
    } catch (...) {
      return(NULL);
    }
    if (vals != NULL) {
      memcpy(ns,vals,scnt);
      delete[] vals;
    }
    vals = ns;
    cmax = blks * block_size;
  } else if (siz < (cmax - block_size)) {  // Shrink
    try {
      ns = new char[(blks*block_size)+1]; 
      memset(ns,0,(blks*block_size)+1);
    } catch (...) {
      return(NULL);
    }
    if (vals != NULL) {
      memcpy(ns,vals,siz);
      delete[] vals;
    }
    vals = ns;
    cmax = blks * block_size;
  } 

  return(vals);
}

/*==============================================================*/
/** @fn char String::get(int idx) 
* @brief Get a character from this collection.
*
* Get a character from this collection.
*
* @param idx The index of the element.
* @return The character.
* @throw IndexOutOfBoundsException.
*/                                                             
char String::get(int idx) {
  if ((idx < 0)||(idx >= scnt)) throw string_out_of_bounds.format(CSTR("String::get() specified index is out-of-bounds index = %d"),idx);
  return (vals[idx]);
}

/*==============================================================*/
/** @fn char String::set(int idx,char c) 
* @brief Set a character in this collection.
*
* Set a character in this collection.
*
* @param idx The index of the element.
* @param c The character.
* @return The character.
* @throw IndexOutOfBoundsException.
*/                                                             
char String::set(int idx,char c) {
  if ((idx < 0)||(idx >= scnt)) throw string_out_of_bounds.format(CSTR("String::set() specified index is out-of-bounds index = %d"),idx);
  vals[idx] = c;
  return c;
}

/*==============================================================*/
/** @fn const char *String::c_str() const
* @brief Expose C style character pointer.
*
* Expose C style character pointer.  Dangerous use only as
* a constant string.  Will be destroyed if underlying String
* object is deleted. 
*
* @param None
* @return C style character pointer.
*/                                                             
const char *String::c_str() const {
  return vals; 
}

namespace daqhwyapi {

/*==============================================================*/
// binary operator+                 
/*==============================================================*/
String operator+(const String& s1,const char v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const char v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const int v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const int v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const unsigned int v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const unsigned int v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const short v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const short v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const unsigned short v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const unsigned short v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const long v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const long v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const unsigned long v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const unsigned long v1,String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const long long v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const long long v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const unsigned long long v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const unsigned long long v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const float v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const float v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const double v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const double v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const bool v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const bool v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const void *v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const void *v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const char *v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const char *v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const String& s2) {
  String ws(s1);
  ws.append(s2);
  return(ws);
}

String operator+(const String& s1,const std::string& v2) {
  String ws(s1);
  ws.append(v2);
  return(ws);
}

String operator+(const std::string& v1,const String& s2) {
  String ws(v1);
  ws.append(s2);
  return(ws);
}

} // namespace daqhwyapi
