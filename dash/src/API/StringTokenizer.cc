/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <ctype.h>

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_STRINGTOKENIZER_H
#include <dshapi/StringTokenizer.h>
#endif

namespace daqhwyapi {
/**
* @var tokenizer_no_such_element
* @brief Exception to throw when there are no more elements.
*
* Exception to throw when there are no more elements.
*/
static NoSuchElementException tokenizer_no_such_element;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn StringTokenizer::StringTokenizer()
* @brief Default constructor.
*                                        
* Default constructor.  
*                                         
* @param None
* @return this                 
*/      
StringTokenizer::StringTokenizer() { 
  token_cnt = -1;
  cur_pos = NULL;
  thedelims.clear();
  thestring.clear();
}

/*===================================================================*/
/** @fn StringTokenizer::StringTokenizer(String& rStr)
* @brief Constructor with a String.
*                                        
* Constructor with a String to tokenize.  
*                                         
* @param rStr The string to tokenizer
* @return this                 
*/      
StringTokenizer::StringTokenizer(String& rStr) { 
  token_cnt = -1;
  cur_pos = NULL;
  thedelims.clear();
  thestring.clear();
  setup(rStr,NULL);
}

/*===================================================================*/
/** @fn StringTokenizer::StringTokenizer(String& rStr,const char *delim)
* @brief Constructor with a String.
*                                        
* Constructor with a String to tokenize.  
*                                         
* @param rStr The string to tokenizer
* @param delim The delimiters.
* @return this                 
*/      
StringTokenizer::StringTokenizer(String& rStr,const char *delim) { 
  token_cnt = -1;
  cur_pos = NULL;
  thedelims.clear();
  thestring.clear();
  setup(rStr,delim);
}

/*===================================================================*/
/** @fn void StringTokenizer::setup(String& rStr,const char *delim)
* @brief Set up this tokenizer. 
*                                        
* Set up and initialize this tokenizer.
*                                         
* @param rStr The string to tokenizer
* @param delim The delimiters.
* @return this                 
*/      
void StringTokenizer::setup(String& rStr,const char *delim) { 
  thestring.clear();
  thedelims.clear();
  thestring = rStr;
  thedelims = delim;
  token_cnt = -1;
  cur_pos = thestring.vals;
}

/*===================================================================*/
/** @fn StringTokenizer::~StringTokenizer()
* @brief Destructor.
*                                        
* Destructor method for this class. 
*                                         
* @param None
* @return None 
*/      
StringTokenizer::~StringTokenizer() { 
  thestring.clear();
  thedelims.clear();
  token_cnt = -1;
  cur_pos = NULL;
}

/*==============================================================*/
/** @fn int StringTokenizer::countTokens() 
* @brief Count the number of tokens.
*
* Count the number of tokens that can be returned from the current
* position.
*
* @param None
* @return The number of tokens.
*/                                                             
int StringTokenizer::countTokens() {
  if (cur_pos == NULL) return 0;
  if (token_cnt < 0) { // Count them
    token_cnt = 0; 
    char *p = cur_pos;
    while ((*p) != '\0') { 
      p = skip_delims(p);    
      if ((*p) != '\0') {
        token_cnt++;
        p = find_delim(p);
      }
    }
  }
  return token_cnt;
}

/*==============================================================*/
/** @fn bool StringTokenizer::hasMoreTokens() 
* @brief Check if there are more tokens available.
*
* Check if ther are more tokens available.
*
* @param None
* @return If there are more tokens available.
*/                                                             
bool StringTokenizer::hasMoreTokens() {
  if (token_cnt < 0) countTokens(); 
  if (token_cnt > 0) return true;
  else return false;
}

/*==============================================================*/
/** @fn String StringTokenizer::nextToken() 
* @brief Get the next token.
*
* Get the next token using the current delimiters.
*
* @param None
* @return The next token.
* @throw NoSuchElementException if there are not more tokens.
*/                                                             
String StringTokenizer::nextToken() {
  if (cur_pos == NULL) throw tokenizer_no_such_element.format(CSTR("StringTokenizer::nextToken() No more tokens to be found"));

  cur_pos = skip_delims(cur_pos);
  if ((*cur_pos) == '\0') {
    cur_pos = NULL;
    thestring.clear();
    thedelims.clear();
    throw tokenizer_no_such_element.format(CSTR("StringTokenizer::nextToken() No more tokens to be found"));
  }

  String tok;
  char *p = find_delim(cur_pos);
  if (p == '\0') {
    tok = cur_pos; 
    cur_pos = NULL;
    thestring.clear();
    thedelims.clear();
  } else {
    char op = (*p);
    (*p) = '\0';
    tok = cur_pos;
    (*p) = op;
    cur_pos = p;
  }

  token_cnt--;
  return tok;
}

/*==============================================================*/
/** @fn String StringTokenizer::nextToken(const char *ndelim) 
* @brief Get the next token.
*
* Get the next token after switch to a new delimiter set.
*
* @param ndelim The new delimiter set.
* @return The next token.
* @throw NoSuchElementException if there are not more tokens.
*/                                                             
String StringTokenizer::nextToken(const char *ndelim) {
  thedelims.clear();
  thedelims = ndelim;
  return nextToken();
}

/*==============================================================*/
/** @fn char *StringTokenizer::skip_delims(char *pstr) 
* @brief Skip multiple occurances of delimiter characters.
*
* Skip multiple occurances of delimiter characters.
*
* @param pstr The string to process.
* @return A pointer to the first nondelimiter or NULL.
*/                                                             
char *StringTokenizer::skip_delims(char *pstr) {
  if (pstr == NULL) return NULL;
  if ((*pstr) == '\0') return pstr;

  char *p = pstr;
  int len = strlen(p);

  for (int i = 0; i < len; i++) {
    if (is_delim(*p)) p++; 
    else break;
  }

  return p;
}

/*==============================================================*/
/** @fn char *StringTokenizer::find_delim(char *pstr) 
* @brief Find the next delimiter.
*
* Find the next delimiter.
*
* @param pstr The string to process.
* @return A pointer to the next delimiter or NULL.
*/                                                             
char *StringTokenizer::find_delim(char *pstr) {
  if (pstr == NULL) return NULL;
  if ((*pstr) == '\0') return pstr;

  char *p = pstr;
  int len = strlen(p);

  for (int i = 0; i < len; i++) {
    if (!is_delim(*p)) p++; 
    else break;
  }

  return p;
}

/*==============================================================*/
/** @fn bool StringTokenizer::is_delim(char c) 
* @brief Check if a character is a delimiter.
*
* Check if a character is a delimiter.
*
* @param c The character to check.
* @return If the character is a delimiter.
*/                                                             
bool StringTokenizer::is_delim(char c) {
  char *d = thedelims.vals;
  int dlen = 0;
  if (thedelims.size() <= 0) d = NULL;
  else dlen = strlen(d);

  if (d == NULL) { // use white space
    if (isspace(c)) return true;
  } else { // use specified delimiters
    for (int j = 0; j < dlen; j++) {
      if (c == d[j]) return true;
    }
  }

  return false;
}

