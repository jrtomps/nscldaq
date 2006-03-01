#ifndef RECORDDUMP_HELP_H
#define RECORDDUMP_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Reads a DAQ record stream from stdin and outputs the \n" \
  "record headers in human readable format to stderr.\n" \
  "\n" \
  "Usage: recorddump [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "  --dump=NBYTES     Dump up to NBYTES of record data in hex format\n" \
  "  --print           Also dump data as printable characters if possible.\n" \
  "\n" \
  "Examples:\n" \
  "  daqtail --run=123 /evt/data/output | recorddump \n" \
  "         Use daqcat to output the records of run 123 found in\n" \
  "         directory /evt/data/output and pipe them into recorddump.\n" \
  "\n" \
  "  cat /evt/data/output/run0123_0001.evt | recorddump \n" \
  "         Cat the first data file segment of run 123 into \n" \
  "         recorddump.\n";

#endif
