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

#include <config.h>
#include "AssemblerCommand.h"
#include "AssemblerErrors.h"

#include <TCLObject.h>
#include <TCLInterpreter.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>

using namespace std;

// Dispatch table structure.. The dispatch table allows
// table driven dispatch of the command subkeywords:


typedef struct {
    const char* pKeyword;
    int         minParameters;
    int         maxParameters;
    int         (AssemblerCommand::*processor)(CTCLInterpreter& interp,
                                               vector<CTCLObject>& objv);
} DispatchEntry *pDispatchEntry;

static const int SUBCOMMANDCOUNT(5);
static DispatchEntry DispatchTable[SUBCOMMANDCOUNT] = {
    {"node",     4,   4,   &Assembler::node},
    {"trigger",  3,   3,   &Assember::trigger},
    {"window",   4,   5,   &Assembler::window},
    {"list",     2,   2,   &Assembler::list},
    {"validate", 2,   2,   &Assembler::validate}
};



/*!
  Construct the assembler command.  The command name
  is fixed and constant as 'assembler'.  See the class
  comments for information about the syntax suppported by
  this command ensemble.
  \param interp - Reference to the interpreter object on which
                  the command will be registered.
*/
AssemblerCommand::AssemblerCommand(CTCLInterpreter& interp) :
	CTCLObjectProcessor(interp, string("assembler"), true)
{	
	// Initialize the array of pointers to node information
	// to be empty:

	memset(m_nodeTable, 0, sizeof(m_nodeTable));
}
/*!
  Destruction of the assembler command object is likely not
  to ever happen as we will want the configuration for the
  lifetime of the program... however we do it cleanly in any event.
  Note that each used element of the m_nodeTable has dynamically
  allocated node names...and these must be deleted.
  The host table itself is statically sized and does not
  require deletion.

*/
AssemblerCommand::~AssemblerCommand()
{
	std::list<int>::iterator i = m_definedNodes.begin();
	while (i != m_definedNodes.end()) {
		int nodeIndex = *i;
		delete [](m_nodeTable[nodeIndex].pNodeName);
		i++;
	}
}
/*!
   operator() is called when the 'assemble' command has been invoked.
   We don't do much here other than ensure than ensure there is a 
   subcommand keyword argument, extract it into a string and then
   dispatch to the appropriate keyword handler.

   \param interpreter   - Reference to the interpreter object that is
                          running this command
   \param objv          - vector of objects that are the command line words.
   \return int   
   \retval TCL_OK       - The command worked.
   \retval TCL_ERROR    - An error was detected in the command.
*/
int
AssemblerCommand::operator()(CTCLInterpreter& interp,
                             vector<CTCLObject>& objv)
{
    // note that since the object vector includes the
    // 'assembler' command keyword we need at least two
    // elements:

    if (objv.size() < 2) {
        return 
            AssemblerErrors::setErrorMessage(interp,
                                             AssemblerErrors::TooFewParameters,
                                             Usage();
    }
    // Extract the keyword and dispatch:

    objv[1].bind(interp);
    string keyword = objv[1];

    return Dispatch(keyword,
                    interp, objv);

}
//////////////////////////////////////////////////////////////////////////////////////
//
// This function manages the node subcommand. The dispatcher has already kindly
// validated our parameter count.
// Parameters:
//    interp   - Reference to the interpreter object that is running our command.
//    objv     - Reference to a vector of command words:
//               objv[2] - dns name or ip address of the node being defined.
//               objv[3] - id of the ndoe being defined.\
// Returns:
//    TCL_OK   - Success and the interpreter result is set to the id.
//               and the node has been added to the table.
//    TCL_ERROR- Failure which leave the interpreter result set with one of
//               The following error messages:
//               DuplicateNode   - The node already is defined by name or IP.
//               DuplicateId     - The node's id is already in use.
//               NoDnsName       - The node parameter is not translatable in dns and
//                                 is not a valid ip address.
//               BadId           - The id is not a valid id (id's  must be in the
//                                 range 0-0xffff.
//
int
AssemblerCommand::node(CTCLInterpreter& interp,
                       vector<CTCLObject>& objv)
{
    objv[2].Bind(interp);
    objv[3].Bind(interp);

    // Extract the parameters; and make sure the id is valid:

    string node = objv[2];
    try {
        int id = objv[3];
    }
    catch(...) {
        // Not an integer most likely...
        
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadId,
                                          Usage());
    }
    // Bad integer value (out of range):

    if ((id < 0) || (id > 0xffff)) {
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadId,
                                          Usage());
    }
    // Id is already in the table?

    if (findNode((short)id)) {
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::DuplicateId,
                                          Usage());
    }
    // Now start on the node.. we will attempt to convert it to a hostentry:

    struct hostent   *pHostInformation = gethostbyname(node.c_str());
    if (!pHostInformation) {
        struct in_addr ipAddress;
        if (inet_aton(node.c_str(), &ipAddress)) {

                // It's a hostname but not in dns:
            
                AssemblerErrors::setErrorText(interp, 
                                              AssemblerErrors::NoDnsName, 
                                              Usage());
        }
        // Try for the host by IP address:

        pHostInformation = gethostbyaddr(&ipAddress, sizeof(struct in_addr), AF_INET);
        if (!pHostInformation) {
            return
                AssemblerErrors::setErrorText(interp, 
                                              AssemblerErrors::NoSuchHost, 
                                              Usage());
        }
        // the host has been gotten.. ensure the actual host does not exist in the table:

        if (FindNode(pHostInformation->h_name)) {
            return
                AssemblerErrors::setErrorText(interp, 
                                              AssemblerErrors::DuplicateNode, 
                                              Usage());
        }
        // Now I think we're good to install the entry.

        m_nodeTable[id].cpuId              = id;
        m_nodeTable[id].isTrigger          = false;
        m_nodeTable[id].windowDefined      = false;
        m_nodeTable[id].windowWidth        = 0;
        m_nodeTable[id].offsetDefined      = false;
        m_nodeTable[id].offset             = 0;

        m_nodeTable[id].pNodeName          = new char[strlen(pHostInformation->h_name) + 1];
        strcpy(m_nodeTable[id].pNodeName, pHostInformation->h_name);
        memcpy(&(m_nodeTable[id].ipAddress), 
               pHostInformation->h_addr_list[0], 
               sizeof (struct in_addr));
        m_definedNodes.push_back(id);

    }
    interp.setResult(objv[3]);
    return TCL_OK;
    
}
//////////////////////////////////////////////////////////////////////////////////////
// 
//   Implements the trigger subcommand.  The trigger subcommand
//   designates a node as the trigger node.
//   Parameters and returns are as for all other command processors.
int
AssemblerCommand::trigger(CTCLInterpreter& interp,
                          vector<CTCLObject>& objv)
{
    // Get the node id:

    int id;
    try {
        objv[2].Bind(interp);
        id = objv[2];
    }
    catch (...) {
        // Not an integer most likely...
        
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadId,
                                          Usage());
    }
    // Bad integer value (out of range):

    if ((id < 0) || (id > 0xffff)) {
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadId,
                                          Usage());
    }
    // Node must exist:

    if (FindNode(id)) {
        clearTrigger();                  // Reset triggerness of any existing node.
        m_nodeTable[id].isTrigger = true;
    }
    else {
        return AssemblerErrors::setErrorText(interp, 
                                             AssemblerErrors::NoSuchId,
                                             Usage());

    }
}
//////////////////////////////////////////////////////////////////////////////////////
//
//  Window subcommand processor.. Set the matching window for a node.
//  The matching window is a width and optional offset.
//
int
AssemblerCommand::window(CTCLInterpreter& interp,
                         vector<CTCLObject>& objv)
{
    // pull out the parameters:

    bool haveOffset(false);

    objv[2].Bind(interp);                  // Node id.
    objv[3].Bind(interp);                  // Mandatory width.

    int id;
    int window;
    int offset;

    try {
        id = objv[2];
    }
    catch (...) {
        // Not an integer most likely...
        
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadId,
                                          Usage());
    }
    try {
        window = objv[3];
    }
    catch (...) {
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadWindowWidth, 
                                          Usage());
    }
    // Validate the id:
    // Bad integer value (out of range):

    if ((id < 0) || (id > 0xffff)) {
        return
            AssemblerErrors::setErrorText(interp,
                                          AssemblerErrors::BadId,
                                          Usage());
    }
    // Node must exist:

    if (!FindNode(id)) {
        return AssemblerErrors::setErrorText(interp, 
                                             AssemblerErrors::NoSuchId,
                                             Usage());

    }
    // Window width must be positive:

    if (window < 0) {
        return AssemblerErrors::setErrorText(interp, 
                                             AssemblerErrors::BadWindowWidth,
                                              Usage());
    }
    //  Is there an offset? If so validate it (can be signed).

    if (objv.size() == 5) {
        haveOffset = true;
        try {
            objv[4].Bind(interp);
            offset = objv[4];
        }
        catch (...) {
            return AssemblerErrors::setErrorText(interp,
                                                 AssemblerErrors::BadOffset,
                                                 Usage());
        }
    }
    // All set for success now:

    m_nodeTable[id].windowWidth = window;
    m_nodeTable[id].windowDefined = true;
    m_nodeTable[id].offsetDefined = false;

    if (haveOffset) {
        m_nodeTable[id].offsetDefined = true;
        m_nodeTable[id].offset        = offset;
    }
    return TCL_OK;

}

