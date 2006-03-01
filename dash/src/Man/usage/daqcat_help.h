#ifndef DAQCAT_HELP_H
#define DAQCAT_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Output the records from a DAQ experiment that has previously been\n" \
  "saved to disk.  DAQ experimental data is stored in segmented \n" \
  "files to avoid exceeding the maximum file size of the filesystem.\n" \
  "To simplify processing experimental data, daqcat understands how\n" \
  "to output the records for a particular run starting at the first\n" \
  "segment and terminating at the last segment.\n" \
  "\n" \
  "Usage: daqcat --run=INTEGER [OPTION]... DIRECTORY\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --run=INTEGER     Mandatory. Specify experiment run number\n" \
  "\n" \
  "Examples:\n" \
  "  daqcat --run=45 /evt/data/output  \n" \
  "         Cat experiment with run number 45 found in directory \n" \
  "         /evt/data/output\n" \
  "\n" \
  "  daqcat --run=123 /evt/data/output | recorddump \n" \
  "         Cat experiment with run number 123 found in directory \n" \
  "         /evt/data/output into program recorddump.\n";

#endif
