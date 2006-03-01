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
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

namespace daqhwyapi {

typedef struct record_encoded_header { 
  uint16_t version;
  uint32_t record_size;
  uint32_t record_type;
  uint16_t status_code;
  uint32_t byte_order;
  uint32_t extended_header_size;
  uint32_t data_size;
  uint32_t entity_count;
} record_encoded_header_t;


/**
* @var record_rtexception
* @brief Exception to throw for Runtime exceptions.
*
* Exception to throw for Runtime exceptions.
*/
static RuntimeException record_rtexception;
  
} // namespace daqhwyapi

using namespace daqhwyapi;

uint16_t Record::current_version = 1;
int Record::encode_buffer_size = sizeof(record_encoded_header_t);
int Record::default_max_record_size = (1024*1024);

uint16_t Record::status_ok = 0;       // Record Ok
uint16_t Record::status_err = 1;      // Unspecified record error
uint16_t Record::status_trunc = 2;    // Record truncated
uint16_t Record::status_runshort = 3; // Run incomplete

// These were all defined in nscldaq-8.0-pre1
uint32_t Record::type_unknown = 0;
uint32_t Record::type_data = 1;
uint32_t Record::type_scaler = 2;
uint32_t Record::type_snapsc = 3;
uint32_t Record::type_statevar = 4;
uint32_t Record::type_runvar = 5;
uint32_t Record::type_pktdoc = 6;
uint32_t Record::type_stats = 9;
uint32_t Record::type_beginrun = 11;
uint32_t Record::type_endrun = 12;
uint32_t Record::type_paramdescript = 30;

// These are new record types
uint32_t Record::type_badend = 7;
uint32_t Record::type_continue = 8;
uint32_t Record::type_physics = 1;

// All user packet types must have a value greater than
// those used by the system, so...
uint32_t Record::type_maxsystem = 128;

// Record name to type mappings
static uint32_t record_type_ids[] = {
  1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 30, (uint32_t)-1 
};

static char *record_type_names[] = {
  "Physics",
  "Data",
  "Scaler",
  "SnapSc",
  "StateVar",
  "RunVar",
  "PktDoc",
  "BadEnd",
  "ContinueNextFile",
  "Stats",
  "BeginRun",
  "EndRun",
  "ParamDescript",
  NULL
};

// Reserved system types (those that cannot be specified either
// numerically or by symbolic name by a user).
static uint32_t record_type_reserved[] = {
  7, 8, 11, 12, (uint32_t)-1 
};

/*===================================================================*/
/** @fn Record::Record()
* @brief Default constructor.
*                                        
* Default concstructor.
*                                         
* @param None
* @return this
*/      
Record::Record() { }

/*===================================================================*/
/** @fn void Record::initHeader(record_header_t& rHeader)
* @brief Initialize a record header.
*                                        
* Initialize a record header by setting most sizes to zero
* and setting the byte order.
*                                         
* @param rHeader The header to initialize.
* @return None
*/      
void Record::initHeader(record_header_t& rHeader) { 
  rHeader.version = Record::current_version;
  rHeader.record_size = Record::encode_buffer_size;
  rHeader.record_type = Record::type_physics;
  rHeader.status_code = Record::status_ok;
  rHeader.byte_order = 0x01020304;
  rHeader.extended_header_size = 0;
  rHeader.data_size = 0;
  rHeader.entity_count = 0;
}

/*===================================================================*/
/** @fn int Record::encodeHeader(ubyte *buf,int blen,record_header_t& rHeader)
* @brief Encode a record header.
*                                        
* Encode a record header for transfer over the DAQ pipeline.
*                                         
* @param buf Output buffer for the encoded header.
* @param blen Length of the output buffer.
* @param rHeader The header to encode.
* @return The number of output buffer bytes consumed or -1.
* @retval The nubmer of bytes consumed on sucessful encoding.
* @retval -1 If buf is not long enough to complete encoding.
*/      
int Record::encodeHeader(ubyte *buf,int blen,record_header_t& rHeader) { 
  if ((blen < Record::encode_buffer_size)||(buf == NULL)) return -1; 
  record_encoded_header_t *enchdr = (record_encoded_header_t*)buf;
  enchdr->version = htons(rHeader.version);
  enchdr->record_size = htonl(rHeader.record_size);
  enchdr->record_type = htonl(rHeader.record_type);
  enchdr->status_code = htons(rHeader.status_code);
  enchdr->byte_order = rHeader.byte_order;
  enchdr->extended_header_size = htonl(rHeader.extended_header_size);
  enchdr->data_size = htonl(rHeader.data_size);
  enchdr->entity_count = htonl(rHeader.entity_count);
  return Record::encode_buffer_size;
}

