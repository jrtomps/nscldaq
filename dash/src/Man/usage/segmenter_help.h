#ifndef SEGMENTER_HELP_H
#define SEGMENTER_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Segment a DAQ pipeline record stream into segments for storage on\n" \
  "disk.  The program does the following:\n" \
  "\n" \
  "1. Monitors stdin for records.\n" \
  "\n" \
  "2. Upon receipt of a BEGINRUN record, segmenter extracts the run \n" \
  "number and creates a file named run{run_number}_0000.evt.\n" \
  "\n" \
  "3. The BEGINRUN record is then written to this file.\n" \
  "\n" \
  "4. Upon receipt of an ENDRUN record, segmenter writes the ENDRUN \n" \
  "record to file, closes the file and exits.\n" \
  "\n" \
  "5. In response to a broken pipe, segmenter creates a BADEND record, \n" \
  "signifying that the run has ended, writes this record to file and exits.\n" \
  "\n" \
  "6. All other records are written to file without interpretation. \n" \
  "\n" \
  "7. If any write would create a file larger than the current segsize, \n" \
  "segmenter closes the currently open file, increment the segment number \n" \
  "and create a new file named run{run_number}_{sequence}.evt and \n" \
  "continues writing records.  \n" \
  "\n" \
  "Usage: segmenter [OPTION]... DIRECTORY\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --segsize=MEGABYTES Specify run file segment size.  Default is 2GB. \n" \
  "\n" \
  "Examples:\n" \
  "  Readout --run=56 | segmenter /evt/data/output  \n" \
  "         Use Readout to produce records for run number 58 and pipe \n" \
  "         them through segmenter.  Segmenter will write run file\n" \
  "         segments in directory /evt/data/output.\n" \
  "\n" \
  "  Readout --run=123 | segmenter --segsize=5 /evt/data/output \n" \
  "         Use Readout to produce records for run number 128 and pipe \n" \
  "         them through segmenter.  Segmenter will write 5MB run file\n" \
  "         segments in directory /evt/data/output.\n";

#endif
