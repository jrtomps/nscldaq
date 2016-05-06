/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   Implements api instances objects that are tcl bindings to CVardbRingBuffer object.
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/


#include "CTCLVardbRingBufferInstance.h"
#include "CVardbRingBuffer.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include <stdexcept>
#include <tcl.h>
#include <Exception.h>

/**
 * constructor
 *    @param interp  - interpreter on whic the command is being registered.
 *    @param command - name of the command.
 *    @param uri     - URI specifying connection to the database.
 *    
 */
CTCLVardbRingBufferInstance::CTCLVardbRingBufferInstance(
    CTCLInterpreter& interp, const char* command, const char* uri
) :
    CTCLObjectProcessor(interp, command, true),
    m_pApi(0)
{
        m_pApi = new CVardbRingBuffer(uri);
}

/**
 * destructor
 */
CTCLVardbRingBufferInstance::~CTCLVardbRingBufferInstance()
{
    delete m_pApi;
}

/**
 * operator()
 *    Gains control when the command is executing.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - command line words.
 *  @return int   -  TCL_OK on success, TCL_ERROR on error.
 */
int
CTCLVardbRingBufferInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    int result = TCL_OK;
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2);
        std::string subcommand = objv[1];
        
        if (subcommand == "haveSchema") {
            haveSchema(interp, objv);
        } else if (subcommand == "createSchema") {
            createSchema(interp, objv);
        } else if (subcommand == "create") {
            create(interp, objv);    
        } else if (subcommand == "destroy") {
            destroy(interp, objv);
        } else if (subcommand == "setMaxData") {
            setMaxData(interp, objv);    
        } else if (subcommand == "setMaxConsumers") {
            setMaxConsumers(interp, objv);
        } else if (subcommand == "ringInfo") {
            ringInfo(interp, objv);
        } else if (subcommand == "list") {
            list(interp, objv);
        } else if (subcommand == "setEditorPosition") {
                
            setEditorPosition(interp, objv);
        } else if (subcommand == "getEditorXPosition") {
                
            getEditorXPosition(interp, objv);
        } else if (subcommand == "getEditorYPosition") {
            getEditorYPosition(interp, objv);
        } else {
            throw std::runtime_error("api instance - invalid subcommand");
        }
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        result = TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        result = TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        result = TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        result = TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught report this as a bug");
        result = TCL_ERROR;
    }
    
    return result;                
}
/**
 * haveSchema
 *    wraps the API haveSchema operation.
 *
 *  @param interp - The interpreter that is running the command.
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
 *         been initialized and false otherwise.
 */
void CTCLVardbRingBufferInstance::haveSchema(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "haveSchema - incorrect parameter count");
    interp.setResult(Tcl_NewBooleanObj(m_pApi->haveSchema()));
}
/**
 * createSchema
 *    Creates a schema for managing ring buffer definitions.
 *
 *  @param interp - The interpreter that is running the command.
 *  @param objv   - Vector of command line words.
 */
void CTCLVardbRingBufferInstance::createSchema(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "createSchema - incorrect parameter count");
    m_pApi->createSchema();
}
/**
 * create
 *     Make a new ring buffer. the name and host parameters are required.  A dict that
 *     can have the following keys is optional:
 *     -   datasize - number of bytes of data storage.
 *     -   maxconsumers - Maximum number of consumers.
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
*/
void
CTCLVardbRingBufferInstance::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 4);
    requireAtMost(objv, 5);
    
    std::string name = objv[2];
    std::string host = objv[3];
    
    Tcl_Obj* pDict = Tcl_NewDictObj();
    if (objv.size() == 5) {
        pDict  = objv[4].getObject();
    }
    
    
    
    // Default values for other parameters
    
    unsigned dataSize     = 8*1024*1024;
    unsigned maxConsumers = 100;
    
    // If the dict has the parameters, override them:
    
    getDictUnsigned(interp, pDict, "datasize", &dataSize);
    getDictUnsigned(interp, pDict, "maxconsumers", &maxConsumers);
    
    m_pApi->create(name.c_str(), host.c_str(), dataSize, maxConsumers);
}
/**
 * destroy
 *    Destroy a ring buffer definition.
 *
 *
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
*/
void
CTCLVardbRingBufferInstance::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4);
    std::string name = objv[2];
    std::string host = objv[3];
    
    m_pApi->destroy(name.c_str(), host.c_str());
}