/*===================================================================*/
/** @fn int Record::decodeHeader(ubyte *buf,int blen,record_header_t& rHeader)
* @brief Decode a record header.
*                                        
* Decode a record header from the DAQ pipeline.
*                                         
* @param buf Input buffer for the encoded header.
* @param blen Length of the input buffer.
* @param rHeader Output decoded header.
* @return The number of the input buffer bytes used.
* @retval The nubmer of bytes used on sucessful decode.
* @retval -1 If buf is not long enough to complete decoding.
* @throw RuntimeException On version mismatch.
*/      
int Record::decodeHeader(ubyte *buf,int blen,record_header_t& rHeader) { 
  if ((blen < Record::encode_buffer_size)||(buf == NULL)) return -1; 
  record_encoded_header_t *enchdr = (record_encoded_header_t*)buf;
  rHeader.version = ntohs(enchdr->version);

  if (rHeader.version != Record::current_version) {
    throw record_rtexception.format(CSTR("Record::decodeHeader() Record header version mismatch %d != %d"),rHeader.version,Record::current_version);
  }   
 
  rHeader.record_size = ntohl(enchdr->record_size);
  rHeader.record_type = ntohl(enchdr->record_type);
  rHeader.status_code = ntohs(enchdr->status_code);
  rHeader.byte_order = enchdr->byte_order;
  rHeader.extended_header_size = ntohl(enchdr->extended_header_size);
  rHeader.data_size = ntohl(enchdr->data_size);
  rHeader.entity_count = ntohl(enchdr->entity_count);
  return Record::encode_buffer_size;
}

/*===================================================================*/
/** @fn void Record::copyHeader(record_header_t& rDst,record_header_t& rSrc)
* @brief Copy a record header to another record header.
*                                        
* Copy a record header to another record header.
*                                         
* @param rDst The destination record header.
* @param rSrc The source record header.
* @return None
*/      
void Record::copyHeader(record_header_t& rDst,record_header_t& rSrc) { 
  rDst.version = rSrc.version;
  rDst.record_size = rSrc.record_size;
  rDst.record_type = rSrc.record_type;
  rDst.status_code = rSrc.status_code;
  rDst.byte_order = rSrc.byte_order;
  rDst.extended_header_size = rSrc.extended_header_size;
  rDst.data_size = rSrc.data_size;
  rDst.entity_count = rSrc.entity_count;
}

/*===================================================================*/
/** @fn void Record::encodeTime(struct tm& rTime);
* @brief Encode a time for inclusion as record data.
*                                        
* Encode a time (struct tm) in network byte order for inclusion
* as record data.  The method operates in place, ie, rTime is
* returned encoded.
*                                         
* @param rTime The struct tm to encode. 
* @return None
*/      
void Record::encodeTime(struct tm& rTime) {
  // The struct tm has the following structure:
  //
  //   struct tm {
  //     int     tm_sec;         /* seconds */
  //     int     tm_min;         /* minutes */
  //     int     tm_hour;        /* hours */
  //     int     tm_mday;        /* day of the month */
  //     int     tm_mon;         /* month */
  //     int     tm_year;        /* year */
  //     int     tm_wday;        /* day of the week */
  //     int     tm_yday;        /* day in the year */
  //     int     tm_isdst;       /* daylight saving time */
  //   };
  //
  // The variables will be encoded in network byte order for portablility.
  //
  rTime.tm_sec = htonl(rTime.tm_sec);
  rTime.tm_min = htonl(rTime.tm_min);
  rTime.tm_hour = htonl(rTime.tm_hour);
  rTime.tm_mday = htonl(rTime.tm_mday);
  rTime.tm_mon = htonl(rTime.tm_mon);
  rTime.tm_year = htonl(rTime.tm_year);
  rTime.tm_wday = htonl(rTime.tm_wday);
  rTime.tm_yday = htonl(rTime.tm_yday);
  rTime.tm_isdst = htonl(rTime.tm_isdst);
}

