#ifndef RATEMETER_HELP_H
#define RATEMETER_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "\n" \
  "Reads a DAQ record stream from standard in and either consume the\n" \
  "record or pass it through to standard out.  Calculate rate\n" \
  "statistics and print them to standard error.  The size of the \n" \
  "record header is not included in when calculating statistics.\n" \
  "\n" \
  "Usage: ratemeter [--consume] [--every=INTEGER]... [OPTION]... \n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --every=INTEGER   An attempt will be made to output statistics to \n" \
  "                    standard error every INTEGER seconds of wall clock \n" \
  "                    time.  That is, statistics will output after the\n" \
  "                    specified number of seconds have passed and a \n" \
  "                    new record has been read.  Default is 10 seconds.\n" \
  "\n" \
  "  --consume         Records will be consumed rather written to\n" \
  "                    standard out. \n" \
  " \n" \
  "Examples:\n" \
  "  Readout --run=123 | ratemeter --every=10 | recorddump\n" \
  "         Calculate statistics for run 123, outputting statistics\n" \
  "         on standard error every 10 seconds.  Records\n" \
  "         are then written to standard output for consumption by\n" \
  "         recorddump.\n" \
  "\n" \
  "  Readout --run=890 | ratemeter --consume --every=5\n" \
  "         Calculate statistics for run 890, outputting statistics\n" \
  "         on standard error every 5 seconds.  Records\n" \
  "         are then consumed.\n" \
  "\n";

#endif