/**
 * setMaxData
 *    Sets the data size for a ring buffer that already exists.
 *
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
*/
void
CTCLVardbRingBufferInstance::setMaxData(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 5);
    std::string name = objv[2];
    std::string host = objv[3];
    unsigned    size = sizeValue(objv[4]);
 
    
    m_pApi->setMaxData(name.c_str(), host.c_str(), size);
}
/**
 * setMaxConsumers
 *    Sets the maximum number of simultaneous consumers
 *
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
*/
void
CTCLVardbRingBufferInstance::setMaxConsumers(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 5);
    std::string name = objv[2];
    std::string host = objv[3];
    unsigned    size = sizeValue(objv[4]);
 
    
    m_pApi->setMaxConsumers(name.c_str(), host.c_str(), size);
}
/**
 * ringInfo
 *    Leave a dict in the interp result that describes a ringbuffer.
 *    The dict has the following keys:
 *    - name  - name of the ring.
 *    - host  - host the ring is in.
 *    - datasize - number of bytes of data buffer the ring has.
 *    - maxconsumers - maximum number of consumers.
 *
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
*/
void
CTCLVardbRingBufferInstance::ringInfo(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv,  4);
    std::string name = objv[2];
    std::string host = objv[3];
    
    CVardbRingBuffer::RingInfo info = m_pApi->ringInfo(name.c_str(), host.c_str());
    
    Tcl_Obj* dict = ringDict(interp, &info);
    interp.setResult(dict);
}

/**
 * list
 *    Create a Tcl List of dicts that describe all defined rings.
 *  @param objv   - Vector of command line words.
 *  @note The interpreter result is a boolean that is true if the schema has
*/
void
CTCLVardbRingBufferInstance::list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2);
    
    std::vector<CVardbRingBuffer::RingInfo> listing = m_pApi->list();
    CTCLObject result;
    result.Bind(interp);
    
    for (auto p = listing.begin(); p != listing.end(); p++  ) {
        Tcl_Obj* pDictObj = ringDict(interp, &(*p));   // p is interator not ptr.
        CTCLObject dict(pDictObj);
        dict.Bind(interp);
        result += dict;
    }
    interp.setResult(result);
}
/**
 * setEditorPosition
 *    Set a new position for the object on a canvas.
 *
 *  @param interp - interpreter running the command.
 *  @param objv   - command line words.
 */
void
CTCLVardbRingBufferInstance::setEditorPosition(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
        requireExactly(objv, 6);
        
        std::string ring = objv[2];
        std::string host = objv[3];
        int x            = objv[4];
        int y            = objv[5];
        
        m_pApi->setEditorPosition(ring.c_str(), host.c_str(), x, y);
}
/**
 * getEditorXPosition
 *   Set the interpreter result with the X position of the object on the
 *   editor canvas.
 *  @param interp - interpreter running the command.
 *  @param objv   - command line words.
 */
void
CTCLVardbRingBufferInstance::getEditorXPosition(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
        requireExactly(objv, 4);
        std::string ring = objv[2];
        std::string host = objv[3];
        
        int x = m_pApi->getEditorXPosition(ring.c_str(), host.c_str());
        CTCLObject result;
        result.Bind(interp);
        
        result = x;
        interp.setResult(result);
        
}
/**
 * getEditorYPosition
 *   Set the interpreter result with the X position of the object on the
 *   editor canvas.
 *  @param interp - interpreter running the command.
 *  @param objv   - command line words.
 */
void
CTCLVardbRingBufferInstance::getEditorYPosition(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
        requireExactly(objv, 4);
        std::string ring = objv[2];
        std::string host = objv[3];
        
        int y = m_pApi->getEditorYPosition(ring.c_str(), host.c_str());
        CTCLObject result;
        result.Bind(interp);
        
        result = y;
        interp.setResult(result);
        
}
/*-----------------------------------------------------------------------------
 *  Utilties
 */

