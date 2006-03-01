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
#include <math.h>
#include <errno.h>
#include <limits.h>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_PIPEINJECTOR_H
#include <dshapi/PipeInjector.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_FDSELECTOR_H
#include <dshapi/FdSelector.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

#ifndef DAQHWYAPI_DSHUTILS_H
#include <dshapi/DSHUtils.h>
#endif

namespace daqhwyapi {
/**
* @var pipeinjector_ioexception
* @brief Exception to throw for IO exceptions.
*
* Exception to throw for IO exceptions.
*/
static IOException pipeinjector_ioexception;

/**
* @var pipeinjector_rtexception
* @brief Exception to throw for Runtime exceptions.
*
* Exception to throw for Runtime exceptions.
*/
static RuntimeException pipeinjector_rtexception;

#define __MY_BUFFERSIZE__ 8192

} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn PipeInjector::PipeInjector()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
PipeInjector::PipeInjector() {
  mainin = NULL;
  mainout = NULL;
  injector = NULL;
  errorin = NULL;
  errorout = NULL;
  ignore_injector = false;
  rec_type = Record::type_unknown;
}

/*==============================================================*/
/** @fn PipeInjector::~PipeInjector()
* @brief Destructor.
*
* Destroy this PipeInjector.
*
* @param None
* @return None
*/                                                             
PipeInjector::~PipeInjector() {
  if (mainout != NULL) mainout->flush();
  if (errorout != NULL) errorout->flush();
  mainin = NULL;
  mainout = NULL;
  injector = NULL;
  errorin = NULL;
  errorout = NULL;
  rec_type = Record::type_unknown;
}

/*==============================================================*/
/** @fn bool PipeInjector::connect(FdInputStream& aInError,FdOutputStream& aOutError)
* @brief Connect an input/output stream error stream pair.
*
* Connect a Fd input stream and an Fd output stream as a pair
* of error streams.  That is, error messages will be read from aInError
* and written to aOutError. 
*
* @param aInError The error input stream to connect.
* @param aOutError The error output stream to connect.
* @return If the connection was successful.
* @throw IOException If error streams are already connected.
*/                                                             
bool PipeInjector::connect(FdInputStream& aInError,FdOutputStream& aOutError) {
  if (errorin != NULL) throw pipeinjector_ioexception.format(CSTR("PipeInjector::connect(): Error input/output streams already connected."));
  errorin = &aInError;
  errorout = &aOutError;
  return true;
}


/*==============================================================*/
/** @fn bool PipeInjector::connect(FdInputStream& aStream)
* @brief Connect an input stream to this connector.
*
* Connect a Fd input stream to this connector.
* This stream is the main input stream into this injector.
*
* @param aStream The input stream to connect.
* @return If the connection was successful.
* @throw IOException If already connected to an input stream.
*/                                                             
bool PipeInjector::connect(FdInputStream& aStream) {
  if (mainin != NULL) throw pipeinjector_ioexception.format(CSTR("PipeInjector::connect(): Main input stream already connected."));
  mainin = &aStream;
  return true;
}

/*==============================================================*/
/** @fn bool PipeInjector::connect(FdOutputStream& aStream)
* @brief Connect an output stream to this connector.
*
* Connect a Fd output stream to this connector.
* This stream is the main output stream out of this injector.
*
* @param aStream The output stream to connect.
* @return If the connection was successful.
* @throw IOException If already connected to an output stream.
*/                                                             
bool PipeInjector::connect(FdOutputStream& aStream) {
  if (mainout != NULL) throw pipeinjector_ioexception.format(CSTR("PipeInjector::connect(): Main output stream already connected."));
  mainout = &aStream;
  return true;
}

/*==============================================================*/
/** @fn bool PipeInjector::connect(FdRecordReader& aReader)
* @brief Connect a Reader to this injector.
*
* Connect a Reader to this injector.  The reader will act
* to read records that will be subsequently injected into
* the main stream of the pipeline.
*
* Once this Reader emits an eof, the reader will be 
* shutdown and ignored from that point forward.
*
* @param aReader The reader to connect.
* @return If the connection was successful.
* @throw IOException If already connected to an injection reader.
*/                                                             
bool PipeInjector::connect(FdRecordReader& aReader) {
  if (injector != NULL) throw pipeinjector_ioexception.format(CSTR("PipeInjector::connect(): Injection reader already connected."));
  injector = &aReader;
  return true;
}

