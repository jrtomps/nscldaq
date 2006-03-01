#ifndef ASCIIONRAMP_HELP_H
#define ASCIIONRAMP_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Enable the injection of ASCII data into the data stream.  The\n" \
  "asciionramp program enables ASCII output from USERCOMMAND\n" \
  "(scripts or other programs) to be easily injected into the data \n" \
  "stream.  The output of USERCOMMAND is automagically packaged into\n" \
  "well structured records prior to being injected into the data stream.\n" \
  "The essential characteristics of asciionramp are as follows:\n" \
  "\n" \
  "1.  Records appearing on asciionramp's stdin are transfered to\n" \
  "    stdout.\n" \
  "\n" \
  "2.  Once a BEGINRUN record has been read from stdin, the BEGINRUN\n" \
  "    record is written to stdout and USERCOMMAND executed.  \n" \
  "    USERCOMMAND's stdout is connected to a pipe, enabling asciionramp\n" \
  "    to read data produced by USERCOMMAND.  USERCOMMAND's stderr is\n" \
  "    also connected to a pipe, enabling asciionramp to read error\n" \
  "    messages produced by USERCOMMAND and relay them to asciionramp's\n" \
  "    stderr.\n" \
  "\n" \
  "3.  Data produced by USERCOMMAND is automagically packaged into\n" \
  "    records and written to asciionramp's stdout.\n" \
  "\n" \
  "4.  If USERCOMMAND exits and --restart is not specified, asciionramp\n" \
  "    continues reading records from stdin and writing them to stdout.\n" \
  "\n" \
  "5.  If asciionramp's stdin is closed, asciionramp kills USERCOMMAND\n" \
  "    (if necessary) and then exits.\n" \
  "\n" \
  "Usage: asciionramp --packet=PACKET [OPTION]... USERCOMMAND\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --oneliner        Produce a new record for each line of output\n" \
  "                    produced by USERCOMMAND and write it to stdout.\n" \
  "\n" \
  "  --separator=CHARACTER  Collect output from USERCOMMAND until CHARACTER\n" \
  "                    is received.  The collected output, excluding the\n" \
  "                    separator, is packaged into a single record and\n" \
  "                    written to stdout. \n" \
  "\n" \
  "  --restart[=RETRIES] (default: infinite restarts)\n" \
  "                    Restart USERCOMMAND each time it exits.  If RETRIES\n" \
  "                    is specified, only restart the program RETRIES\n" \
  "                    times.  \n" \
  "\n" \
  "  --packet=PACKET   Mandatory.  PACKET can be either a single positive \n" \
  "                    integer or a symbolic name (e.g., Physics).  \n" \
  "                    Records produced from the output of USERCOMMAND will\n" \
  "                    be assigned a type of PACKET.\n" \
  "                    \n" \
  "Examples:\n" \
  "  Readout --run=45 | asciionramp --packet=104 --oneliner timestamp.sh |\n" \
  "               segmenter /evt/data/output\n" \
  "          Inject a time stamp into the data stream.  Each line output by\n" \
  "          timestamp.sh is packaged into a record and written to stdout.\n" \
  "\n" \
  "  Readout --run=46 | asciionramp --packet=103 --restart periodic.sh |\n" \
  "               segmenter /evt/data/output\n" \
  "          Inject periodically produced data into the data stream.  \n" \
  "          The entire output of periodic.sh is packaged as a single\n" \
  "          record and written to stdout.  When periodic.sh terminates,\n" \
  "          it is restarted.\n" \
  "\n" \
  "  Readout --run=47 | asciionramp --packet=105 --separator=\"\\f\" \n" \
  "               --restart thresholds.sh | segmenter /evt/data/output\n" \
  "          Collect output from thresholds.sh until a form feed character\n" \
  "          is received.  The collected output, excluding the form \n" \
  "          feed character, is packaged into a single record and \n" \
  "          written to stdout. When thresholds.sh terminates,\n" \
  "          it is restarted.\n" \
  "\n";

#endif