/*===================================================================*/
/** @fn void Record::decodeTime(struct tm& rTime);
* @brief Decode a time.
*                                        
* Decode a time (struct tm) into host byte order.
* The method operates in place, ie, rTime is returned encoded.
*                                         
* @param rTime The struct tm to decode. 
* @return None
*/      
void Record::decodeTime(struct tm& rTime) {
  // The struct tm has the following structure:
  //
  //   struct tm {
  //     int     tm_sec;         /* seconds */
  //     int     tm_min;         /* minutes */
  //     int     tm_hour;        /* hours */
  //     int     tm_mday;        /* day of the month */
  //     int     tm_mon;         /* month */
  //     int     tm_year;        /* year */
  //     int     tm_wday;        /* day of the week */
  //     int     tm_yday;        /* day in the year */
  //     int     tm_isdst;       /* daylight saving time */
  //   };
  //
  // The variables will be decoded into host byte order.
  //
  rTime.tm_sec = ntohl(rTime.tm_sec);
  rTime.tm_min = ntohl(rTime.tm_min);
  rTime.tm_hour = ntohl(rTime.tm_hour);
  rTime.tm_mday = ntohl(rTime.tm_mday);
  rTime.tm_mon = ntohl(rTime.tm_mon);
  rTime.tm_year = ntohl(rTime.tm_year);
  rTime.tm_wday = ntohl(rTime.tm_wday);
  rTime.tm_yday = ntohl(rTime.tm_yday);
  rTime.tm_isdst = ntohl(rTime.tm_isdst);
}

/*===================================================================*/
/** @fn void Record::packTimeStamped(record_header_t& rHdr,ByteArray& rData,unsigned int aType,unsigned long aRun,String& rMsg)
* @brief Build a time stamped record.
*                                        
* Build (pack) a time stamped record that includes a message.
* The time stamp is a "struct tm" encoded in network byte order for
* portability.  The record header is not encoded when returned.
*                                         
* @param rHdr Output.  The header for a time stamped record.
* @param rData Output.  Data for the time stamped record.
* @param aType Record type (such as Record::type_beginrun).
* @param aRun Run number.
* @param rMsg Message to add to the record (usually the title).
* @return None 
* @throw RuntimeException If time can not be determined.
*/      
void Record::packTimeStamped(record_header_t& rHdr,ByteArray& rData,unsigned int aType,unsigned long aRun,String& rMsg) {
  // Get the current time
  time_t mytime;
  if (time(&mytime) == ((time_t)-1)) {
    char ebuf[DAQHWYAPI_STRERROR_SIZE+1];
    throw record_rtexception.format(CSTR("Record::packTimeStamped() Failed to retrieve current time: msg=\"%s\" rc=%d"),strerror_r(errno,ebuf,DAQHWYAPI_STRERROR_SIZE),errno);
  }

  // Now convert to localtime
  struct tm thetime;
  if (localtime_r(&mytime,&thetime) == NULL) {
    throw record_rtexception.format(CSTR("Record::packTimeStamped() Failed to convert to localtime."));
  }

  // Encode the time
  Record::encodeTime(thetime);
  
  // Resize the record data to accomodate the message and time stamp.
  int msglen = rMsg.size();

  // time + run number + null terminated msg
  int datalen = sizeof(thetime)+sizeof(unsigned long)+(msglen+1);
  rData.renew(datalen);

  // Add the time to the record data.  
  ubyte *p = rData.elements;
  memcpy((char*)p,(char*)&thetime,sizeof(thetime));
  p += sizeof(thetime);

  // Add the run number
  uint32_t num = htonl(aRun);
  memcpy(p,(char*)&num,sizeof(num));
  p += sizeof(num);

  // Add the message and the null terminator
  if (msglen > 0) memcpy(p,rMsg.c_str(),msglen+1);
  else (*p) = '\0'; // Empty null terminated message. 

  // Build the record header
  Record::initHeader(rHdr);
  rHdr.record_size = Record::encode_buffer_size + datalen;
  rHdr.record_type = aType;
  rHdr.data_size = datalen;
  rHdr.entity_count = 1;
}


