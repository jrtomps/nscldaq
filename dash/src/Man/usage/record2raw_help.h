#ifndef RECORD2RAW_HELP_H
#define RECORD2RAW_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Convert DSH records directory into raw data.  Raw data is simply\n" \
  "a stream of data bytes about which no structure is assumed. \n" \
  "Control records, such as BEGINRUN, ENDRUN, BADEND and CONTINUE,\n" \
  "are consumed and not passed to stdout as part of the raw data\n" \
  "stream.\n" \
  "\n" \
  "Usage: record2raw [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "Examples:\n" \
  "  daqcat --run=45 /evt/data/output | record2raw \n" \
  "         Produce a raw data stream from the event data for run 45\n" \
  "         stored in directory /evt/data/output.  Conversion continues\n" \
  "         until record2raw receives an eof on stdin.\n" \
  "\n" \
  "  daqcat --run=45 /evt/data/output | sieve --packet=Physics | record2raw \n" \
  "         Produce a raw data stream from the event data for run 45\n" \
  "         stored in directory /evt/data/output.  Sieve restricts the \n" \
  "         type of records fed to record2raw to be of type Physics.\n" \
  "         Conversion continues until record2raw receives an eof on stdin.\n" \
  "\n";

#endif