/////////////////////////////////////////////////////////////////////////////////////
//
//   Command processor for the list subcommand. We iterate through the nodes that
//   are defined (using the m_definedNode list), and put the information about them
//   in lists.  We will be producing lists of lists.  first list element will
//   be the trigger node .. or empty if not defined.
//   The remainder of the list is a set of sublists for each node that has been
//   defined (see describeNode for information about the format of this list.
//
int
AssemblerCommand::list(CTCLInterpreter& interp,
                       vector<CTCLObject>& objv)
{
    CTCLObject result;
    result.Bind(interp);

    // iterate through the defined nodes getting the trigger...
    // and node information for each node

    CTCLObject trigger;
    trigger.Bind(interp);
    CTCLObject  nodeInfo;
    nodeInfo.Bind(interp);

    list<int>::iterator p = m_definedNodes.begin();
    while (p != m_definedNodes.end()) {
        int node = *p;
        if (m_nodeTable[node].isTrigger) {
            trigger = node;
        }
        CTCLObject nodeDescription;
        nodeDescription.Bind(interp);
        describeNode(nodeDescription, m_nodeTable[node]);
        nodeInfo += nodeDescription;

        p++;
    }
    // Build the final result.

    result += trigger;                // Trigger...
    result += nodeInfo;               // List of node information.

    return TCL_OK;

}
////////////////////////////////////////////////////////////////////////////////////////
//
//  Verify the sanity of the setup.
//
int
AssemblerCommand::verify(CTCLInterpreter& interp,
                         vector<CTCLObject& objv)
{
    bool foundTrigger(false);

    list<int>::iterator p = m_definedNodes.begin();
    while (p != m_definedNodes.end()) {
        int node = *p;
        if (m_nodeTable[node].isTrigger) {
            foundTrigger = true;
        }
        if (!m_nodeTable[node].windowDefined) {

            // Node underspecified.


            char noWindow[1000];
            sprintf(noWindow, "Node %s has not had a matching width specified",
                node);
            interp.setResult(noWindow);
            return TCL_ERROR;
        }

        p++;
    }
    // Is the trigger there?

    if (!foundTrigger) {
        interp.setResult("No trigger node specified");
        return TCL_ERROR;
    }
    return TCL_OK;
}
//////////////////////////////////////////////////////////////////////////////////////
//  This function dispatches commands given their keyword.
//  Parameters:
//      keyword   - The subcommand keyword.
//      interp    - The Tcl interpreter that is executing the command.
//      objv      - The vector of parameters.
//  Returns:
//     TCL_OK     - If the command was dispatched and executed correctly.
//     TCL_ERROR  - If the command was dispatched but failed.
//     TCL_ERROR  - If the command could not be dispatched.
//   In the case of TCL_ERROR, the interpreter's result is set with
//   an appropriate error message either by us or the dispatched command processor.
//
int
AssemblerCommand::Dispatch(string keyword, 
                           CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
    for (int i = 0; i < SUBCOMMANDCOUNT; i++) {
            // First attempt to match the command.
        if (keyword == string(DispatchTable[i].pKeyword)) {
            
            // Next validate the parameter count against the table:
            
            if (objv.size() < DispatchTable[i].minParameters) {
                return 
                    AssemblerErrors::setErrorText(interp, 
                                                  AssemblerErrors::TooFewParameters,
                                                  Usage());
                
            }
            if (obj.size() > DispatchTable[i].maxParameters) {
                string message = 
                    AssemblerErrors::errorText(AssemblerErrors::TooManyParameters);
                message       += '\n';
                message       += Usage();
                interp.setResult(message);
                return TCL_ERROR;
            }
            // Dispatch and return the results from that:

            return this->*DispatchTable[i].processor(intper,objv);
        }
    }
    // Invalid command keyword:

    string message = AssemblerErrors::errorText(AssemblerErrors::InvalidSubcommand);
    message += "\n";
    message += Usage();
    interp.setResult(message);
    return TCL_ERROR;
}
/////////////////////////////////////////////////////////////////////////
//
// Locate a node by id... just a matter of seeing if it is in the
// defined nodes list and then returning either a pointer to the
// entry in the node table or null if not.
//
pEventFragmentContributor
AssemblerCommand::findNode(unsigned short id)
{
    list<int>::iterator p = m_definedNodes.begin();
    while (p != m_definedNodes.end()) {
        if (id == *p) {
            return &(m_nodeTable[id]);
        }
        p++;
    }
    return static_cast<pFragmentContributor>(0);
}
/////////////////////////////////////////////////////////////////////
//
// Locate a node by its name... in this case we'll need to also
// compare the name passed into the ones in the node table that 
// correspond to defined nodes.
//
pEventFragmentContributor
AssemblerCommand::findNode(const char* node)
{
    list<int>::iterator p = m_definedNodes.begin();
    while (p != m_definedNodes.end()) {
        int id = *p;
        if (strcmp(node, m_nodeTable[id].pNodeName) == 0) {
            return &(m_nodeTable[id]);
        }
        p++;
    }
    return static_cast<pFragmentContributor>(0);
}
///////////////////////////////////////////////////////////////////
//
//   Return a string that describes the command usage:
//
string
AssemblerCommand::Usage()
{
    string result = "Usage\n";
    result       += "  assembler node dnsOrIp id\n";
    result       += "  assembler trigger id\n";
    result       += "  assembler window  id width ?offset?\n";
    result       += "  assembler list\n";
    result       += "  assembler verify\n";

    return result;
}

