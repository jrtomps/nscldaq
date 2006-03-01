#ifndef DUMMYBINARYPRODUCER_HELP_H
#define DUMMYBINARYPRODUCER_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Output random sized records from a dummy binary records for consumption\n" \
  "by a DAQ on-ramp. This program produces records with a leading length \n" \
  "field, written as an integer in host byte order, followed by data bytes.  \n" \
  "The length of the data is indicated by the leading length field; the size\n" \
  "of the length field is not included in the data length.  This program\n" \
  "is intended as a binary on-ramp test program.\n" \
  "\n" \
  "Usage: dummybinaryproducer [OPTION]...\n" \
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
  "  --packet=PACKET   If this parameter is specified, then the binary record \n" \
  "                    records emitted are typed binary records.  That is\n" \
  "                    the emitted records will have an host-byte-order integer\n" \
  "                    field, indicating the record type, following the length.\n" \
  "                    This field will be set to PACKET.  PACKET may be an \n" \
  "                    integer or the symbolic name for a well known packet \n" \
  "                    type such as Physics.  \n" \
  "\n" \
  "Examples:\n" \
  "  dummyproducer --run=45 | binaryonramp --packet=104 dummybinaryproducer |\n" \
  "               segmenter /evt/data/output\n" \
  "          Inject output from dummybinaryoneramp into the data stream.  \n" \
  "          Records produced from data from dummybinaryproducer will have a type\n" \
  "          of 104.  If dummybinaryproducer terminates, it will not be restarted.\n" \
  "           \n" \
  "  dummyproducer --run=46 | binaryonramp --packet=103 --restart \n" \
  "               dummybinaryproducer --iterations=10 | segmenter /evt/data/output\n" \
  "          Inject output from dummybinaryproducer into the data stream.  \n" \
  "          Records produced from data produced by dummybinaryproducer will \n" \
  "          have a type of 103.  Dummybinaryproducer will emit 10 records each\n" \
  "          time it is executed and then exit.  When dummybinaryproducer\n" \
  "          terminates, it is restarted by binaryonramp.\n" \
  "\n";

#endif
