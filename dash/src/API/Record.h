#ifndef DAQHWYAPI_RECORD_H
#define DAQHWYAPI_RECORD_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#include <time.h>
#include <netinet/in.h>

#ifndef DAQHWYAPI_MAINDEFS_H
#include <dshapi/maindefs.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

namespace daqhwyapi {

class String;
class ByteArray;

/*=====================================================================*/
/**
* @struct record_header_t
* @brief Basic record header type definition.
*
* Basic record header type definition.
*
* @author  Eric Kasten
* @version 1.0.0
*/
typedef struct record_header_struct {
  uint16_t version;
  uint32_t record_size;
  uint32_t record_type;
  uint16_t status_code;
  uint32_t byte_order;
  uint32_t extended_header_size; 
  uint32_t data_size;
  uint32_t entity_count;
  // ---------------------------------------------
  // The following two fields follow the static
  // sized header.  The extended_header has byte 
  // length extended_header_size and data has byte
  // length data_size.
  // 
  // ubyte *extended_header;
  // ubyte *data;
} record_header_t;

/*=====================================================================*/
/**
* @class Record
* @brief Record mainpulation library.
*
* Static methods for mainpulating records and record headers.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Record : public Object {
  public:
    static uint16_t current_version;
    static int default_max_record_size;
    static int encode_buffer_size; // Minimum size of the encode buffer

    // Record types
    static uint32_t type_unknown;
    static uint32_t type_beginrun;
    static uint32_t type_continue;
    static uint32_t type_endrun;
    static uint32_t type_badend;
    static uint32_t type_physics;

    static uint32_t type_data;
    static uint32_t type_scaler;
    static uint32_t type_snapsc;
    static uint32_t type_statevar;
    static uint32_t type_runvar;
    static uint32_t type_pktdoc;
    static uint32_t type_pause;
    static uint32_t type_resume;
    static uint32_t type_paramdescript;
    static uint32_t type_stats;

    // Maximum system type. User packet types must have
    // a value greater than the maximum reserved system types.
    static uint32_t type_maxsystem;
    
    // Status codes
    static uint16_t status_ok;     // Record Ok
    static uint16_t status_err;    // Unspecified record error
    static uint16_t status_trunc;  // Record truncated
    static uint16_t status_runshort; // Run incomplete

    static void initHeader(record_header_t&);
    static int encodeHeader(ubyte*,int,record_header_t&);
    static int decodeHeader(ubyte*,int,record_header_t&);
    static void copyHeader(record_header_t&,record_header_t&);

    static void encodeTime(struct tm&);
    static void decodeTime(struct tm&);
    static void packTimeStamped(record_header_t&,ByteArray&,unsigned int,unsigned long,String&);
    static void unpackTimeStamped(record_header_t&,ByteArray&,struct tm&,unsigned long&,String&);

    static uint32_t stringToPacketType(String&);
    static String packetTypeToString(uint32_t);
    static bool isReservedType(uint32_t);

  protected:
    Record();   // Constructor 
};

} // namespace daqhwyapi

#endif
