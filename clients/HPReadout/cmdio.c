
/*
**++
**  FACILITY:
**
**      Data acquisition system front end, master processor.
**
**  ABSTRACT:
**
**      This module contains functions which are used to input and manipulate
**	strings which might be command lines. Some of the functions contained
**	are:
**	    gtcmd	- Get a command with prompt.
**	    prompt	- Issue a prompt to stdout.
**	    getline	- Get line of text from stdin.
**	    upcase	- Convert a string to upper case
**	    strtoken	- Parse a delimited token from a string.
**
**  AUTHORS:
**  DATE:     14-Oct-1987
**  Ported to Linux:
**          February 16, 1999 R. Fox
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include    <ctype.h>
#include    <time.h>
#include    <stdio.h>
#include    "daqdatatypes.h"
#include    "cmdio.h"


#ifdef __STDC__
#undef toupper
#define toupper(ch) _toupper(ch)
#endif


/*	Static definitions */
static char* pCopyright = 
"cmdio.c: (c) Copyright NSCL 1999, All rights reserved\n";

/*		Month Lookup table		*/

static const char *montbl[12] = { "JAN", "FEB", "MAR", "APR", "MAY",
			          "JUN", "JUL", "AUG", "SEP", "OCT",
			          "NOV", "DEC" };

/*		Keywords with TRUE value	*/

#define    NONS    3
#define    ONSIZE    10
static  const  char    *ons[NONS] = { "ON",
			              "ENABLE",
			              "TRUE"};

/*		Keywords with FALSE logical value   */

#define    NOFFS    3
#define    OFFSIZE    10
static const   char    *offs[NOFFS] = {"OFF",
			               "DISABLE",
			               "FALSE"};

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      gtcmd - Get a command with prompt:
**
**  FORMAL PARAMETERS:
**
**      FILE* fin:
**        File pointer on which input is taken.
**      FILE* fout:
**        File pointer onwhich to issue prompt.  If NULL, no prompt is
**        issued.
**      const char *pr:
**         The prompt string (if null, not issued).
**	char* line:
**    	   Command line buffer
**	unsigned cnt:
**         Size of command line buffer.
**
**  Returns:
**    Pointer to the command string received.
**
**  SIDE EFFECTS:
**
**      I/O done to fin, fout.
**
**--
**/
char* 
gtcmd(FILE* fin, FILE* fout,
      const char* pr, 
      char* line, 
      unsigned cnt)

