/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright MADCDELAY5.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __MADC32REGISTERS_H
#define __MADC32REGISTERS_H

#ifndef Const
#define Const(name) static const int name =
#endif


Const(MADCDELAY)  1;

// The address modifiers that will be used to access the module:

static const uint8_t initamod(CVMUSBReadoutList::a32UserData);   //  setup using user data access.
static const uint8_t readamod(CVMUSBReadoutList::a32UserBlock);  //  Read in block mode.

static const uint8_t cbltamod(CVMUSBReadoutList::a32UserBlock);
static const uint8_t mcstamod(CVMUSBReadoutList::a32UserData);


// Module address map; for the most part I'm only defining the registers
// we'll actually use.

Const(eventBuffer)          0;

Const(Thresholds)           0x4000;

Const(AddressSource)        0x6000;
Const(Address)              0x6002;
Const(ModuleId)             0x6004;
Const(Reset)                0x6008; // write anything here to reset the module.

Const(Ipl)                  0x6010;
Const(Vector)               0x6012;
Const(IrqReset)             0x6016;
Const(IrqThreshold)         0x6018;
Const(MaxTransfer)          0x601a;
Const(WithdrawIrqOnEmpty)   0x601c;

Const(CbltMcstControl)      0x6020;
Const(CbltAddress)          0x6022;
Const(McstAddress)          0x6024;


Const(LongCount)            0x6030;
Const(DataFormat)           0x6032;
Const(ReadoutReset)         0x6034;
Const(MultiEvent)           0x6036;
Const(MarkType)             0x6038;
Const(StartAcq)             0x603A;
Const(InitFifo)             0x603c;
Const(DataReady)            0x603e;

Const(BankOperation)        0x6040;
Const(Resolution)           0x6042;
Const(OutputFormat)         0x6044;

Const(HoldDelay0)           0x6050;
Const(HoldDelay1)           0x6052;
Const(HoldWidth0)           0x6054;
Const(HoldWidth1)           0x6056;
Const(EnableGDG)            0x6058;
Const(InputRange)           0x6060;
Const(ECLTermination)       0x6062;
Const(ECLGate1OrTiming)     0x6064;
Const(ECLFCOrTimeReset)     0x6066;
Const(NIMGate1OrTiming)      0x606a;
Const(NIMFCOrTimeReset)     0x606c;
Const(NIMBusyFunction)      0x606e;
Const(EventCounterReset)    0x6090;
Const(TimingSource)         0x6096;
Const(TimingDivisor)        0x6098;
Const(TimestampReset)       EventCounterReset; // As of firmware 5.

Const(TestPulser)           0x6070; // In order to ensure it's off !


// Mcast/CBLT control register bits:

Const(MCSTENB)              0x80;
Const(MCSTDIS)              0x40;
Const(FIRSTENB)             0x20;
Const(FIRSTDIS)             0x10;
Const(LASTENB)              0x08;
Const(LASTDIS)              0x04;
Const(CBLTENB)              0x02;
Const(CBLTDIS)              0x01;

#endif
