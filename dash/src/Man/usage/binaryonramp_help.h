#ifndef BINARYONRAMP_HELP_H
#define BINARYONRAMP_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Enable the injection of binary data into the data stream.  The\n" \
  "binaryonramp program enables binary output from USERBINARY\n" \
  "(executable programs) to be easily injected into the data \n" \
  "stream.  The output of USERBINARY is automagically packaged into\n" \
  "well structured records prior to being injected into the data stream.\n" \
  "The essential characteristics of binaryonramp are as follows:\n" \
  "\n" \
  "1.  Records appearing on binaryonramp's stdin are transfered to\n" \
  "    stdout.\n" \
  "\n" \
  "2.  Once a BEGINRUN record has been read from stdin, the BEGINRUN\n" \
  "    record is written to stdout and USERBINARY executed.  \n" \
  "    USERBINARY's stdout is connected to a pipe, enabling binaryonramp\n" \
  "    to read data produced by USERBINARY.  USERBINARY's stderr is\n" \
  "    also connected to a pipe, enabling binaryonramp to read error\n" \
  "    messages produced by USERBINARY and relay them to binaryonramp's\n" \
  "    stderr.\n" \
  "\n" \
  "3.  Data produced by USERBINARY is expected to produce output \n" \
  "    that includes a leading byte count, indicating the number of data\n" \
  "    bytes produced, followed by the data.  The leading byte count\n" \
  "    should be a 4 byte integer in host byte order.\n" \
  "\n" \
  "4.  Data produced by USERBINARY is automagically packaged into\n" \
  "    records and written to binaryonramp's stdout.\n" \
  "\n" \
  "5.  If USERBINARY exits and --restart is not specified, binaryonramp\n" \
  "    continues reading records from stdin and writing them to stdout.\n" \
  "\n" \
  "6.  If binaryonramp's stdin is closed, binaryonramp kills USERBINARY\n" \
  "    (if necessary) and then exits.\n" \
  "\n" \
  "Usage: binaryonramp --packet=PACKET [OPTION]... USERBINARY\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --restart[=RETRIES] (default: infinite restarts)\n" \
  "                    Restart USERBINARY each time it exits.  If RETRIES\n" \
  "                    is specified, only restart the program RETRIES\n" \
  "                    times.  \n" \
  "\n" \
  "  --packet=[PACKET|typed]  Mandatory.  PACKET can be either a single positive \n" \
  "                    integer or a symbolic name (e.g., Physics).  \n" \
  "                    Records produced from the output of USERBINARY will\n" \
  "                    be assigned a type of PACKET.  If the argument is \n" \
  "                    the string \"typed,\" then the output of USERBINARY is\n" \
  "                    assumed to have an host-byte-order integer field, \n" \
  "                    indicating the record type, following the length.\n" \
  "                    \n" \
  "Examples:\n" \
  "  Readout --run=45 | binaryonramp --packet=104 mybinary |\n" \
  "               segmenter /evt/data/output\n" \
  "          Inject output from mybinary into the data stream.  \n" \
  "          Records produced from data from mybinary will have a type\n" \
  "          of 104.  If mybinary terminates, it will not be restarted.\n" \
  "           \n" \
  "  Readout --run=46 | binaryonramp --packet=103 --restart mybinary |\n" \
  "               segmenter /evt/data/output\n" \
  "          Inject output from mybinary into the data stream.  \n" \
  "          Records produced from data from mybinary will have a type\n" \
  "          of 103.  When mybinary terminates, it is restarted.\n" \
  "\n";

#endif
