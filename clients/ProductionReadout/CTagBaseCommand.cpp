/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CTagBaseCommand.cpp file////////////////////////////////////
#include <config.h>
#include <TCLResult.h>
#include "CTagBaseCommand.h"     
#include <RangeError.h>
#include <string>             
#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

static const unsigned int DEFAULTTAGBASE = 2;
static const unsigned int DEFAULTTAGWIDTH= 2;
static const unsigned int LASTVALIDTAG   = 23;

/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CTagBaseCommand  object;
   are performed.
   
   The command registered will be "tagbase"
*/
CTagBaseCommand::CTagBaseCommand () :
  CDAQTCLProcessor(string("tagbase"), 0),
  m_nTagBase(DEFAULTTAGBASE),		
  m_nTagBits(DEFAULTTAGWIDTH)
 
{
} 


// Functions for class CTagBaseCommand

/*!
    Does initial command decode on the tagbase command.  Control
    is then dispatched to one of SetBase, List, or NumTags
    depending on the subcommand or lack of a subcommand.
    

	\param rInterp   - The interpreter on which the command is executing.
	\param rResult   - Object encapsulation of the result string to be
	                   return to the interpreter by the command.  The
			   contents of this string depend not only on the 
			   sub-function executed, but also on the
			   success or failure of that subfunction.
	\param argc,argv - Command parameters.  Note that argv[0] is the
	                   name of the command.

	\return TCL_OK if everything went well.

	\return TCL_ERROR on error.


*/
int 
CTagBaseCommand::operator()(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			    int  argc, char** argv)  
{
  argc--; argv++;

  // if there are no more parameters, this is a List operation.

  int status;
  if(!argc) {
    status = List(rInterp, rResult, argc, argv);
  }
  else if (string(*argv) == string("-base")) { // Set new base/size.
    argc--;
    argv++;
    status = SetBase(rInterp, rResult, argc, argv);
  }
  else if(string(*argv) == string("-size")) {
    argc--;
    argv++;
    status = NumTags(rInterp, rResult, argc, argv);
  }
  else {			// Unrecognized keyword...
    Usage(rResult);
    status = TCL_ERROR;
  }
  return status;
}  

/*!
    Sets the tag base for the buffers emitted by this daq Readout 
    program.
    Command Syntax:

        tagbase newbase [newsize]

	\param rInterp    - Interpreter under which the command is being
	                    run.
	\param rResult    - Result string to be sent back to the interpreter
	                    by cammand execution.
	\param argc,argv  - Parameters following command keyword.

	\returns TCL_OK  - If everything succeeded.

	\returns TCL_ERROR - If there was a problem.

	\note  If newsize is not supplied, we default back to the
	default tagwidth.  Newsize is rounded up to the nearest power of 2.
	so that tags fit in a mask.

	\note Spectrodaq requires and it is a reported command error
	     if newbase + newsize > LASTVALIDTAG.
*/
int 
CTagBaseCommand::SetBase(CTCLInterpreter& rInterp, CTCLResult& rResult, 
			 int  argc, char** argv)  
{
  int newbase = m_nTagBase;
  int newbits = DEFAULTTAGWIDTH;
  

  // There must be at least a tagbase parameter, no more than tagbase and
  // width.

  if((!argc) || (argc > 2)) {
    Usage(rResult);
    return TCL_ERROR;
  }
  // Parse the tag base..

  if(ParseInt(*argv, &newbase) != TCL_OK) {
    Usage(rResult);
    rResult += "   >> new tag base was not a valid integer << \n";
    return TCL_ERROR;
  }
  // Range check the tag base:

  if((newbase < 0) || (newbase > LASTVALIDTAG)) {
    Usage(rResult);
    rResult += "   >> new tag base is an invalid value << \n";
    return TCL_ERROR;
  }
  // If present, parse out the width:

  argc--;
  argv++;
  if(argc) {
    if(ParseInt(*argv, &newbits) != TCL_OK) {
      Usage(rResult);
      rResult += " >> new tag size is not a valid integer << \n";
      return TCL_ERROR;
    }
  }

  // Range check the width:
  
  if((newbits < 1) && ((newbase + newbits) > LASTVALIDTAG)) {
    Usage(rResult);
    rResult = " >> Tag width together with tagbase is invalid << \n";
    return TCL_ERROR;
  }
  // All the stuff is valid  .. set it and return success wigth an empty
  // result string:

  m_nTagBase = (unsigned int)newbase;
  m_nTagBits = (unsigned int)newbits; 

  return TCL_OK;

}  

/*!
    Lists the tag base currently in use.  The result is 
    the stringified value of m_nTagBase.

    \param rInterp      - Interpreter under which the command is executing.
    \param rResult      - Result string for the command.
    \param argc,argv    - Parameters following the command (aggc == 0  is
                          required).

    \returns TCL_OK  - If there are no parse error.

    \returns TCL_ERROR - if there was a parse error.

    \note Once parsing is complete it's impossible to have an error.
*/
int 
CTagBaseCommand::List(CTCLInterpreter& rInterp, CTCLResult& rResult, 
		      int argc, char** argv)  
{
  if(argc) {
    Usage(rResult);
    return TCL_ERROR;
  }


  EncodeInteger(rResult, m_nTagBase);

  return TCL_OK;
}  

/*!
    Returns the number of items in a tag set.  The result is the
    stringified valueof m_nTagBits.

    \param rInterp    - Interpreter under which the command is running.
    \param rResult    - Result string the command returns to the 
                        interpreter.
    \param argc,argv  - Parameters following the -size switch; must be none.

    \returns TCL_OK  - If there are no parse error.

    \returns TCL_ERROR - if there was a parse error.

    \note Once parsing is complete it's impossible to have an error.

*/
int 
CTagBaseCommand::NumTags(CTCLInterpreter& rfinterp, CTCLResult& rResult, 
			 int argc, char** argv)  
{
  if(argc) {
    Usage(rResult);
    return TCL_ERROR;
  }

  EncodeInteger(rResult, m_nTagBits);
  return TCL_OK;

}
/*!
   Gets a tag from the set.

   \param nOffset - Tag offset from the base.

   \throws CRangeError if nOffset > m_nTagBits.

   */
unsigned int
CTagBaseCommand::getTag(unsigned int nOffset)
{
  if(nOffset > m_nTagBits) {
    throw CRangeError(0, m_nTagBits - 1, nOffset,
		      "Retrieving a tag in CTagBaseCommand::getTag()");
  }

  return m_nTagBase + nOffset;
}
/*!
   Append a usage text string to the result object.  This is usually
   called in response to a failed parse.

   \param rResult - Object encapsulated Result string to be appended to.
   */
void
CTagBaseCommand::Usage(CTCLResult& rResult)
{
  rResult += "Usage:\n";
  rResult += "    tagbase\n";
  rResult += "    tagbase -size\n";
  rResult += "    tagbase -base n ?num=2?\n";


}
/*!
   Utility member to encode an integer into a result string.

   \param rResult - Object encapsulated result string (appended to).
   \param nValue  - Integer Value to encode.
   */
void
CTagBaseCommand::EncodeInteger(CTCLResult& rResult, int value)
{
  char EncodedValue[100];
  sprintf(EncodedValue, "%d", value);
  rResult += EncodedValue;

}