/*==============================================================*/
/** @fn void PipeInjector::setRecordType(uint32_t aType)
* @brief Set the ouput record type.
*
* Set the output record type.  All records injected by this 
* injector's child program will by output with the specified
* record type.  If no record type has been set, all records will
* have a type of Record::type_unknown. 
*
* @param aType The new record type.
* @return None
*/                                                             
void PipeInjector::setRecordType(uint32_t aType) {
  rec_type = aType;
}

/*==============================================================*/
/** @fn void PipeInjector::run()
* @brief Injector main loop.
*
* Implements the injector main loop.  The input, output and
* reader should all be connected prior to calling start() and
* beginning the main loop.
*
* Part of the Runnable interface.
*
* @param None
* @return None
*/                                                             
void PipeInjector::run() {
  int rlen = 0;         // Current record length
  FdSelector selector;  // For selecting when not on a rec boundary
  ByteArray rec(__MY_BUFFERSIZE__);  // For reading injection records
  record_header_t hdr;  // A record header

  // Initialize
  Record::initHeader(hdr);

  // Don't ignore the injector just yet
  ignore_injector = false;

  // If on a record boundary anything goes, so add all file descriptors
  selector.addReadFd(mainin->getFD());
  selector.addReadFd(errorin->getFD());
  selector.addReadFd(injector->getFD());

  // Main loop continues until eof on the primary input stream
  while ((!ignore_injector)&&(!mainin->eof())&&(rlen >= 0)) {
    if (rlen == 0) { // at a record boundary
      // See if something is availble somewhere
      selector.select();
      check_errorinput();
      if (!ignore_injector) {
        check_injector();
        if (!ignore_injector) {
          rlen = check_maininput(hdr); 
          if (rlen > 0) rec.renew(rlen);
        } else {
          break;  // Short circuit the while loop
        }
      }
    } 

    if (rlen == 0) continue; // Nothing on maininput yet.

    // Ok, read from maininput.
    int rrc = mainin->read(rec.elements,rlen);
    if (rrc < 0) break; // EOF on input

    // Now write the new data
    int bn = rrc;
    int oset = 0;
    int wrc = 0;
    while (bn > 0) {
      wrc = mainout->write(rec.elements,rrc);
      if (wrc < 0) break; // EOF on output
      bn -= wrc;
      oset += wrc;
      rlen -= wrc; // Adjust count
    }
    if (wrc < 0) break;  // EOF -- break main loop
  }

  // Flush everything just in case
  mainout->flush();
  errorout->flush();

  if (rlen > 0) { // Short record write
    throw pipeinjector_ioexception.format(CSTR("PipeInjector::run() Failed to finish writing records.  Wrote %d bytes of a record with byte length %d."),(hdr.record_size - rlen),hdr.record_size);
  }
}

/*==============================================================*/
/** @fn void PipeInjector::start()
* @brief Start the injector main loop.
*
* Starts the injector main loop.  The input, output and
* reader should all be connected prior to calling this method.
*
* Part of the Runnable interface.
*
* @param None
* @return None
* @throws RuntimeExecption On input, output, or reader stream not connected.
*/                                                             
void PipeInjector::start() {
  if (mainin == NULL) throw pipeinjector_rtexception.format(CSTR("PipeInjector::start(): Primary input stream not connected"));
  if (mainout == NULL) throw pipeinjector_rtexception.format(CSTR("PipeInjector::start(): Primary output stream not connected"));
  if (injector == NULL) throw pipeinjector_rtexception.format(CSTR("PipeInjector::start(): Injection reader stream not connected"));
  if (errorin == NULL) throw pipeinjector_rtexception.format(CSTR("PipeInjector::start(): Error input stream not connected"));
  if (errorout == NULL) throw pipeinjector_rtexception.format(CSTR("PipeInjector::start(): Error output stream not connected"));

  try {
    // Run the main loop
    this->run();

    // Null streams when done
    if (mainin != NULL) mainin = NULL;
    if (mainout != NULL) mainout = NULL;
    if (injector != NULL) injector = NULL;
    if (errorin != NULL) errorin = NULL;
    if (errorout != NULL) errorout = NULL;
  } catch (std::exception& dhae) {
    // Null streams when done
    if (mainin != NULL) mainin = NULL;
    if (mainout != NULL) mainout = NULL; 
    if (injector != NULL) injector = NULL;
    if (errorin != NULL) errorin = NULL;
    if (errorout != NULL) errorout = NULL;

    // And rethrow the exception
    throw dhae;
  } 
}

