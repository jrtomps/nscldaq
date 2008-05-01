/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

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
		  uint8_t       bytes[2];
		  uint16_t 	whole;
	       } word;

typedef union {
		uint8_t               bytes[4];
		uint32_t	      whole;
	     } longword;


/*
** Functional Description:
**    DeriveTranslation - This function derives a pair of translation
**			  tables which allow one to map words and longs
**			  in one representation to words and longs in
**			  another representation.
** Formal Parameters:
**	uint8_t fromsigw[2]:
**  Signature word in from machine order.
**
**	uint8_t tosigw[2];
**  Signature word in to machine order.
**
**	uint8_t fromsigl[4]:
**  Signature longword in from machine order.
**
**	uint8_t tosigl[4]:
**  Signature longword in to machine order.
**
**	int wordtable[2]:
**  Word translation table (output)
**
**	int longtable[4];
**  Longword translation table (output)
**/
static void DeriveTranslation(uint8_t* fromsigw, 
			      uint8_t* tosigw,
			      uint8_t* fromsigl, 
			      uint8_t* tosigl,
			      int*     wordtable,
			      int*     longtable)
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
bool hostsameasforeign(conversion)
DaqConversion *conversion;
{
    int i;


	/* Check out that there's a null mapping in both directions */
	/* for words.						    */

    for(i = 0; i < 2; i++)
	if((conversion->HostToForeignWord[i] != i) ||
	   (conversion->ForeignToHostWord[i] != i)  )
	    return false;

	/* Apply the same check for longs:  */

    for(i = 0; i < 4; i++)
	if( (conversion->HostToForeignLong[i] != i) ||
	    (conversion->ForeignToHostLong[i] != i)  )
		return false;

	/* If we get this far, the mapping is unity */

    return true;
}


/*
** Functional Description:
**	MakeCvtBlock	- This function creates a conversion block
**			  given a set of foreign signatures.
** Formal Parameters:
**	uint32_t lsig:
**  Long word signature in foreign machine byte order.
**
**	uint16_t ssig:
**  Short word signature in foreign machine byte order.
**
**	DaqConversion *conversion:
**  Pointer to a buffer to hold the mapping arrays.
**/
void makecvtblock(uint32_t lsig, 
		  uint32_t ssig, 
		  DaqConversion* conversion)
long lsig;
short ssig;
DaqConversion *conversion;
{
    uint32_t hostsigl;
    uint16_t hostsigw;

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

uint32_t ftohl(DaqConversion* conversion, 
	   uint32_t       datum)
{
   longword source;
   longword converted;
   int i;

   source.whole = datum;
   for( i = 0; i < 4; i++)
      converted.bytes[conversion->ForeignToHostLong[i]] = source.bytes[i];

   return converted.whole;
}
uint16_t ftohw(DaqConversion* conversion, 
	    uint16_t datum)
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
uint32_t htofl(
	       DaqConversion* conversion, 
	       uint32_t       datum)
{
    longword source;
    longword converted;
    int i;

    source.whole = datum;
    for(i = 0; i < 4; i++)
	converted.bytes[conversion->HostToForeignLong[i]] = source.bytes[i];

    return converted.whole;
}
uint16_t htofw(
	       DaqConversion* conversion,  
	       uint16_t       datum)
{
     word source;
     word converted;
     int  i;

     source.whole = datum;
     for(i = 0; i < 2; i++)
	converted.bytes[conversion->HostToForeignWord[i]] = source.bytes[i];
     return converted.whole;
}