{
  if(fout && pr) {		/* Need both prompt and output file */
    prompt(fout, pr);		/* Issue prompt		*/
  }
  getlin(fin, line, cnt);	/* get the command line */
  upcase(line);			/* Convert to upper case    */
  return line;			/* As promised, return pointer to line. */
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      prompt	- Issues a prompt string to stdout.
**		    We're trying to minimize the RTL, so the only run time
**		    routine we use is putchar()
**
**  FORMAL PARAMETERS:
**
**      FILE* fout:
**        File pointer to which the prompt is delivered.
**      const char* str:
**         Prompt string assumed to be null terminated.
**
**
**--
**/
void
prompt(FILE* fout, const char* str)
{
  /*  if(isatty(fileno(fout))) { */
    fputs(str, fout);
    fflush(fout);
    /*  } */
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      getlin - Gets an input line.  The input line is assumed to be 
**		  terminated by a non printing character.  If the line doesn't
**		  fit in the input buffer, then we'll truncate, but still read
**		  up to the line terminator.  NOTE: truncation will still leave
**		  the input line terminated with a null.
**
**  FORMAL PARAMETERS:
**
**      FILE* fin:
**         File pointer on which input is taken.
**
**      char* str:
**         Input buffer
**      unsigned cnt 
**         Number of characters in input buffer.
** Returns:
**      Pointer to the string read in.
**
**--
**/
char*
getlin(FILE* fin, char* str, int cnt)
{
  int n;
  fgets(str, cnt, fin);
  n = strlen(str);
  
  /*  Need to trim any newline off the string. */

  if(str[n-1] == '\n')
    str[n-1] = '\0';

  return str;
  
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      upcase	- Converts an entire string to upper case.
**
**  FORMAL PARAMETERS:
**
**      char* str:
**        The string to convert (assumed null terminated).
**
**  RETURNS:
**      Pointer to the converted string.
**
**--
**/
char*
upcase(char *str)
{

    while (*str)
	if (islower(*str))
	    {
		*str = toupper(*str);
		*str++;
	    }
	else
	    str++;
    return str;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      strtoken - Finds a token in an text line.
**
**  FORMAL PARAMETERS:
**
**      const char* str
**            Pointer to null terminated string to search
**	const char* sep
**            Pointer ot a string of delimeter characters
**	char *tok 
**            Pointer to token string, must be long enough to hold
**	      entire token plus null terminator.
**
**
**  FUNCTION VALUE:
**
**      Pointer past the token... suitable for input to next strtoken call.
**
**
**--
**/
char*
strtoken(const char* str, const char* sep, char* token)
{
    /*		External functions	    */

    int    strchr();			/* Is character in string	*/


    while (*str && strchr(sep, *str))
	str++;				/* Hunt for first non separator.    */

    while (*str && (!strchr(sep, *str)))
	*token++ = *str++;		/* Fill token til sep or eol	    */

    *token = '\0';			/* terminate token.		    */

    return    (char*)str;		/* Points to EOS or first sep after token   */
}

/* */
/*++ */
/*  FUNCTIONAL DESCRIPTION: */
/* */
/*      fndkey	- Look up keyword in table. */
/* */
/*  FORMAL PARAMETERS: */
/* */
/*      const char* key:
**            Character string key. */
/*	const char** tbl:
**            Table of valid keyword. */
/*	unsigned nkeys
**            Number of keywords in table. */
/* */
/* */
/*  FUNCTION VALUE: */
/* */
/*      Keyword index or -1 if not found.	    */
/*			 -2 if ambiguous keyword.    */
/* */
/*-- */
/* */
int
fndkey(const char* key, const char** tbl, unsigned nkeys)
{
    int    i;
    int    lastfnd, numfound;
    int    len;
    int    strcmp();

    lastfnd = -1;			    /* Most recent match.	 */
    numfound = 0;			    /* Number of matches	 */
    len	    = strlen(key);		    /* Length of string to lookup */
    for (i=0;  i < nkeys;  i++)
	{
	    if (strncmp(key, *tbl, len) == 0) /* Look for partial match: */
		{

		    lastfnd = i;	    /* save index..		 */
		    numfound++;		    /* Count the found key.	 */
		}
	    tbl++;		    /* Point to next table entry    */
	}
    switch (numfound)
	{

	    case 1:			    /* Only one match is ok. */
		return    lastfnd;	    /* So return index.	     */
		break;
	    case 0:			    /* None found is not found */
		return    -1;		    /* return not found code.  */
		break;
	    default:			    /* Anything else is ambiguous */
		return    -2;		    /* So return the code.	 */
	}
}

/* */
/*++ */
/*  FUNCTIONAL DESCRIPTION: */
/* */
/*      fndkeyex	- Look up keyword in table. Require exact match. */
/* */
/*  FORMAL PARAMETERS: */
/* */
/*      const char* key	
**         Character string key. */
/*	const char** tbl
**         Table of valid keyword. */
/*	unsigned nkeys 
**         Number of keywords in table. */
/* */
/* */
/*  FUNCTION VALUE: */
/* */
/*      Keyword index or -1 if not found. */
/* */
/*-- */
/* */
int    fndkeyex(const char* key, const char** tbl,  unsigned nkeys)
{
    int    i;
    int    strcmp();

    for (i=0;  i < nkeys;  i++)
	if (strcmp(key, *tbl) == 0)	    /* Return index when match	*/
	    return    i;
	else
	    tbl++;		    /* Point to next table entry    */
    return    -1;			    /* Couldn't find at all	    */
}

/**/
/*++:*/
/*  FUNCTIONAL DESCRIPTION:*/
/**/
/*      cvtcnt	- Convert a positive (counting) integer from text to binary*/
/**/
/*  FORMAL PARAMETERS:*/
/**/
/*      txt - String to convert.*/
/**/
/**/
/*  FUNCTION VALUE:*/
/**/
/*      >=0 - Converted value*/
/*	-1  - Invalid unsigned number passed in.*/
/**/
/*--*/
/**/
int	cvtcnt(const char* txt)
{
    int    c1;

    c1 = *txt;				/* Fetch the first character.	*/
    if (isdigit(c1) || (c1 == '+'))	/* Legitimate first character	*/
	return    atoi(txt);		/* Return converted value	*/
    else
	return    -1;			/* Return error value		*/
}

/* */
/*++ */
/*  FUNCTIONAL DESCRIPTION: */
/* */
/*      parsedate   - This function parses a valid date string. */
/*		      into a PSOSdate structure. */
/* */
/*  FORMAL PARAMETERS: */
/* */
/*      const char* cdate	- Character encoded date in 
**                                          %2d-%3s-%4d */
/*					      ^  ^   ^ */
/*					      |  |   +-> Year */
/*					      |  +-----> Month */
/*					      +--------> Day */
/*	struct tm* psosdate:
**         UNIX tm structure to be filled in... Note, only the following 
**         fields will be filled in:
**          tm_mday        - Day of the month
**          tm_mon         - Month of the year.
**          tm_year        - Full year.  
**        all other fields are not modified from the input structure.*/
/* */
/*  IMPLICIT INPUTS: */
/* */
/*      Names of the months are in the keyword table below. */
/* */
/*  COMPLETION CODES: */
/* */
/*      0	- If it worked. */
/*	-1	- If conversion failed. */
/* */
/*-- */
/* */

int    
parsedate(const char* cdate, struct tm* psd)
{
    /*		Local variables		*/

    int    dy, mon, yr;
    char    mtxt[4];
    int    nvals;

    nvals = sscanf(cdate, "%2d-%3s-%4d", &dy, mtxt, &yr);
    if (nvals ==3)
	{				/* Date had proper format   */

	    mon = fndkeyex(mtxt, 
			   (const char**)montbl, 
			   12);      /* Look up month.	*/
	    if (mon >= 0)
		{			/* Found month.. can't fail */

		    psd->tm_mday   = dy;
		    psd->tm_mon    = mon+1;
		    psd->tm_year   = yr;
		    return    0;
		}
	    else			/* Bad month text	    */
		return    -1;
	}
    else				/* Badly formatted string   */
	return    -1;
}

/* */
/*++ */
/*  FUNCTIONAL DESCRIPTION: */
/* */
/*      parsetime   Takes a text time string and translates it into internal */
/*		    system format. */
/* */
/*  FORMAL PARAMETERS: */
/* */
/*      const char* ctime
**             Character time in hh:mm:ss form */
/*			hh  - Hours (0-23) */
/*			mm  - Minutes (0-59) */
/*			ss  - Seconds (0-59) */
/*	struct tm* psostime
**             UNIX time/date structure.  Only the following fields
**             will be filled in: 
**                  tm_hour         - Hour of the day.
**                  tm_min          - Minutes of the hour.
**                  tm_sec          - Seconds of the minute.
**             All other fields of this structure are not modified.
*/
/* */
/*  COMPLETION CODES */
/* */
/*      0   - Parse succeeded */
/*	-1  - Parse failed. */
/* */
/*-- */
/* */
int    
parsetime(const char* ctime, struct tm* pst)
{

    /*	    Local variables */

    int    hh, mm, ss;
    int    ncvt;

    ncvt = sscanf(ctime, "%2d:%2d:%2d", &hh, &mm, &ss);
    if (ncvt == 3)
	{				    /* Correct format	*/

	    if ((hh < 0) || (hh >= 24))
		return    -1;		    /* Invalid hour	*/
	    if ((mm < 0) || (mm >= 60))
		return    -1;		    /* Invalid minute	*/
	    if ((ss < 0) || (ss >= 60))
		return    -1;		    /* Invalid second	*/
	    pst->tm_sec = ss;
	    pst->tm_min =mm;
	    pst->tm_hour = hh;
	    return    0;		    /* Correct parse.	*/
	}
    else
	return    -1;			    /* Bad format.	*/
}

/*								       */
/*++								       */
/*  FUNCTIONAL DESCRIPTION:					       */
/*								       */
/*      isonoff	- Parse logical token.				       */
/*								       */
/*  FORMAL PARAMETERS:						       */
/*								       */
/*      txt - text to check agains valid logical token.		       */
/*								       */
/*  IMPLICIT INPUTS:						       */
/*								       */
/*      offs, ons   - Valid logical keyword tables.		       */
/*								       */
/*--								       */
/*								       */
int    
isonoff(char* txt)
{
    /*	    External functions	*/


    if (fndkey(txt, (const char**)ons,  NONS) >= 0)
	return    1;				/* Logical true.    */
    if (fndkey(txt, (const char**)offs, NOFFS) >= 0)
	return    -1;				/* Logical false.   */
    return    0;				/* Bad text.	    */
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      lookup	- This function looks up an item in a dynamically allocated
**		    symbol table. Exact matches are required.
**
**  FORMAL PARAMETERS:
**
**      const char* key:
**            Pointer to lookup key.
**	const char** tbl:
**	      Array of pointers that makes up table.
**	unsigned tblsiz:
**             Number of entries in table.
**
**
**  FUNCTION VALUE:
**
**      0..tblsiz-1	- Lookup key found.
**	-1		- Key not found.
**
**--
**/
int	
lookup(const char* key, const char** tbl, unsigned tblsiz)
{
    int	i;

    for (i=0;  i < tblsiz;  i++)
	{
	    if (*tbl != 0)
		{
		    if (strcmp(key, *tbl) == 0)
			return i;
		}
	    tbl++;
	}
    return -1;
}
