#define autoread                  // set for ECL signal commanded readout cycle

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <tk.h>

#include <iostream>

#ifndef __unix__
#include "cpus.h"
#endif

#ifdef __unix__
#include <stdlib.h>
// #include <daqinterface.h>
//#include <spectrodaq.h>
#endif

#include <daqdatatypes.h>
#include <camac.h>
//#include <ReadoutStateMachine.h>
#include <macros.h>
//#include <hinp32.h>
#include <XLM.h>
// #include <HiRACard.h>
#include <constants.h>

#ifdef VME16
#undef VME16
#endif
#if CPU == MICROPROJECT
#define VME16 0
#endif

#ifndef __unix__
#include <vme.h>
#endif
#include <buftypes.h>

#ifdef __unix__
#include <camac.h>
#else
#if CPU == IRONICS
#undef CAMBAS
#define CAMBAS (b)	0xFA800000
#endif

#if CPU == MICROPROJECT
#undef CAMBAS
#define CAMBAS (b)	0xFE800000
#endif
#endif

#include <CVMUSB.h>

/********************************************************************/

#define MaxMB 3
#define MaxChips  32
#define MaxBoards  MaxMB * 16
#define NumChannels 32
#define TRUE 1
#define FALSE 0
#define MaxShiftLen 122*16 + 160  // 122 bits * 16 boards + 160 for motherboard
#define MSLwords (MaxShiftLen)/16  // # of 16-bit words in MaxShiftLen

#define BANKA FPGA_ABus
//  #define BANKA 0x400014
#define BANKB FPGA_BBus
//  #define BANKB 0x400018

/* global variables */

const int FPGABANK = BANKA;
int XLMSLOT[5];
int XLMTYPE[5];
int xlm_crate[5];
int num_mb;

static int ShiftRegLen[MaxMB+1];
static int NumBoards[MaxMB+1];
static int NumChips[MaxMB+1];
static int SelectedChannel = 0;
static int SelectedChip = 1;
static int SelectedBoard = 1;
static char ShowSignals = 'n';
static char result[24];
static char res1;

const int acq_delay_off=20;
const int pause_delay_off=0;
const int SIS_delay_off=10; 

const int cycle_timeout_off=0;
const int global_timeout_off=10; 

struct ChipStruct {
  char gain;
  char polarity;
  char TVCRange;
  //  char Test1;
  char ExtShaper;
  char DiscMode;
  unsigned int New;
  unsigned int DiscMask;  // needs to be 32 bits
  unsigned int CSARef;
  unsigned int ResetCV;
  unsigned int ARef;
  unsigned int ZC2;
  unsigned int DACRef;
  unsigned int CFDRef;
  unsigned int EOffset;
  unsigned int TOffset;
  unsigned int CFDCap;
  unsigned int CSAOffset;
  unsigned int ShapOffset;
  int Threshold[NumChannels];
  //  int motherboard;             // Which motherboard the chip is in
  int slot;                    // holds number of motherboard slot chip is in
  int pos;                     // tells whether this is first or 2nd chip on board
};

struct MotherboardStruct {
  int config[MaxBoards+1];        // accepts up to 16 slots (1 full motherboard)
  int routing[MaxBoards+1];       // holds routing (A,B,C) selection
  int ChipNo[MaxBoards+1];       // holds number of first chip
  int SumOffA;
  int SumOffB;
  int SumOffC;
  int pause_delay;
  int acq_delay;
  int SIS_delay;
  int cycle_timeout;
  int global_timeout;
  int trigger_delay;
  int force_track_delay;
  int acq_all_delay;
  int glob_dis_delay;
  int coinc_window;
  int forceread;
  int XLM;                       // Which XLM the module is controlled by (0,1,2, etc.)
  int bank;                      // Which connector bank (0:A, 1:B)
};

// forward declarations

void ReloadConfig(int MBNO);
void LoadSerialDAC(int MBNO);
void LoadMBDAC(int MBNO);
void LoadMBMultSteering(int MBNO);
void LoadMBOrSteering(int MBNO);

XLM *theXLM[5];

FILE *dbfile;
struct ChipStruct  chips[MaxMB+1][MaxChips+1];
struct MotherboardStruct motherboard[MaxMB+1];


