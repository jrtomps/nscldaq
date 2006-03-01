#ifndef DAQTAIL_HELP_H
#define DAQTAIL_HELP_H


// Do not edit help_string.  Automatically generated usagedata
static const char *help_string = "" \
  "Output the records from a DAQ experiment that has previously been\n" \
  "saved to disk.  DAQ experimental data is stored in segmented \n" \
  "files to avoid exceeding the maximum file size of the filesystem.\n" \
  "To simplify processing experimental data, daqtail understands how\n" \
  "to output the records for the current experiment as the are \n" \
  "appended to the current segment.  \n" \
  "\n" \
  "Creation of new runs and new run segments in the specified directory\n" \
  "is understood by daqtail.  When new runs are added to the directory,\n" \
  "daqtail will detect that a new run has been stared and automatically\n" \
  "resume processing with this new run.  When new segments are created\n" \
  "for the current run, daqtail will detect that a new segment has been\n" \
  "created and continue processing with this new segment.\n" \
  "\n" \
  "Unless the --catchup option has been specified, processing begins\n" \
  "by outputting the last record of the most recent segment.\n" \
  "\n" \
  "Usage: daqtail [OPTION]... DIRECTORY\n" \
  "\n" \
  "Options:\n" \
  "  --version         Output version information and exit     \n" \
  "  --help            Display this help and exit\n" \
  "  --debug=SECONDS   Output a debugging command and wait SECONDS\n" \
  "                        before beginning execution of main loop\n" \
  "\n" \
  "  --catchup         Start processing with the first record\n" \
  "                    of the first segment of the current run.  \n" \
  "\n" \
  "  --collate=[ctime|runnumber] (default: ctime)\n" \
  "                    Collate the event files found in DIRECTORY\n" \
  "                    according to either file ctime (inode change time)\n" \
  "                    or by runnumber.  \n" \
  "\n" \
  "  --file            Treat DIRECTORY as a file.  The file specified will\n" \
  "                    be tailed until an ENDRUN, BADEND or CONTINUE \n" \
  "                    record is encountered.  Note that --collate\n" \
  "                    has no effect when tailing a specific file.   If\n" \
  "                    --catchup is specified, processing begins at the\n" \
  "                    start, instead of the end, of the specified file.\n" \
  "                    \n" \
  "Examples:\n" \
  "  daqtail /evt/data/output  \n" \
  "         Tail most recent experiment found in directory /evt/data/output\n" \
  "\n" \
  "  daqtail --catchup /evt/data/output | recorddump \n" \
  "         Tail most recent experiment found in directory \n" \
  "         /evt/data/output into program recorddump.\n" \
  "         Processing begins with the first segment of the most \n" \
  "         recent run.\n" \
  "\n" \
  "  daqtail --collate=runnumber /evt/data/output  \n" \
  "         tail most recent experiment found in directory /evt/data/output.\n" \
  "         Collate using run numbers instead of ctime. \n" \
  "\n" \
  "  daqtail --file experiment.data\n" \
  "         tail the file experiment.data and exit when an ENDRUN, BADEND or\n" \
  "         CONTINUE record is encountered.\n" \
  "\n";

#endif