/////////////////////////////////////////////////////////////////////////
//
//  Reset the trigger flags in all used nodes.
//
void
AssemblerCommand::clearTrigger()
{
    list<int>::iterator p = m_definedNodes.begin();
    while (p != m_definedNodes.end()) {
        int id = *p;
        m_nodeTable[id].isTrigger = false;   // Don't bother to find the 'right one'.
        p++;
    }
}
//////////////////////////////////////////////////////////////////////////
// Produce a list that describes a node.
// The list has the following elements, and is returned in a tcl object
// that must have been bound to an interpreter;
//  nodeName nodeIp nodeId matchingWidth offset
//
//  If the matching window has not been defined, then both matchingWidth
//  and offset are {}'s.  If the matching window has been defined, but not
//  the offet, the offset is shown as zero.
//
void
AssemblerCommand::describeNode(CTCLObject&               description,
                               EventFragmentContributor& node)
{
    struct in_addr inet;
    inet.s_addr = node.ipAddress;

    description += node.pNodeName;
    description += inet_ntoa(inet);
    description += node.cpuId;

    if (node.windowDefined) {
        description += node.windowWidth;
        int width = node.offset;
        if (!node.offsetDefined) width = 0;
        description += width;
    }
    else {
        description += "";      // Window width.
        description += "";      // Window Offset.
    }
}