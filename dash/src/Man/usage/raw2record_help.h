#ifndef RAW2RECORD_HELP_H
#define RAW2RECORD_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Convert raw data directly into DSH records.  This program is\n" \
  "not intended as an injection program, but as a primary source that\n" \
  "can consume raw data from stdin and write DSH records on stdout.\n" \
  "Data read from stdin is automagically packaged into well structured \n" \
  "records prior to being written to stdout.  The essential \n" \
  "characteristics of raw2record are as follows:\n" \
  "\n" \
  "1.  Raw2record first emits a BEGINRUN record on stdout.\n" \
  "\n" \
  "2.  Data read from stdin is treated as a stream of bytes.\n" \
  "\n" \
  "3.  Data read from stdin is automagically packaged into\n" \
  "    records and written to raw2record's stdout.\n" \
  "\n" \
  "4.  If raw2record's stdin is closed, or raw2record receives a\n" \
  "    SIGINT, raw2record will emit a ENDRUN record on stdout and \n" \
  "    then exit.\n" \
  "\n" \
  "Usage: raw2record --run=INTEGER --packet=INTEGER [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --run=INTEGER     Mandatory. Specify experiment run number\n" \
  "  --title=STRING    Experiment title string\n" \
  "  --size=INTEGER    Size of data in bytes to include in each record. \n" \
  "                        Default 8192 bytes.\n" \
  "\n" \
  "  --packet=INTEGER  Mandatory.  Emitted records will have a record \n" \
  "                    type of PACKET.  PACKET may be an integer or the \n" \
  "                    symbolic name for a well known packet type such \n" \
  "                    as Physics. \n" \
  "\n" \
  "Examples:\n" \
  "  raw2record --run=45 --packet=104 --title=\"Experiment number 45\" \n" \
  "         Produce records using run number 45 and title\n" \
  "         \"Experiment number 45.\"  Records produced from raw\n" \
  "         data read from stdin will have a record type of 104.\n" \
  "\n" \
  "  raw2record --size=1024 --run=51 --packet=Physics \n" \
  "             --title=\"Experiment number 51\" \n" \
  "         Produce records using run number 51 and title\n" \
  "         \"Experiment number 51.\"  Records produced from raw data \n" \
  "         read from stdin will have a record type of Physics.  Each\n" \
  "         record will contain 1024 bytes of raw data.\n";

#endif
