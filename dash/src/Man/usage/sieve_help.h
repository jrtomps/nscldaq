#ifndef SIEVE_HELP_H
#define SIEVE_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Output only those records from a DAQ experiment that have the \n" \
  "specified packet types.  That is, only those records that have\n" \
  "packet types corresponding with those specified using the\n" \
  "--packet flag will be written to standard out.  Records with type\n" \
  "BEGINRUN, ENDRUN, BADEND or CONTINUE are also written to standard\n" \
  "out to enable proper data stream parsing by consumers.  \n" \
  "All other records, are dropped.\n" \
  "\n" \
  "Packet types can be specified as either a single positive integer,\n" \
  "a range of positive integers or by using one of the known symbolic\n" \
  "names for some well known packet types.  Currently, the list\n" \
  "of symbolic well known packet types includes: Physics (a.k.a, Data),\n" \
  "Scaler, SnapSc, StateVar, RunVar, PktDoc and ParamDescript.\n" \
  "\n" \
  "Usage: sieve --packet=PACKETS [--packet=PACKETS]... [OPTION]... \n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --packet=PACKETS  Mandatory.  If more than one occurrence of this\n" \
  "                    flag appears on the command line, all packets and\n" \
  "                    ranges specified will be considered.  PACKETS can\n" \
  "                    be either a single positive integer, a range of\n" \
  "                    positive integers (e.g., 101-118 or 101,118) or\n" \
  "                    a symbolic name (e.g., Physics).  All ranges are\n" \
  "                    inclusive.\n" \
  "\n" \
  "  --invert          Invert the sense of record type matching.  That\n" \
  "                    is, rather than including only those record types\n" \
  "                    specified, only exclude those record types specified.\n" \
  "\n" \
  "Examples:\n" \
  "  Readout --run=123 | sieve --packet=101\n" \
  "         Output only those records that have packet type 101 to\n" \
  "         standard output.\n" \
  "\n" \
  "  Readout --run=890 | sieve --packet=105-116 | recorddump \n" \
  "         Pipe those records that have have a packet type falling in\n" \
  "         the range from 105 to 116 (inclusive) into the program \n" \
  "         recorddump.\n" \
  "\n" \
  "  Readout --run=567 | sieve --packet=Physics --packet=150 | recorddump \n" \
  "         Pipe those records that have have a packet type corresponding\n" \
  "         to the symbolic type of Physics and those records \n" \
  "         with packet type 150 into the program recorddump.\n";

#endif
