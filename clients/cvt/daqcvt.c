/*
** Facility:
**   Machine conversions.
** Abstract:
**  daqcvt.c - This file contains routines which can be used
**	       to convert integers of various sizes and shapes
**	       between different machines.  The method is based
**	       on having signature words and longwords with
**	       known values.  These can be taken apart as byte
**	       strings and the mappings between sources and targets
**	       derived.  These mappings are represented as index
**	       tables which are used to do the actual byte re-arrangements.
** Author:
**	Ron Fox
**	NSCL
**	Michigan State Univeristy
**	East Lansing, MI 48824-1321
**
*/
static char *sccsinfo="@(#)daqcvt.c	1.2 7/8/92 Implementation\n";



/*
** Include files:
**
*/

#include "daqcvt.h"

/*
**  Locally used data types:
*/

/*	A word that can be treated as a byte string */

typedef  union {
		  unsigned char bytes[2];
		  short 	whole;
	       } word;

typedef union {
		unsigned char bytes[4];
		long	      whole;
	     } longword;


/*
** Functional Description:
**    DeriveTranslation - This function derives a pair of translation
**			  tables which allow one to map words and longs
**			  in one representation to words and longs in
**			  another representation.
** Formal Parameters:
**	char fromsigw[2]:
**  Signature word in from machine order.
**
**	char tosigw[2];
**  Signature word in to machine order.
**
**	char fromsigl[4]:
**  Signature longword in from machine order.
**
**	char tosigl[4]:
**  Signature longword in to machine order.
**
**	int wordtable[2]:
**  Word translation table (output)
**
**	int longtable[4];
**  Longword translation table (output)
**/
static void DeriveTranslation(fromsigw, tosigw,
			      fromsigl, tosigl,
			      wordtable,longtable)
char fromsigw[2], tosigw[2], fromsigl[4], tosigl[4];
int wordtable[2], longtable[4];
{
    int i,j;

    /* Derive the word table by byte value matching: */

    for(i = 0; i < 2; i++)
       for(j = 0; j < 2; j++)
	  if(fromsigw[i] == tosigw[j])
	    wordtable[i] = j;

    /* Derive the longword table by similar byte matching */

    for(i = 0; i < 4; i++)
	for(j = 0; j < 4; j++)
	    if(fromsigl[i] == tosigl[j])
		longtable[i] = j;


}


/*
** Functional Description:
**     hostsameasforeign    - This function returns true if a set
**			      of conversion tables passed in indicates
**			      that both systems use the same byte ordering.
** Formal Parameters:
**	DaqConversion *conversion:
**  Pointer to translation tables.
*/
int hostsameasforeign(conversion)
DaqConversion *conversion;
{
    int i;


	/* Check out that there's a null mapping in both directions */
	/* for words.						    */

    for(i = 0; i < 2; i++)
	if((conversion->HostToForeignWord[i] != i) ||
	   (conversion->ForeignToHostWord[i] != i)  )
	    return 0;

	/* Apply the same check for longs:  */

    for(i = 0; i < 4; i++)
	if( (conversion->HostToForeignLong[i] != i) ||
	    (conversion->ForeignToHostLong[i] != i)  )
		return 0;

	/* If we get this far, the mapping is unity */

    return 1;
}


/*
** Functional Description:
**	MakeCvtBlock	- This function creates a conversion block
**			  given a set of foreign signatures.
** Formal Parameters:
**	long lsig:
**  Long word signature in foreign machine byte order.
**
**	short ssig:
**  Short word signature in foreign machine byte order.
**
**	DaqConversion *conversion:
**  Pointer to a buffer to hold the mapping arrays.
**/
void makecvtblock(lsig, ssig, conversion)
long lsig;
short ssig;
DaqConversion *conversion;
{
    long hostsigl;
    short hostsigw;

    hostsigl = CVT_LONGSIGNATURE;
    hostsigw = CVT_WORDSIGNATURE;

	/* First derive foreign to host conversions: */

    DeriveTranslation((char *)&ssig, (char *)&hostsigw,
		      (char *)&lsig,  (char *)&hostsigl,
		      conversion->ForeignToHostWord,
		      conversion->ForeignToHostLong);

	/* Second derive host to foreign conversions: */

    DeriveTranslation((char *)&hostsigw, (char *)&ssig,
		      (char *)&hostsigl, (char *)&lsig,
		      conversion->HostToForeignWord,
		      conversion->HostToForeignLong);
}


/*
**  Functional descriptions:
**	ftohl	- Convert foreign to host longwords.
**	ftohw	- Convert foreign to host words.
**/

long ftohl(conversion, datum)
DaqConversion *conversion;
long datum;
{
   longword source;
   longword converted;
   int i;

   source.whole = datum;
   for( i = 0; i < 4; i++)
      converted.bytes[conversion->ForeignToHostLong[i]] = source.bytes[i];

   return converted.whole;
}
short ftohw(conversion, datum)
DaqConversion *conversion;
short datum;
{
    word source;
    word converted;
    int i;

    source.whole = datum;
    for(i = 0; i < 2; i++)
	converted.bytes[conversion->ForeignToHostWord[i]] = source.bytes[i];
    return converted.whole;
}


/*
** Functional Descriptions:
**	htofl	- Convert host to foreign longword.
**	htofw	- Convert word from host format to foreign one.
**/
long htofl(conversion, datum)
DaqConversion *conversion;
long datum;
{
    longword source;
    longword converted;
    int i;

    source.whole = datum;
    for(i = 0; i < 4; i++)
	converted.bytes[conversion->HostToForeignLong[i]] = source.bytes[i];

    return converted.whole;
}
short htofw(conversion,  datum)
DaqConversion *conversion;
short datum;
{
     word source;
     word converted;
     int  i;

     source.whole = datum;
     for(i = 0; i < 2; i++)
	converted.bytes[conversion->HostToForeignWord[i]] = source.bytes[i];
     return converted.whole;
}


