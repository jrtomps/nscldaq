#ifndef BINARY2RECORD_HELP_H
#define BINARY2RECORD_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Convert binary data directly into DSH records.  This program is\n" \
  "not intended as an injection program, but as a primary source that\n" \
  "can consume binary data from stdin and write DSH records on stdout.\n" \
  "Data read from stdin is automagically packaged into well structured \n" \
  "records prior to being written to stdout.  The essential \n" \
  "characteristics of binary2record are as follows:\n" \
  "\n" \
  "1.  Binary2record first emits a BEGINRUN record on stdout.\n" \
  "\n" \
  "2.  Data read from stdin is expected to have a structure\n" \
  "    that includes a leading byte count, indicating the number of data\n" \
  "    bytes produced, followed by the data.  The leading byte count\n" \
  "    should be a 4 byte integer in host byte order.\n" \
  "\n" \
  "3.  Data read from stdin is automagically packaged into\n" \
  "    records and written to binary2record's stdout.\n" \
  "\n" \
  "4.  If binary2record's stdin is closed, or binary2record receives a\n" \
  "    SIGINT, binary2record will emit a ENDRUN record on stdout and \n" \
  "    then exit.\n" \
  "\n" \
  "Usage: binary2record --run=INTEGER --packet=INTEGER [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --run=INTEGER     Mandatory. Specify experiment run number\n" \
  "  --title=STRING    Experiment title string\n" \
  "  --packet=[PACKET|typed] Mandatory. Record type to use for records produced.\n" \
  "                    PACKET may be an integer or the symbolic name for\n" \
  "                    a well known packet type such as Physics.  If the\n" \
  "                    argument is the string \"typed,\" then the binary records\n" \
  "                    read are assumed to have an host-byte-order integer\n" \
  "                    field, indicating the record type, following the length.\n" \
  "\n" \
  "Examples:\n" \
  "  binary2record --run=45 --packet=104 --title=\"Experiment number 45\" \n" \
  "         Produce records using run number 45 and title\n" \
  "         \"Experiment number 45.\"  Records produced from binary\n" \
  "         data read from stdin will have a record type of 104.\n";

#endif
