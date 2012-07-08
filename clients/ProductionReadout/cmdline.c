/*
  File autogenerated by gengetopt version 2.5  
  generated with the following command:
  gengetopt 

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Check for configure's getopt check result.  */
#ifndef HAVE_GETOPT_LONG
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include "cmdline.h"


void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n", PACKAGE, VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n"
"Purpose:\n"
"  Reads out experimental data to spectrodaq\n"
"\n"
"Usage: %s [OPTIONS]...\n", PACKAGE);
  printf("   -h      --help            Print help and exit\n");
  printf("   -V      --version         Print version and exit\n");
  printf("   -pINT   --port=INT        Enable tcl server functionality, next parameter is the port\n");
  printf("   -w      --window          Use Tk interpreter if set (default=off)\n");
  printf("   -c      --camac-trigger   Use  CAMAC triggers instead of VME (default=off)\n");
}


int
cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info)
{
  int c;	/* Character of the parsed option.  */
  int missing_required_options = 0;	

  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->port_given = 0 ;
  args_info->window_given = 0 ;
  args_info->camac_trigger_given = 0 ;
#define clear_args() { \
  args_info->window_flag = 0;\
  args_info->camac_trigger_flag = 0;\
}

  clear_args();

  optarg = 0;
  optind = 1;
  opterr = 1;
  optopt = '?';

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "port",	1, NULL, 'p' },
        { "window",	0, NULL, 'w' },
        { "camac-trigger",	0, NULL, 'c' },
        { NULL,	0, NULL, 0 }
      };

      c = getopt_long (argc, argv, "hVp:wc", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          clear_args ();
          cmdline_parser_print_help ();
          exit (0);

        case 'V':	/* Print version and exit.  */
          clear_args ();
          cmdline_parser_print_version ();
          exit (0);

        case 'p':	/* Enable tcl server functionality, next parameter is the port.  */
          if (args_info->port_given)
            {
              fprintf (stderr, "%s: `--port' (`-p') option given more than once\n", PACKAGE);
              clear_args ();
              exit (1);
            }
          args_info->port_given = 1;
          args_info->port_arg = atoi (optarg);
          break;

        case 'w':	/* Use Tk interpreter if set.  */
          if (args_info->window_given)
            {
              fprintf (stderr, "%s: `--window' (`-w') option given more than once\n", PACKAGE);
              clear_args ();
              exit (1);
            }
          args_info->window_given = 1;
          args_info->window_flag = !(args_info->window_flag);
          break;

        case 'c':	/* Use  CAMAC triggers instead of VME.  */
          if (args_info->camac_trigger_given)
            {
              fprintf (stderr, "%s: `--camac-trigger' (`-c') option given more than once\n", PACKAGE);
              clear_args ();
              exit (1);
            }
          args_info->camac_trigger_given = 1;
          args_info->camac_trigger_flag = !(args_info->camac_trigger_flag);
          break;

        case 0:	/* Long option with no short option */

        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          exit (1);

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c\n", PACKAGE, c);
          abort ();
        } /* switch */
    } /* while */

  if ( missing_required_options )
    exit (1);


  return 0;
}