/**
 * getDictUnsigned
 *    If a dict has the requested key its value overrides the value
 *    passed in:
 *
 *  @param interp  - The interpreter executing this command.
 *  @param dict    - Pointer to a dict object.
 *  @param key     - Pointer to the key string.
 *  @param pValue  - Pointer to the value.  If dict is a dict and it has key
 *                   _and_ the key is integer, the value is assigned they key.
 * @throw std::domain_error - if the value for the key is not  positive integer.
 */
void
CTCLVardbRingBufferInstance::getDictUnsigned(
    CTCLInterpreter& interp, Tcl_Obj* dict, const char* key, unsigned* pValue
)
{
    // The key must be turned into a Tcl_Obj:
    
    CTCLObject keyObj;
    keyObj.Bind(interp);
    keyObj = key;
    
    Tcl_Obj* dictValue(NULL);
    
    int status = Tcl_DictObjGet(
        interp.getInterpreter(), dict, keyObj.getObject(), &dictValue
    );
    if (status != TCL_OK) {
        throw std::domain_error("Not a dict but must be");
    }
    
    if (dictValue == NULL) return;                    // NO such key.
    // Require dictValue is integer and positive .. else domain error too.
    
    int proposed;
    status =
        Tcl_GetIntFromObj(interp.getInterpreter(), dictValue, &proposed);
    if(status != TCL_OK) {
        throw std::domain_error("Dict value does not have integer rep.");
    }
    if (proposed <= 0) {
        throw std::domain_error("Dict value must be > 0.");
    }
    *pValue = proposed;

}
/**
 * sizeValue
 *   @param obj  - object to extract an unsigned value from.
 *   @return unsigned - obj interpreted as a size.
 *   @throw std::domain_error - not > 0.
 *   @throw CTCLException (no integer rep).
 */
unsigned
CTCLVardbRingBufferInstance::sizeValue(CTCLObject& obj)
{
    int value = obj;
    if (value <= 0) {
        throw std::domain_error("Value must be > 0");
    }
    return value;
}


/**
 * ringDict
 *    Turn a RingInfo struct into a standard dict that describes a ring.
 *    See ringInfo above for information about the dict keys.
 *
 * @param interp - interpreter that can be used with the Tcl api.
 * @param pInfo  - Pointer to the RingInfo struct describing the ring.
 * @return Tcl_Obj* - pointer to the created dict.
 */
Tcl_Obj*
CTCLVardbRingBufferInstance::ringDict(CTCLInterpreter& interp, CVardbRingBuffer::pRingInfo pInfo)
{
    Tcl_Obj* pResult = Tcl_NewDictObj();
    putDict(interp, pResult, "name", pInfo->s_name);
    putDict(interp, pResult, "host", pInfo->s_host);
    putDict(interp, pResult, "datasize", pInfo->s_dataSize);
    putDict(interp, pResult, "maxconsumers", pInfo->s_maxConsumers);
    
    return pResult;
}
/**
 * putDict
 *    Put a value in a Tcl dict.  This overload puts a value that is an
 *    std::string
 *
 *  @param interp - interpreter object.
 *  @param pDict  - Pointer to the dict being modified.
 *  @param pKey   - Pointer to the text containing the key.
 *  @param value  - std::string value.
 */
void
CTCLVardbRingBufferInstance::putDict(
    CTCLInterpreter& interp, Tcl_Obj* pDict, const char* pKey, std::string value
)
{
    Tcl_Obj* pKeyObj = Tcl_NewStringObj(pKey, -1);
    Tcl_Obj* pValObj = Tcl_NewStringObj(value.c_str(), -1);
    
    Tcl_DictObjPut(interp.getInterpreter(), pDict, pKeyObj, pValObj);
    
}
/**
 * putDict
 *    Put a value in to a TclDict. This overload puts a value that is unsigned.
 *
 *  @param interp - interpreter object.
 *  @param pDict  - Pointer to the dict being modified.
 *  @param pKey   - Pointer to the text containing the key.
 *  @param value  - std::string value.
 */
void
CTCLVardbRingBufferInstance::putDict(
    CTCLInterpreter& interp, Tcl_Obj* pDict, const char* pKey, unsigned value
)
{
    Tcl_Obj* pKeyObj = Tcl_NewStringObj(pKey, -1);
    Tcl_Obj* pValObj = Tcl_NewIntObj(value);
    
    Tcl_DictObjPut(interp.getInterpreter(), pDict, pKeyObj, pValObj);
    
    
}