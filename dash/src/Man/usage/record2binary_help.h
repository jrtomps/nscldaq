#ifndef RECORD2BINARY_HELP_H
#define RECORD2BINARY_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Convert DSH records directory into binary data.  Binary data is\n" \
  "data stream consisting of records with a leading length field, \n" \
  "written as an integer in host byte order, followed by data bytes.\n" \
  "Control records, such as BEGINRUN, ENDRUN, BADEND and CONTINUE,\n" \
  "are consumed and not passed to stdout as part of the raw data\n" \
  "stream.\n" \
  "\n" \
  "Usage: record2binary [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --typed           Output typed binary data.  That is, add a \n" \
  "                    host-byte-order integer field that indicates the \n" \
  "                    record type following the length field.\n" \
  "\n" \
  "Examples:\n" \
  "  daqcat --run=45 /evt/data/output | record2binary \n" \
  "         Produce a binary data stream from the event data for run 45\n" \
  "         stored in directory /evt/data/output.  Conversion continues\n" \
  "         until record2binary receives an eof on stdin.\n" \
  "\n" \
  "  daqcat --run=45 /evt/data/output | sieve --packet=Physics | record2binary \n" \
  "         Produce a binary data stream from the event data for run 45\n" \
  "         stored in directory /evt/data/output.  Sieve restricts the \n" \
  "         type of records fed to record2binary to be of type Physics.\n" \
  "         Conversion continues until record2binary receives an eof on stdin.\n" \
  "\n";

#endif