/*===================================================================*/
/** @fn void Record::unpackTimeStamped(record_header_t& rHdr,ByteArray& rData,struct tm& rTime,unsigned long& rRun,String& rMsg)
* @brief Unpack a time stamped record.
*                                        
* Unpack a time stamped record that includes a message.
* The time stamp is a "struct tm" encoded in network byte order for
* portability.  The record header is not encoded when returned.
*                                         
* @param rHdr The decoded header for a time stamped record.
* @param rData Data for the time stamped record.
* @param rTime Output.  Time stamp for this record.
* @param rRun Output. Run number.
* @param rMsg Output. The record message.
* @return None 
* @throw RuntimeException If unpacking is not possible.
*/      
void Record::unpackTimeStamped(record_header_t& rHdr,ByteArray& rData,struct tm& rTime,unsigned long& rRun,String& rMsg) {
  // First clear the output data structures
  memset((char*)&rTime,'\0',sizeof(struct tm));
  rRun = 0;
  rMsg = "";

  if (rData.length != rHdr.data_size) {
    throw record_rtexception.format(CSTR("Record::unpackTimeStamped() Record data length does not match record data size %d != %d."),rData.length,rHdr.data_size);
  }

  if (rData.length == 0) return; // Nothing to do

  // First unpack the time stamp
  ubyte *p = rData.elements;
  int dsiz = rData.length; 
  memcpy((char*)&rTime,(char*)p,sizeof(rTime));
  p += sizeof(rTime);
  dsiz -= sizeof(rTime);

  // Decode the time stamp
  Record::decodeTime(rTime);

  // Get the run number  
  rRun = ntohl(*((uint32_t*)p));
  p += sizeof(uint32_t);
  dsiz -= sizeof(uint32_t);

  // Get the message
  if (dsiz > 1) rMsg.append((char*)p,dsiz-1);  // Append if containes more than a '\0'
}

/*===================================================================*/
/** @fn uint32_t Record::stringToPacketType(String& pcktstr)
* @brief Convert a string into a DSH packet type integer.
*                                        
* Convert a string into a DSH packet type integer.  That is, given
* a string representation of a packet type (e.g. "Physics"), lookup
* the integer representation and return it. 
*                                         
* @param pcktstr The packet type string.
* @return The integer packet type
* @throw RuntimeException If the string is not a known packet type.
*/      
uint32_t Record::stringToPacketType(String& pcktstr) { 
  int idx = 0;
  while (record_type_names[idx] != NULL) {
    String str(record_type_names[idx]);
    if (str.equalsIgnoreCase(pcktstr)) return record_type_ids[idx];
    idx++;
  } 

  throw record_rtexception.format(CSTR("Record::stringToPacketType() packet type \"%s\" is not recognized."),pcktstr.c_str());
}

/*===================================================================*/
/** @fn String Record::packetTypeToString(uint32_t pcktype)
* @brief Return the symbolic name for a packet type integer.
*                                        
* Return the symbolic name for a packet type integer.  
*                                         
* @param pcktype The packet type integer.
* @return The symbolic name for the packet type integer or the string "Unknown".
*/      
String Record::packetTypeToString(uint32_t pcktype) { 
  int idx = 0;
  String str; 

  while (record_type_names[idx] != NULL) {
    if (record_type_ids[idx] == pcktype) {
      str = record_type_names[idx];
      return str;
    }
    idx++;
  } 

  str = "Unknown";
  return str;
}

/*===================================================================*/
/** @fn bool Record::isReservedType(uint32_t pcktype)
* @brief Check if a packet type has been reserved.
*                                        
* Check if a packet type is reserved from use as a user
* specified packet type.
*                                         
* @param pcktype The packet type integer.
* @return True If the packet type is reserved and false otherwise.
*/      
bool Record::isReservedType(uint32_t pcktype) { 
  int idx = 0;

  // Iterate over the reserved list.
  while (record_type_reserved[idx] != ((uint32_t)-1)) {
    if (pcktype == record_type_reserved[idx]) return true;
    idx++;
  } 

  return false;
}

