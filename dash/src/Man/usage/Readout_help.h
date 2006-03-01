#ifndef READOUT_HELP_H
#define READOUT_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Read experiment input from DAQ hardware and output event records. \n" \
  "Each execution of Readout begins with a BEGINRUN record and\n" \
  "ends with either an ENDRUN or BADEND record depending on how\n" \
  "Readout was terminated.  If Readout is terminated\n" \
  "with a SIGINT (Ctrl-C), then Readout emits an\n" \
  "ENDRUN record.  If Readout is terminated by a SIGTERM, \n" \
  "then Readout will emit a BADEND record prior to exiting.\n" \
  "All other records are of type PHYSICS.\n" \
  "\n" \
  "Usage: Readout --run=INTEGER [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --run=INTEGER     Mandatory. Specify experiment run number\n" \
  "  --title=STRING     Experiment title string\n" \
  "\n" \
  "Examples:\n" \
  "  Readout --run=45 --title=\"Experiment number 45\" \n" \
  "         Produce records using run number 45 and title\n" \
  "         \"Experiment number 45.\"\n" \
  "\n" \
  "  Readout --run=123 --title=\"A cool experiment\" | recorddump \n" \
  "         Produce records using run number 123 and title\n" \
  "         \"A cool experiment\" and pipe them into program recorddump.\n";

#endif
