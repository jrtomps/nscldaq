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

#ifndef __CCAENV560REGISTERS_H
#define __CCAENV560REGISTERS_H

/*
   This file defines the register layout and bits for the 
   CAENV560N 16 channel 100MHz scaler. It can be included in either
   C or C++.
*/
typedef struct _CAENV560Registers {
  unsigned short pad1[2];	/* Unused			0x0000 */
  unsigned short Ivr;		/* Interrupt Vector register    0x0004 */
  unsigned short Ilvr;		/* Interrupt Level & Veto reg.  0x0006 */
  unsigned short EVMEI;      	/* Enable VME Interrupt         0x0008 */
  unsigned short DVMEI;		/* Disable VME Interrupt	0x000A */
  unsigned short CVMEI;		/* Clear VME Interrupt          0x000C */
  unsigned short Reqr;		/* Request Register             0x000E */
  unsigned long  Counters[16];	/* Live Counter registers       0x0010 */
  unsigned short SClear;	/* Scale clear		        0x0050 */
  unsigned short VMEVS;       	/* VME VETO set 	        0x0052 */
  unsigned short VMEVR;		/* VME VETO reset	        0x0054 */
  unsigned short SInc;		/* Scale Increase		0x0056 */
  unsigned short SSsr;		/* Scale Status register        0x0058 */
  unsigned short pad2[0x50];	/* Unused			0x005A */
  unsigned short FCode;		/* Fixed code 			0x00FA */
  unsigned short Mfrmod;	/* Manufacturer & Module Type   0x00FC */
  unsigned short VerSN;		/* Version & Serial Number      0x00FE */

} CAENV560Registers, *pCAENV560Registers;


/*  Scale status register mask bits: 0 - 2X32 bit, 1 - 1X64 bit       */
/*                S0    S1    S2    S3    S4    S5    S6    S7        */
/*                Bit3  Bit2  Bit1  Bit0  Bit7  Bit6  Bit5  Bit4      */
int SSRMask[8] = {0x08, 0x04, 0x02, 0x01, 0x80, 0x40, 0x20, 0x10};

/* Interrupt Level and Veto Register Masks                    */

#define Intlev(Word)   ((Word) & 0x3) 	/* Interrupt Level    */
#define Vetostat(Word) (((Word) >> 7) & 0x1) /* 0-inhibit, 1-count   */

/* Interrupt Vector register                                         */

#define IVRStat(Word)  ((Word) &  0xff)	/* Interrupt Ack. Status/ID  */

/* Module Identifier Word  macros:  */

#define MIFAFC(Word)   (((Word) >> 8) & 0xff)
#define MIF5FC(Word)   ((Word) & 0xff)
#define MIModTyp(Word) ((Word) & 0x3ff)
#define MIMfrNo(Word)  (((Word) >> 10) & 0x3f)
#define MISN(Word)     ((Word) & 0xfff)
#define MIVer(Word)    (((Word) >> 12) & 0xf)

#endif
