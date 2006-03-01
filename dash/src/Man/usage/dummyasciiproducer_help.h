#ifndef DUMMYASCIIPRODUCER_HELP_H
#define DUMMYASCIIPRODUCER_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Output random sized records from a dummy ASCII records for consumption\n" \
  "by a DAQ ASCII on-ramp.  This program produces printable ASCII \n" \
  "records separated by a separator character.  This program is intended \n" \
  "as an ASCII on-ramp test program.\n" \
  "\n" \
  "Usage: dummyasciiproducer [OPTION]...\n" \
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
  "  --iterations=INTEGER Specify the number of data records this\n" \
  "                    producer should emit prior to exiting.\n" \
  "\n" \
  "  --delay=MICROSECONDS Output occurs at a delay rate of MICROSECONDS\n" \
  "\n" \
  "  --separator=CHARCTER Specify that CHARCTER should be used as the\n" \
  "                    separator character.  Default is a new line\n" \
  "                    character.\n" \
  "\n" \
  "Examples:\n" \
  "  dummyproducer --run=45 | asciionramp --packet=104 dummyasciiproducer |\n" \
  "               segmenter /evt/data/output\n" \
  "          Inject output from dummybinaryoneramp into the data stream.  \n" \
  "          Records produced from data from dummyasciiproducer will have a type\n" \
  "          of 104.  If dummyasciiproducer terminates, it will not be restarted.\n" \
  "           \n" \
  "  dummyproducer --run=46 | asciionramp --packet=103 --restart\n" \
  "               dummyasciiproducer --iterations=10 --separator='\\f' | \n" \
  "               segmenter /evt/data/output\n" \
  "          Inject output from dummyasciiproducer into the data stream.  \n" \
  "          Records produced from data produced by dummyasciiproducer will \n" \
  "          have a type of 103.  Dummybinaryproducer will emit 10 records each\n" \
  "          time it is executed and then exit.  A formfeed character will\n" \
  "          be used as the separator character.  When dummyasciiproducer\n" \
  "          terminates, it is restarted by asciionramp.\n" \
  "\n";

#endif
