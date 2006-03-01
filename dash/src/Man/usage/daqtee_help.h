#ifndef DAQTEE_HELP_H
#define DAQTEE_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Daqtee writes records from a DAQ experiment to two separate \n" \
  "programs enabling online filtering and data processing while\n" \
  "retaining the unfiltered or processed data.  Records written\n" \
  "to the secondary (tee) data stream can be selected a priori\n" \
  "using the --packet flag.  That is, daqtee considers each record \n" \
  "read from standard input.  If the record's packet type matches \n" \
  "one of the specified packet types, then the record will be written \n" \
  "to the tee data stream.  All records read from stdin are \n" \
  "unconditionally written to the main data stream on stdout.  \n" \
  "If the --packet flag is not specified, then all records read from \n" \
  "stdin are also written to the tee data stream.\n" \
  "\n" \
  "Packet types can be specified as either a single positive integer,\n" \
  "a range of positive integers or by using one of the known symbolic\n" \
  "names for some well known packet types.  Currently, the list\n" \
  "of symbolic well known packet types includes: Physics (a.k.a, Data),\n" \
  "Scaler, SnapSc, StateVar, RunVar, PktDoc and ParamDescript.\n" \
  "\n" \
  "The essential characteristics of daqtee are as follows:\n" \
  "\n" \
  "1. daqtee passes all records read on stdin to stdout.\n" \
  "\n" \
  "2. daqtee runs a client program with stdin connected to a pipe \n" \
  "   to which it write a copy of the records specified using the\n" \
  "   --packet flag.  If, the --packet flag is not specified then\n" \
  "   a copy of all the records received on stdin will be written\n" \
  "   to the pipe.\n" \
  "\n" \
  "3. If PROGRAM breaks the pipe, daqtee degenerates to writing\n" \
  "   all records read on stdin to stdout.\n" \
  "\n" \
  "4. If an eof is received on stdin, daqtee exits.\n" \
  "\n" \
  "Usage: daqtee [--packet=PACKETS] ... [OPTION]... PROGRAM\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                    before beginning execution of main loop\n" \
  "\n" \
  "  --packet=PACKETS  If more than one occurrence of this flag appears \n" \
  "                    on the command line, all packets and ranges \n" \
  "                    specified will be considered.  PACKETS can\n" \
  "                    be either a single positive integer, a range of\n" \
  "                    positive integers (e.g., 101-118 or 101,118) or\n" \
  "                    a symbolic name (e.g., Physics).  All ranges are\n" \
  "                    inclusive.\n" \
  "\n" \
  "Examples:\n" \
  "  Readout --run=123 | daqtee --packet=101 myfilter | segmenter\n" \
  "         Output all records read on stdin to stdout and write\n" \
  "         a copy of records that have a packet type of 101 to\n" \
  "         myfilter.\n" \
  "\n" \
  "  Readout --run=890 | daqtee --packet=PktDoc recorddump | segmenter\n" \
  "         Output all records read on stdin to stdout and write\n" \
  "         a copy of records that have a packet type of PktDoc to\n" \
  "         recorddump.\n" \
  "\n";

#endif
