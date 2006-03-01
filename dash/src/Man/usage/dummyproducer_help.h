#ifndef DUMMYPRODUCER_HELP_H
#define DUMMYPRODUCER_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Output random sized records from a dummy DAQ experiment.  Each\n" \
  "execution of dummyproducer begins with a BEGINRUN record and\n" \
  "ends with either an ENDRUN or BADEND record depending on how\n" \
  "the producer was terminated.  If dummyproducer is terminated\n" \
  "with a SIGINT (Ctrl-C), then dummyproducer emits an\n" \
  "ENDRUN record.  If dummyproducer is terminated by a SIGTERM, \n" \
  "then dummyproducer will emit a BADEND record prior to exiting.\n" \
  "All other records are of type PHYSICS.\n" \
  "\n" \
  "Usage: dummyproducer --run=INTEGER [OPTION]...\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --size=INTEGER    Specify a fixed record size.  The default\n" \
  "                    is to generate random sized records up to a\n" \
  "                    maximum of 64K.\n" \
  "\n" \
  "  --run=INTEGER     Mandatory. Specify experiment run number\n" \
  "  --delay=MICROSECONDS Output occurs at a delay rate of MICROSECONDS\n" \
  "  --title=STRING     Experiment title string\n" \
  "\n" \
  "  --packet=PACKET   Emitted records will have a record type of PACKET.\n" \
  "                    PACKET may be an integer or the symbolic name for \n" \
  "                    a well known packet type such as Physics.  If this\n" \
  "                    parameter is not specified, the default record\n" \
  "                    type is Physics.\n" \
  "\n" \
  "Examples:\n" \
  "  dummyproducer --run=45 --title=\"Experiment number 45\" \n" \
  "         Produce records using run number 45 and title\n" \
  "         \"Experiment number 45.\"\n" \
  "\n" \
  "  dummyproducer --run=123 --delay=1000000 | recorddump \n" \
  "         Produce records using run number 123 once per second\n" \
  "         and pipe them into program recorddump.\n";

#endif
