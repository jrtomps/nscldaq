#ifndef DAQHWYAPI_STRING_H
#define DAQHWYAPI_STRING_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

namespace daqhwyapi {

#define STRING_WRKSPACE_SIZE 256

class StringTokenizer;

/**
* @class String
* @brief String definition.
*
* The definition for the String class implementing a dynamic
* string.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class String : public Object {
  public: 
    String();          // Default constructor 
    String(const String&);   // Copy constructor 
    String(const std::string&);   // Constructor  with std::string
    String(const char*);     // Constructor with char*
    String(const char*,int); // Constructor with char* and length
    String(const char);  // Constructor with char
    String(const int);     // Constructor with int
    String(const unsigned int);  // Constructor with unsigned int
    String(const short);  // Constructor with short
    String(const unsigned short);  // Constructor with unsigned short
    String(const long);  // Constructor with long
    String(const long long);  // Constructor with long long
    String(const long unsigned int);  // Constructor with long uint
    String(const long long unsigned int); // Constructor with long long uint
    String(const float);  // Constructor with float
    String(const double);  // Constructor with double 
    String(const bool);  // Constructor with bool
    String(const void*);  // Constructor with void*
    virtual ~String();         // Destructor

    void clear();           // Empty this String
    bool equals(const String&);   // Check for equality
    bool equalsIgnoreCase(const String&);  // Check for equality (case insensitive)
    int size();               // Return the size (number of elements)
    char get(int);            // Get a char.  
    char set(int,char);       // Set a char.
    const char *c_str() const;      // Expose C style char pointer.

    bool append(char);        // Append a char.
    bool append(char*);       // Append a char*.
    bool append(const char*); // Append a char*.
    bool append(char*,int);   // Append a char* and length.
    bool append(const String&);     // Append a String.
    bool append(const std::string&);     // Append a std::string.
    bool append(int);         // Append an int.
    bool append(unsigned int);  // Append an unsigned int.
    bool append(short);       // Append a short.
    bool append(unsigned short int); // Append an unsigned short.
    bool append(long);        // Append a long.
    bool append(long long);   // Append a long long.
    bool append(long unsigned int); // Append a long unsigned int.
    bool append(long long unsigned int); // Append a long long unsigned int.
    bool append(float);       // Append a float.
    bool append(double);      // Append a double.
    bool append(bool);        // Append a bool.
    bool append(void*);       // Append a void*.

    String& operator+=(const char);  // Append
    String& operator+=(const char*);  // Append
    String& operator+=(const String&);  // Append
    String& operator+=(const std::string&);  // Append
    String& operator+=(const int);  // Append
    String& operator+=(const unsigned int);  // Append
    String& operator+=(const short);  // Append
    String& operator+=(const unsigned short);  // Append
    String& operator+=(const long);  // Append
    String& operator+=(const long long);  // Append
    String& operator+=(const long unsigned int);  // Append
    String& operator+=(const long long unsigned int);  // Append
    String& operator+=(const float);  // Append
    String& operator+=(const double);  // Append
    String& operator+=(const bool);  // Append
    String& operator+=(const void*);  // Append

    String& operator=(const char);  // Assign
    String& operator=(const char*);  // Assign
    String& operator=(const String&);  // Assign
    String& operator=(const std::string&);  // Assign
    String& operator=(const int);  // Assign
    String& operator=(const unsigned int);  // Assign
    String& operator=(const short);  // Assign
    String& operator=(const unsigned short);  // Assign
    String& operator=(const long);  // Assign
    String& operator=(const long long);  // Assign
    String& operator=(const long unsigned int);  // Assign
    String& operator=(const long long unsigned int);  // Assign
    String& operator=(const float);  // Assign
    String& operator=(const double);  // Assign
    String& operator=(const bool);  // Assign
    String& operator=(const void*);  // Assign

    char& operator[](const int idx); // Subscript

  protected:
    void initialize();        // Initialize this object
    char *grow(int);          // Grow this String

  private:
    friend class daqhwyapi::StringTokenizer;

    /**
    * @var vals
    * @brief Array values.
    *
    * Pointer to a simple array of Object values.
    */
    char *vals; 

    /**
    * @var scnt
    * @brief Current length of storage requested for allocation.
    *
    * Current length of storage requested for allocation in
    * units of Object pointers.  That is, this is a count of the number
    * of Object values for which allocation has been requested
    * and completed.
    */
    int scnt; 

    /**
    * @var cmax
    * @brief Current maximum array elements allocated.
    *
    * Since the allocator often allocates storage in blocks,
    * the maximum current allocation may be larger than that
    * requested.
    */
    int cmax;  

    /**
    * @var wrkspace
    * @brief Array of work space.
    *
    * Array of work space for converting values.
    */
    char wrkspace[STRING_WRKSPACE_SIZE]; 

    /**
    * @var block_size
    * @brief Size of the allocation blocks.
    *
    * Size of the allocation blocks for growing the list.
    */
    static int block_size;
};

// Binary plus operator
daqhwyapi::String operator+(const daqhwyapi::String& s1,const char v2);
daqhwyapi::String operator+(const char v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const int v2);
daqhwyapi::String operator+(const int v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const unsigned int v2);
daqhwyapi::String operator+(const unsigned int v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const short v2);
daqhwyapi::String operator+(const short v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const unsigned short v2);
daqhwyapi::String operator+(const unsigned short v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const long v2);
daqhwyapi::String operator+(const long v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const unsigned long v2);
daqhwyapi::String operator+(const unsigned long v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const long long v2);
daqhwyapi::String operator+(const long long v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const unsigned long long v2);
daqhwyapi::String operator+(const unsigned long long v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const float v2);
daqhwyapi::String operator+(const float v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const double v2);
daqhwyapi::String operator+(const double v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const bool v2);
daqhwyapi::String operator+(const bool v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const char *v2);
daqhwyapi::String operator+(const char *v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const void *v2);
daqhwyapi::String operator+(const void *v1,const daqhwyapi::String& s2);
daqhwyapi::String operator+(const daqhwyapi::String& s1,const daqhwyapi::String& s2);

daqhwyapi::String operator+(const daqhwyapi::String& s1,const std::string& v2);
daqhwyapi::String operator+(const std::string& v1,const daqhwyapi::String& s2);

} // namespace daqhwyapi

#endif
