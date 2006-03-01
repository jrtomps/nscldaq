#ifndef SAMPLE_HELP_H
#define SAMPLE_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Selectively sample records from a DAQ experiment that have the \n" \
  "packet types specified using the --packet flag.  \n" \
  "That is, sample considers each record read from standard input.  \n" \
  "If the record's packet type matches one of the specified \n" \
  "packet types, then the record will only be written to standard \n" \
  "output if writing to standard output will not block.  All \n" \
  "other record types are unconditionally written to standard \n" \
  "output even if the write required to do so would block.  \n" \
  "\n" \
  "Packet types can be specified as either a single positive integer,\n" \
  "a range of positive integers or by using one of the known symbolic\n" \
  "names for some well known packet types.  Currently, the list\n" \
  "of symbolic well known packet types includes: Physics (a.k.a, Data),\n" \
  "Scaler, SnapSc, StateVar, RunVar, PktDoc and ParamDescript.\n" \
  "\n" \
  "Usage: sample --packet=PACKETS [--packet=PACKETS]... [OPTION]... \n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                    before beginning execution of main loop\n" \
  "\n" \
  "  --packet=PACKETS  Mandatory.  If more than one occurrence of this\n" \
  "                    flag appears on the command line, all packets and\n" \
  "                    ranges specified will be considered.  PACKETS can\n" \
  "                    be either a single positive integer, a range of\n" \
  "                    positive integers (e.g., 101-118 or 101,118) or\n" \
  "                    a symbolic name (e.g., Physics).  All ranges are\n" \
  "                    inclusive.\n" \
  "\n" \
  "Examples:\n" \
  "  Readout --run=123 | sample --packet=101\n" \
  "         Output those records that have packet type 101 only if\n" \
  "         writing to standard output will not block.  All other\n" \
  "         records are written regardless of blocking.\n" \
  "\n" \
  "  Readout --run=890 | sample --packet=105-116 | recorddump \n" \
  "         Pipe those records that have have a packet type falling in\n" \
  "         the range from 105 to 116 (inclusive) into the program \n" \
  "         recorddump only if writing to standard output will not\n" \
  "         block.  All other records are written regardless of\n" \
  "         blocking.\n" \
  "\n" \
  "  Readout --run=567 | sample --packet=Physics --packet=150 | recorddump \n" \
  "         Pipe those records that have have a packet type corresponding\n" \
  "         to the symbolic type of Physics and those records \n" \
  "         with packet type 150 into the program recorddump only if\n" \
  "         writing to standard output will not block.  All other records\n" \
  "         are written regardless of blocking.\n";

#endif
