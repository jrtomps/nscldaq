#ifndef DAQHWYAPI_STRINGTOKENIZER_H
#define DAQHWYAPI_STRINGTOKENIZER_H

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

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {

/**
* @class StringTokenizer
* @brief StringTokenizer definition.
*
* The definition for the StringTokenizer class implementing a tokenizer
* for daqhwyapi::Strings.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class StringTokenizer : public Object {
  public: 
    StringTokenizer(String&);   // Constructor with a String
    StringTokenizer(String&,const char*); // Constructor with String and delim
    virtual ~StringTokenizer();         // Destructor
    int countTokens();          // Count the number of tokens
    bool hasMoreTokens();       // Check for more tokens
    String nextToken();         // Get the next token
    String nextToken(const char*);  // Get the next token with new delims

  protected:
    StringTokenizer();          // Default constructor 
    void setup(String&,const char*);  // Initialize this tokenizer
    char *skip_delims(char*);  // Skip delimiter characters
    char *find_delim(char*);  // Find the next delimiter
    bool is_delim(char);  // Is this character a delimiter

  private:
    /**
    * @var thestring
    * @brief The string to tokenize
    *
    * A copy of the string to tokenize.
    */
    String thestring; 

    /**
    * @var cur_pos
    * @brief Current tokenizer position.
    *
    * Current tokenizer position.
    */
    char *cur_pos; 

    /**
    * @var thedelims
    * @brief Current set of delimiters.
    *
    * Current set of delimiters.
    */
    String thedelims; 

    /**
    * @var token_cnt
    * @brief Current number of tokens.
    *
    * Current number of tokens.
    */
    int token_cnt; 
};

} // namespace daqhwyapi

#endif