/*==============================================================*/
/** @fn void PipeInjector::check_injector()
* @brief Check the injector for a record.
*
* Check the injector for a record.
*
* @param None
* @return None
*/                                                             
void PipeInjector::check_injector() {
  ByteArray rec; // For reading injection records
  record_header_t hdr;
  ubyte hdrbuf[Record::encode_buffer_size];

  // Initialize
  Record::initHeader(hdr);

  // If we are at eof then...
  int ieof = injector->eof();
  if (ieof) {
    ignore_injector = true;
    return;
  }

  // Now check the injector
  if (injector->ready()) {
    int rc = injector->readRecord(hdr,rec);
    if (rc <= 0) {  // EOF or error
      injector->close();
      ignore_injector = true;
    } else if (rc > 0) {  // Add the record to the main stream
      int bn = rc;
      int oset = 0;

      // First the header
      hdr.record_type = rec_type;
      int hn = Record::encodeHeader(hdrbuf,Record::encode_buffer_size,hdr);
      while (hn > 0) {  // Inject the header
        int wrc = mainout->write(hdrbuf,oset,hn); 
        if (wrc < 0) { // EOF on output
          mainout->close();
          bn = -1; // Don't write data
          break;
        } else {
          hn -= wrc;
          oset += wrc;
        }
      }
      
      // Now the data
      oset = 0;
      while (bn > 0) {  // Inject the data
        int wrc = mainout->write(rec.elements,oset,bn);
        if (wrc < 0) { // EOF on output
          mainout->close();
          break;
        } else {
          bn -= wrc;
          oset += wrc;
        }
      }
    }
  }
}

/*==============================================================*/
/** @fn int PipeInjector::check_maininput(record_header_t& hdr)
* @brief Check the main input stream for a new record.
*
* Check the main input stream for a new record.
*
* @param hdr A record header to fill in.
* @return The remaining length of the new record to read or <0 on error.
*/                                                             
int PipeInjector::check_maininput(record_header_t& hdr) {
  ubyte hdrbuf[Record::encode_buffer_size]; // For reading headers
  int rlen = 0;

  if (mainin->ready()) { // Have some bytes on input
    // Maybe we have enough for a new header 
    if (mainin->available() >= Record::encode_buffer_size) {
      int rc = mainin->read(hdrbuf,Record::encode_buffer_size); 
      if (rc < Record::encode_buffer_size) { // EOF or other error
        rlen = -1;
      } else {
        rc = mainout->write(hdrbuf,Record::encode_buffer_size); // Write the header 
        if (rc < 0) { // EOF on output
          mainout->close();
          rlen = -2;
        } else { // Write was ok
          // If we can't decode the header, we have a problem
          if (Record::decodeHeader(hdrbuf,Record::encode_buffer_size,hdr) < 0) {
            throw pipeinjector_rtexception.format(CSTR("PipeInjector::check_maininput() Failed to decode header"));
          }
          // Calculate the remaining record size
          rlen = hdr.record_size + hdr.extended_header_size - Record::encode_buffer_size;  
        }
      }
    }
  }

  return rlen;
}

/*==============================================================*/
/** @fn void PipeInjector::check_errorinput()
* @brief Check the erro input stream for a new message.
*
* Check the error input stream for a new message.
*
* @param None.
* @return None.
*/                                                             
void PipeInjector::check_errorinput() {
  ubyte msgbuf[__MY_BUFFERSIZE__]; // For reading messages 

  if (errorin->ready()) { // Have some bytes on input
    int avail = errorin->available();
    avail = (avail > __MY_BUFFERSIZE__) ? __MY_BUFFERSIZE__ : avail;
    if (avail > 0) {
      int rc = errorin->read(msgbuf,avail); 
      if (rc > 0) { 
        rc = errorout->write(msgbuf,rc); // Write the message
        errorout->flush();
        if (rc <= 0) { // EOF on output
          errorout->close();
          rc = -1;
        } 
      } 
    }
  }
}
