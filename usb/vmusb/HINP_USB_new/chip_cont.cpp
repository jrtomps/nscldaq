/*
** Ported to Linux and CES/CBD 8210 device driver.
** At present, the preprocessor symbol __unix__ is used to switch on/off
** Linux/unix specific  code. #ifndef for it is used to switch off VME 
** 68K specific code.
**   >>>>This module must be compiled without optimization<<<<
**      Ron Fox
**      January 28, 1999
*/
// mod 3/5/03 JE for hardware-controlled acq cycle
// mod 10/23/02 Jon Elson for SIS 3301
// mod 12/4/02 Jon Elson to put HINP16 acquisition sequence into code
// mod 3/24/03 Jon ELson added Tk/Tcl control of chip
// mod 12/01/03 JE added DAC on chip board to serial string
// mod 7/7/06 JE move CSA and Shaper offsets to chip data structure
// mod 7/10/06 JE make show signals global
// mod 11/7/06 JE add exclusive option to show signals
// mod 7/2009 -8/2009 unify chips on board to one screen, support new chip board version
//                       put MB settings first in file
// mod 9/3/2010 JE for new NSCL software

/* Program notes :
This program is the "back end" of the silstrip.tcl and motherboard1.tcl GUI pages.
The C++/Tcl interface is handled by a SWIG (software interface group) library.
For convenience, I have made a prototype-only c file "chip_cont.swi" that only contains
the function prototypes for functions in this file that are to be called from the GUI.
These are mostly Getxxx and SetChipxxx functions.  SWIG uses the .swi file to create
chip_cont_wrap.cxx, the wrapper that handles the CPP name mangling and type conversions.
This file HAS to be a .CXX file to get the (un)mangling right.
The Makefile handles the generation and compiling of the wrapper file.
*/

#include <chip_cont.h>
#include <fstream>
#include <string.h>

#define  serialDelay 135

// general initial program setup, find out how many XLMs, motherboards, etc. are
//   in the system
// this program is capable of working with the XLM XXV, the XLM 80M is considered obsolete.
// the file export_path sets up environment variable READOUTINPUTPATH to point to the config file's directory.

void InitXLMs ()
{
  int numcsi,numadc,numxlm,xlm_count,xlm_side;
  char line[256];
  string buffer;
  
  // FIXME - getenv segfaults if env var is not defined
  const char* pEnvVar;
  pEnvVar =  getenv("READOUTINPUTPATH");
  
  if (pEnvVar == 0) {
    printf("environment variable READOUTINPUTPATH is not defined, exiting.\n");
    exit(1);
  }
  buffer = pEnvVar;

  string filename = buffer + "/config.dat";
  cout << "Using configuration file:  " << filename << endl;

  ifstream file(filename.c_str(),ios::in);
  /*  configuration file config.dat contents :
First line :  <Number of XLM modules>  <Number of MotherBoards>
Remaining lines to match Number of XLM modules
     <XLM crate #>  <XLM Slot #>  <Type of XLM>
Type of XLM :  1 means one LVDS port with integral ADC
               2 means two LVDS ports with external ADC (SiS 3100)
  */
      
  string iname;
  string name = "CHIP";
  bool chipDefined = false;
  int xlm_crate[60];
  //get number of XLMs and their location from the config file
  while (!file.eof()){
    cout << "begin reading file" << endl;
    file >> iname; // get name of processor to consider
    cout << "name is " << name << endl;
    file.getline(line,256);
    if (iname == name){
      cout << "processor name " << name << endl;
      file >> numxlm >> num_mb;
      cout << "number of XLMs " << numxlm << endl;
      chipDefined = true;

      for(int i=1;i<numxlm+1;i++)
	{
	  file.getline(line,256);
	  file >> xlm_crate[i] >> XLMSLOT[i] >> XLMTYPE[i];
	  cout << "Slot " << XLMSLOT[i] << " LVDS Ports  " << XLMTYPE[i] << " number " << i <<endl;
	}
    }     
  }
  file.close();

  //initialize how the XLMs are connected to the motherboards
  
  xlm_count = 1;
  xlm_side = BANKA;
  printf("Init BANKA = %x\n",BANKA);
  for(int i=1;i<num_mb+1;i++) {
    motherboard[i].XLM = xlm_count;
    motherboard[i].bank = xlm_side;
    //    cout << "i " << i << " xlm_count " << xlm_count << " xlm_side " << xlm_side << endl;
    printf("i %d xlm_count %d xlm_side %x\n",i,xlm_count,xlm_side);
    if (XLMTYPE[xlm_count] == 1) {
      xlm_count ++;
      xlm_side = BANKA;
    } else {
      if (xlm_side == BANKB) {
	xlm_side = BANKA;
	xlm_count ++;
      } else {
	xlm_side = BANKB;
      }
    }
    //    xlm_count--;      // correct XLM count
  }

  cout <<"Motherboard structure made." <<endl;
  /******************************/
  int done[22];
  for(int i=0;i<22;i++)
    done[i]=0;

  cout<<"Initializing XLMs"<<endl;

  cout << " numxlm = " << numxlm << endl;
  for(int i=1;i<numxlm+1;i++)
    {
      char title[100];
      sprintf(title,"ASICSCard%d\n",i);      
      cout<<"   XLM "<<i<< " crate " << xlm_crate[i] << " slot " << XLMSLOT[i] << endl;
      theXLM[i] = 
       	new XLM(title,xlm_crate[i],XLMSLOT[i]);
      cout<<"    XLM initialized."<<endl;
      theXLM[i]->Initialize(title,xlm_crate[i],XLMSLOT[i]);
      theXLM[i]->initList(); // do this only once per program invocation
    }
}

void ResetXLM(int MBNO)
{
  //  int XLMNUM = motherboard[MBNO].XLM;
  //  theXLM[XLMNUM]->BootFPGA();
}

// scans motherboard to determine pattern of occupied/vacant slots, and whether
//    those are new or old style chip boards
//    old option jumper bit that was used to signify one or two chips per board now
//    indicates new or old board style (thus all boards must have 2 chips)
void ScanMothers(int MBNO) 
{
  // following is for scanning configuration registers of motherboard
  
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;

  unsigned short cfg_readback[2112];  // one short for each bit in string
  
  int value,i,j,k,lo,hi,done,serport;
  int status, bytesRead;
  long response[150];           // buffer for response from XLM
  long* Presp =&response[0];    // set pointer to response buffer
  long l;
  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }
  for (i=1;i<=MaxBoards;i++) 
    {  // clear config array
      motherboard[MBNO].config[i]=0;
    }
  for (i=1;i<=MaxChips;i++) 
    {
      chips[MBNO][i].slot =0;
      chips[MBNO][i].pos = 0;
    }

  theXLM[XLMNUM]->clearList();
  theXLM[XLMNUM]->addWrite32(FPGA+FBANK,0);
  // transfer config bit pattern into parallel load SR in motherboard FPGA
  theXLM[XLMNUM]->addWrite32(FBANK,forcereset|selextbus);
  theXLM[XLMNUM]->addWrite32(FBANK,forcereset|serclk|selextbus);
  theXLM[XLMNUM]->addWrite32(FBANK,forcereset|selextbus);
  theXLM[XLMNUM]->addWrite32(FBANK,selextbus);

  done = FALSE;
  for (i=0;i<ShiftRegLen[MBNO];i+= 16) 
    {
      theXLM[XLMNUM]->addWrite32(serport, 1); //send single 1 sentinel bit
      theXLM[XLMNUM]->addDelay(serialDelay);   // delay between serial transmissions
      theXLM[XLMNUM]->addRead32(serport); //read back data
    }
  //  printf("sizeof(response) reads %d\n",sizeof(response));
  //  printf("Presp = %lx\n",Presp);
  status = theXLM[XLMNUM]->executeList(Presp, sizeof(response), &bytesRead);
  l=0;  
  for (i=0;i<ShiftRegLen[MBNO];i+= 16) {
    k = response[l]>>16 & 0xffff;  // readback data is in bits 31..16 of the serport register
    l++;
    for (j=0;j<16;j++) {
      cfg_readback[i+j+1]=k & 1;
      k = k >> 1;
    }
  }
  printf("dump of shift reg follows\n");
  for (i=1;i<ShiftRegLen[MBNO];i++)  
    {
      printf("%d",cfg_readback[i]);
    }
  printf("\n");
  NumChips[MBNO]=0;
  NumBoards[MBNO]=0;
  j=1;           // counter to match chip numbers to board slots

  for (i=1;i<17;i++) 
    {
      printf("Motherboard: %d slot %d  ",MBNO,i);
      if (cfg_readback[97-i] == 0)                 // check if slot occupied
	{
	  motherboard[MBNO].ChipNo[i]=NumChips[MBNO]+1;
	  if (cfg_readback[81-i] == 0)            // check type of chip in slot
	    {
	      printf(" contains OLD chipset\n");
	      NumChips[MBNO] +=2;
	      NumBoards[MBNO]++;
	      motherboard[MBNO].config[i]=2;
	      chips[MBNO][j].slot = i;  // first chip on board
	      chips[MBNO][j].pos = 1;  // first chip on board
	      chips[MBNO][j].New = FALSE;
	      //	      printf("Setting MB %d chip %d New to %d\n",MBNO,j,chips[MBNO][j].New);
	      j++;
	    } 
	  else 
	    {
	      printf(" contains NEW chips\n");
	      NumChips[MBNO] +=2;
	      NumBoards[MBNO]++;
	      motherboard[MBNO].config[i]=2;
	      chips[MBNO][j].slot = i;  // first chip on board
	      chips[MBNO][j].pos = 1;  // first chip on board
	      chips[MBNO][j].New = TRUE;
	      //	      printf("Setting MB %d chip %d New to %d\n",MBNO,j,chips[MBNO][j].New);
	      j++;
	    }
	} 
      else
	printf(" is empty\n");
    }
  cout<<"Motherboard "<<MBNO<<endl;
  printf("Config Registers report NumBoards[%d] = %d, NumChips[%d] = %d\n",MBNO,NumBoards[MBNO],MBNO,NumChips[MBNO]);
  // compute how long shift reg should be for this config
  j=NumChips[MBNO]*48 + NumBoards[MBNO]*16 + 160;
  if (j != ShiftRegLen[MBNO]) 
    printf("which should have a shift reg length of %d\n",j);
  for (i=1;i<=NumChips[MBNO];i++) {
    printf("Motherboard %d  Chip %d will have ID %d\n",MBNO,i,chips[MBNO][i].slot);
  }
}

// clear system, check functionality of serial string and measure length, in bits,
//     of the serial string
void InitChip(int MBNO) 
{
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;
  int testPattern[7] = {0, 0x1377, 0xcccc, 0x7777, 0xffff, 0x7777, 0x3333};
  unsigned long cfg_array[2]=
    { 0x0000FDFF , 0xffff0800};    // neg pulse at CSA out, ext CSA, long TFC range, enable chan 6 only
  
  unsigned long cfg_readback[2]={0,0};

  int value,i,j,k,lo,hi,done,serport;
  int status, bytesRead;
  long response[150];           // buffer for response from XLM
  long l;
  long* Presp =&response[0];    // set pointer to response buffer

  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }
  theXLM[XLMNUM]->clearList();
  theXLM[XLMNUM]->addWrite32(AccBus, 0x10000);  // take control of FPGA bus
  theXLM[XLMNUM]->addDelay(50);
  // turn off glbl_enable
  theXLM[XLMNUM]->addWrite32(FBANK, selextbus);
  // toggle MB shift reg controls for LTC1660 and MAX335 chips
  theXLM[XLMNUM]->addWrite32(FBANK, forcereset | selextbus);
  theXLM[XLMNUM]->addWrite32(FBANK, selextbus);

  status = theXLM[XLMNUM]->executeList(Presp,sizeof(response), &bytesRead);
  theXLM[XLMNUM]->clearList();
  theXLM[XLMNUM]->addRead32(serport);
  status = theXLM[XLMNUM]->executeList(Presp,sizeof(response), &bytesRead);
  theXLM[XLMNUM]->clearList();

  // now test ability to do it 16 bits at a time
  printf("now test MotherBoard %d on XLMNUM %d FBANK %d\n",MBNO,XLMNUM,FBANK);

  printf("MaxShiftLen = %d  MSLwords = %d\n",MaxShiftLen,MSLwords);
  for (i=0; i<=MSLwords; i++) {
    theXLM[XLMNUM]->addWrite32(serport, 0); //send plenty of zeros to fill shift reg
    theXLM[XLMNUM]->addDelay(serialDelay);   // delay between serial transmissions
  }
  theXLM[XLMNUM]->addRead32(serport);   // throw in a dummy read, makes list execute faster
  theXLM[XLMNUM]->executeList(Presp, sizeof(response), &bytesRead);
  printf("Sent zeros through, bytes read = %d\n",bytesRead);
  theXLM[XLMNUM]->clearList();
  theXLM[XLMNUM]->addWrite32(serport, 1); //send single 1 sentinel bit
  theXLM[XLMNUM]->addDelay(serialDelay);   // delay between serial transmissions
  theXLM[XLMNUM]->addRead32(serport); //read back data
  for (i=0; i<=MSLwords; i++) {
    theXLM[XLMNUM]->addWrite32(serport, 0); //send enough zeros to fill shift reg
    theXLM[XLMNUM]->addDelay(serialDelay);   // delay between serial transmissions
    theXLM[XLMNUM]->addRead32(serport); //read back data
  }
  status = theXLM[XLMNUM]->executeList(Presp, sizeof(response), &bytesRead);
  //  printf("Read back serial response\n");

  if ((response[0] & 0xffff) == 1) { // funky write status???
    char* pd = reinterpret_cast<char*>(response);
    memmove(pd, pd+2, sizeof(response)-2);
  }

  if (response[0] != 0)// check last bits of shift reg
    {
      printf("after loading %d zeros, shift reg still outputs a 1\n",MaxShiftLen);
      // HACK   exit(1);
    }
  i=0;
  k=0;
  done = FALSE;
  while (i <= MaxShiftLen && !done)
    {
      j = response[k];        //shift 16 positions
      k++;
      if (j != 0)
      	{
        while (j & 1 != 1) {
          i++;
          j = j >> 1;
        }
        printf("(16 bits) shift register is %d bits long.\n",i);
        ShiftRegLen[MBNO] = i;
	done = TRUE;
      }
      else i+=16;
      printf("(fast mode) shift reg is %d bits long.\n",i);
      if (i>= MaxShiftLen) {
	printf("Either shift register > %d bits long or output stuck at Zero.\n",MaxShiftLen);
	done = TRUE;
      }
    }
  //  if (ShiftRegLen[MBNO] < 160) ShiftRegLen[MBNO] = 160;
  if (ShiftRegLen[MBNO] < 160) ShiftRegLen[MBNO] = 272;
  //  check serial string for data reliability
  theXLM[XLMNUM]->clearList();
  for (i=0; i<7; i++) {
    theXLM[XLMNUM]->addWrite32(serport, testPattern[i]); //write test pattern
    theXLM[XLMNUM]->addDelay(serialDelay);   // delay between serial transmissions
  }
  for (j=0;j<ShiftRegLen[MBNO]/16-7;j++) {
    theXLM[XLMNUM]->addWrite32(serport, 0);
    theXLM[XLMNUM]->addDelay(serialDelay);   //scroll past rest of shift reg
  }
  for (j=0;j<7;j++) {
    theXLM[XLMNUM]->addWrite32(serport, 0);
    theXLM[XLMNUM]->addDelay(serialDelay);
    theXLM[XLMNUM]->addRead32(serport); //read back 7 test pattern words
  }
  status = theXLM[XLMNUM]->executeList(Presp, sizeof(response), &bytesRead);

  k=0;
  for (i=0; i<7; i++) {
    if ((response[i]>>16 & 0xffff) != testPattern[i]) {
      printf("error, serial pattern %d should read %x but was %lx\n",i,testPattern[i],(response[i]>>16 & 0xffff));
    k=1;
    }
  }
  if (k == 0)
    printf("serial shift register passes data reliability test.\n");
  else
    printf("data errors in serial shift register!\n");
  ScanMothers(MBNO);  // read config pattern from motherboard FPGA
  
  printf("returned from ScanMothers()\n");
  theXLM[XLMNUM]->WriteFPGA(FBANK,selextbus);  // turn off bus drivers on chip boards
  
  
  // step 4.  setup to acquire data
  theXLM[XLMNUM]->WriteFPGA(FBANK,selextbus);
  
  theXLM[XLMNUM]->WriteFPGA(FBANK,glbl_enable|forcereset|selextbus);
  theXLM[XLMNUM]->WriteFPGA(FBANK,selextbus);               //??????????
  i=theXLM[XLMNUM]->ReadFPGA(FBANK);
  cout<<(hex)<<i<<(dec)<<endl; 
  
  if (!(i & orout))
    printf("WARNING! OR does not = 0 as it should after reset!\n");
  if (!(i & token_out)) 
    printf("WARNING! token_out is not high (false) as it should be after reset with no tokenin!\n");
  
  theXLM[XLMNUM]->WriteFPGA(FBANK,tokenin|selextbus);
  i=theXLM[XLMNUM]->ReadFPGA(FBANK);
  
  cout<<(hex)<<i<<(dec)<<endl;  
  
  if (!(i & token_out)) 
    printf("WARNING! token_out is not low (true) as it should be after reset WITH tokenin asserted!\n");
  if (!(i & acqack))
    printf("WARNING! acqack does not = 0 as it should after a reset!\n");
  for (i=0;i<25000;i++) {                    // wait a few us
    j=i+1;
  }
  for (int i=0;i<17;i++) {
    cout<<motherboard[MBNO].config[i]<<endl;
  }
}

/* the following GetXxxx functions return the setting of a particular choice
   to the GUI, after looking up the setting in the database.  */
/* copies  variables from array of structs  chips[] to individual variables */
//  in all these routines, ChipNo now refers to sequential BOARD number

// gets many settings for specified board  (ChipNo)
void GetChip(int MBNO, int ChipNo, char* gain, 
	     char* polarity, char* TVCRange, 
	     char* Test1, char* ExtShaper, char* DiscMode)
{
  int i;

  *gain = chips[MBNO][ChipNo].gain;
  *polarity = chips[MBNO][ChipNo].polarity;
  *TVCRange = chips[MBNO][ChipNo].TVCRange;
  //  *Test1 = chips[MBNO][ChipNo].Test1;
  *ExtShaper = chips[MBNO][ChipNo].ExtShaper;
  *DiscMode = chips[MBNO][ChipNo].DiscMode;
}

char*  GetGain(int MBNO, int ChipNo) {
    //printf("Board %d gain is %c\n",ChipNo, chips[MBNO][ChipNo].gain);
  char* res = &result[0];
  switch (chips[MBNO][ChipNo].gain) {
  case 'l' :  sprintf(res, "low     \x00");
    break;
  case 'h' :  sprintf(res,"high    \x00");
    break;
  case 'e' :  sprintf(res, "external\x00");
    break;
  }
  return (res);
}

char*  GetNew(int MBNO, int ChipNo) {
  //  printf("MB %d Board %d type is %c\n",MBNO, ChipNo, chips[MBNO][ChipNo].New);
  char* res = &result[0];
  switch (chips[MBNO][ChipNo].New) {
  case TRUE  :  sprintf(res, "New\x00");
    break;
  case FALSE :  sprintf(res, "Old\x00");
    break;
  }
  return (res);
}

char* GetPolarity(int MBNO, int ChipNo) {
    //printf("Board %d Pol is %c\n",ChipNo, chips[MBNO][ChipNo].polarity);
  char* res = &result[0];
  switch (chips[MBNO][ChipNo].polarity) {
  case 'p' :  sprintf(res, "positive\x00");
    break;
  case 'n' :  sprintf(res,"negative\x00");
    break;
  }
  return (res);
}

char* GetTVCRange(int MBNO, int ChipNo) {
    //printf("Board %d TVC is %c\n",ChipNo, chips[MBNO][ChipNo].TVCRange);
  char* res = &result[0];
  switch (chips[MBNO][ChipNo].TVCRange) {
  case 's' :  sprintf(res, "short\x00");
    break;
  case 'l' :  sprintf(res,"long \x00");
    break;
  }
  return (res);
}

char* GetTest1(int MBNO, int ChipNo) {
  char* res = &result[0];
  //    printf("Board %d ShowSignals is *%c*\n",ChipNo, ShowSignals);
  switch (ShowSignals) {
  case 'y' : sprintf(res,"yes      \x00");
    break;
  case 'n' : sprintf(res,"no       \x00");
    break;
  case 'e' : sprintf(res,"exclusive\x00");
    break;
  }
  //  printf("in GetTest1 returns *%s*\n",res);
  return (res);
}

char* GetExtShaper(int MBNO, int ChipNo) {
    //printf("Board %d ExtShaper is %c\n",ChipNo, chips[MBNO][ChipNo].ExtShaper);
  char* res = &result[0];
  switch (chips[MBNO][ChipNo].ExtShaper) {
  case 'e' :  sprintf (res, "external\x00");
    break;
  case 'i' :  sprintf(res,"internal\x00");
    break;
  }
  return (res);
}

char* GetDiscMode(int MBNO, int ChipNo) {
  char* res = &result[0];
    //printf("Board %d DiscMode is %c\n",ChipNo, chips[MBNO][ChipNo].DiscMode);
    switch (chips[MBNO][ChipNo].DiscMode) {
    case 'a' :  sprintf(res, "all     ");
      break;
    case 'n' :       sprintf(res,"none    ");
      break;
    case 's' :       sprintf(res,"selected");
      break;
    case 'm' :       sprintf(res,"mask    ");
      break;
    case 'e' :       sprintf(res,"evens   ");
      break;
    case 'o' :       sprintf(res,"odds    ");
      break;
    }
  return (res);
}

char* GetDiscMask(int MBNO, int ChipNo) {
  char* res = &result[0];
  //printf("GetDiscMask for Board %d  %d\n",ChipNo,chips[MBNO][ChipNo].DiscMask);
  sprintf(res,"%d",chips[MBNO][ChipNo].DiscMask);
  return (res);
}


int GetThreshold(int MBNO, int ChipNo, int Channel)
{
    return chips[MBNO][ChipNo].Threshold[Channel];
}

int GetCSARef(int MBNO, int ChipNo)
{
  //printf("GetCSARef for Board %d is %d\n",ChipNo,chips[MBNO][ChipNo].CSARef);
    return chips[MBNO][ChipNo].CSARef;
}

int GetResCV(int MBNO, int ChipNo)
{
  //printf("GetResCV for Board %d is %d\n",ChipNo,chips[MBNO][ChipNo].ResetCV);
    return chips[MBNO][ChipNo].ResetCV;
}

int GetARef(int MBNO, int ChipNo)
{
    return chips[MBNO][ChipNo].ARef;
}

int GetZC2(int MBNO, int ChipNo)
{
    return chips[MBNO][ChipNo].ZC2;
}

int GetDACRef(int MBNO, int ChipNo)
{
    return chips[MBNO][ChipNo].DACRef;
}

int GetCFDRef(int MBNO, int ChipNo)
{
    return chips[MBNO][ChipNo].CFDRef;
}

int GetEOff(int MBNO, int ChipNo)
{
  if (chips[MBNO][ChipNo].pos == 2) {
    return chips[MBNO][ChipNo-1].EOffset;
  } else {
    return chips[MBNO][ChipNo].EOffset;
  }
}

int GetTOff(int MBNO, int ChipNo)
{
  if (chips[MBNO][ChipNo].pos == 2) {
    return chips[MBNO][ChipNo-1].TOffset;
  } else {
     return chips[MBNO][ChipNo].TOffset;
  }
}

int GetCFDCap(int MBNO, int ChipNo)
{
     return chips[MBNO][ChipNo].CFDCap;
}

int GetShapOff(int MBNO, int ChipNo)
{
    return chips[MBNO][ChipNo].ShapOffset;
}

int GetCSAOff(int MBNO, int ChipNo)
{
    return chips[MBNO][ChipNo].CSAOffset;
}

char* GetSlotStatus(int MBNO, int slot)
{
  char* res = &result[0];
  int chip;
  //  printf("Motherboard[MBNO].config[%d] = %d\n",slot,motherboard[MBNO].config[slot]);
  if (motherboard[MBNO].config[slot] == 0) {
    sprintf(result,"Empty    \x00");
  } else {
    chip = motherboard[MBNO].ChipNo[slot];
    chip = chip/2 + 1;
    //    printf("MB %d chip %d New %x\n",MBNO,chip,chips[MBNO][chip].New);
    if (chips[MBNO][chip].New == TRUE) 
      sprintf(result,"New Chips\x00");
    else
      sprintf(result,"Old Chips\x00");
  }
  //  printf("GetSlotStatus for MB %d slot %d returns chip # %d %s %x\n",MBNO,slot,chip,res,chips[MBNO][chip].New);
  return (res);
}

char* GetChipLocation(int MBNO, int chip)
{
  char* res = &result[0];
  sprintf(result,"Slot %d\x00",chips[MBNO][chip].slot);
  return (res);
}

char* GetSlotRouting(int MBNO, int slot)
{
  char* res = &result[0];
  //  printf("GetSlotRouting (%d) returns %d\n",slot,motherboard[1].routing[slot]);
  switch (motherboard[MBNO].routing[slot]) {
  case 0 : result[0] = 'O';
    break;
  case 1 : result[0] = 'A';
    break;
  case 2 : result[0] = 'B';
    break;
  case 3 : result[0] = 'C';
    break;
   }
  result[1] = '\x00';  // terminate string with null char
  return(res);
}

int GetMaxMB()
{
  return num_mb;
}

int GetSumOffA(int MBNO)
{
  //printf("GetSumOffA returns %d\n",motherboard[1].SumOffA);
    return motherboard[MBNO].SumOffA;
}

int GetSumOffB(int MBNO)
{
    return motherboard[MBNO].SumOffB;
}

int GetSumOffC(int MBNO)
{
    return motherboard[MBNO].SumOffC;
}


int GetSISDelay(int MBNO)
{
    return motherboard[MBNO].SIS_delay;
}

int GetAcqDelay(int MBNO)
{
    return motherboard[MBNO].acq_delay;
}

int GetPauseDelay(int MBNO)
{
    return motherboard[MBNO].pause_delay;
}

int GetCycleTimeout(int MBNO)
{
    return motherboard[MBNO].cycle_timeout;
}

int GetGlobalTimeout(int MBNO)
{
    return motherboard[MBNO].global_timeout;
}

int GetTriggerDelay(int MBNO)
{
    return motherboard[MBNO].trigger_delay;
}

int GetCoincWindow(int MBNO)
{
    return motherboard[MBNO].coinc_window;
}

int GetFTDelay(int MBNO)
{
    return motherboard[MBNO].force_track_delay;
}

int GetAADelay(int MBNO)
{
    return motherboard[MBNO].acq_all_delay;
}

int GetGDDelay(int MBNO)
{
    return motherboard[MBNO].glob_dis_delay;
}

int GetForceRead(int MBNO)
{
    return motherboard[MBNO].forceread;
}

// loads the delay settings into the XLM
// this routine is meant to be called from a routine that has already locked the
//  VME and taken control of the FPGA busses
void LoadDelays_inner(int MBNO)
{
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;


  theXLM[XLMNUM]->addWrite32(FPGA_enblA,0);  // turn off external Auto enable to prevent jamming XLM
  theXLM[XLMNUM]->addWrite32(FPGA_enblB,0);  // turn off external Auto enable to prevent jamming XLM
  theXLM[XLMNUM]->addDelay(250);

  /*  Clear the Veto  */

  //  theXLM[XLMNUM]->WriteFPGA(FPGA_clear_veto,1);  //Clear XLM veto latch
  // commented out for now, seems to interfere with loading delays
  //  theXLM[XLMNUM]->WriteFPGA(FPGA_reset,0xffff);  //Reset all XLM state machines

  // set timeouts and delays on XLM80M

  unsigned long delayval1,delayval2,delayval3,delaytot;

  delayval1 = long(float(motherboard[MBNO].pause_delay)/25);
  delayval2 = long(float(motherboard[MBNO].acq_delay)/25);
  delayval3 = long(float(motherboard[MBNO].SIS_delay)/25);

  delaytot = (delayval1<<pause_delay_off)|
    (delayval2<<acq_delay_off)|
    (delayval3<<SIS_delay_off);

  theXLM[XLMNUM]->addWrite32(FPGA_set_delay,delaytot);

  delayval1 = long(float(motherboard[MBNO].cycle_timeout)/25);
  delayval2 = long(float(motherboard[MBNO].global_timeout)/25);

  delaytot = (delayval1<<cycle_timeout_off)|
    (delayval2<<global_timeout_off);

  theXLM[XLMNUM]->addWrite32(FPGA_set_timeout,delaytot);

  delayval2 = long(float(motherboard[MBNO].trigger_delay)/25);
  delayval1 = long(float(motherboard[MBNO].coinc_window)/25);
  delaytot = ((delayval2<<8) | delayval1);
  //  printf("trigger_delay = %d, delay word = %lx\n",motherboard[MBNO].trigger_delay,delaytot);
  theXLM[XLMNUM]->addWrite32(FPGA_trig_delay,delaytot);

  delaytot = long(float(motherboard[MBNO].force_track_delay)/12.5);
  
  theXLM[XLMNUM]->addWrite32(FT_DELAY,delaytot);

  delaytot = long(float(motherboard[MBNO].acq_all_delay)/12.5);
  
  theXLM[XLMNUM]->addWrite32(AA_DELAY,delaytot);

  delaytot = long(float(motherboard[MBNO].glob_dis_delay)/12.5);
  
  theXLM[XLMNUM]->addWrite32(GD_DELAY,delaytot);
}

// wrapper to load the delay settings into the XLM
void LoadDelays(int MBNO)
{
  int XLMNUM = motherboard[MBNO].XLM;
  int status, bytesRead;
  long response[150];           // buffer for response from XLM
  long l;
  long* Presp =&response[0];    // set pointer to response buffer

  theXLM[XLMNUM]->clearList();
  theXLM[XLMNUM]->addWrite32(AccBus, 0x10000);  // take control of FPGA bus
  theXLM[XLMNUM]->addDelay(50);
  LoadDelays_inner(MBNO);
  theXLM[XLMNUM]->addWrite32(AccBus, 0);  // release control of memory busses
  status = theXLM[XLMNUM]->executeList(Presp, sizeof(response), &bytesRead);
}

void LoadForceRead(int MBNO)
{
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;
  int value = motherboard[MBNO].forceread;

  theXLM[XLMNUM]->AccessBus(0x10003);  // take control of memory busses

  //Wait for XLM to give up it's busses

  int i = 0;
  while (theXLM[XLMNUM]->GetOwner('A') != 0 || 
	 theXLM[XLMNUM]->GetOwner('B') != 0 ||
	 theXLM[XLMNUM]->GetOwner('X') != 0) 
    {
      i++;
      if ((i % 10000)==0) 
	{
	  cerr << "LOAD Forced Read: XLM Busses not given up for " << i << " tries\n";
	  cerr<<"XLM Bus Owner A: "<<theXLM[XLMNUM]->GetOwner('A')<<endl;
	  cerr<<"XLM Bus Owner B: "<<theXLM[XLMNUM]->GetOwner('B')<<endl;
	  cerr<<"XLM Bus Owner X: "<<theXLM[XLMNUM]->GetOwner('X')<<endl;	
	}
    }
  
  theXLM[XLMNUM]->WriteFPGA(7,0);  // turn off external Auto enable to prevent jamming XLM
  theXLM[XLMNUM]->WriteFPGA(8,0);  // turn off external Auto enable to prevent jamming XLM


    theXLM[XLMNUM]->WriteFPGA(9,1);  //Clear XLM veto latch

  // set timeouts and delays on XLM80M

  if(FBANK==BANKA)
    theXLM[XLMNUM]->WriteFPGA(FPGA_force_A,value);
  else
    theXLM[XLMNUM]->WriteFPGA(FPGA_force_B,value);
  theXLM[XLMNUM]->AccessBus(0);  // release control of memory busses

}

// the main routine to reload the configuration info into the chips
//It calls other routines (LoadXXX) to do specific parts of the job
//This routine does set the threshold DACs of the chips itself
void reload(int MBNO)
{
  int i,j,k,l,slot,pos,aword,thresh,ChipAddr;
  int status, bytesRead,serport;
  long response[15];           // buffer for response from XLM
  long* Presp =&response[0];    // set pointer to response buffer

  // step 3.  Load Threshold DACs on each HINP16 chip with values from database
  // this needs to be done first because we need to use serial string to select
  //      each chip board slot, due to defect in external address decoding
  //  We then use just bit 0 of the chip address to select which chip on the
  // selected chip board (old chips only)
  
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;
  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }
  printf("reload() XLMNUM = %d FBANK = %d serport = %d\n",XLMNUM,FBANK,serport);
  theXLM[XLMNUM]->clearList();
  theXLM[XLMNUM]->addWrite32(AccBus, 0x10000);  // take control of FPGA bus
  theXLM[XLMNUM]->addDelay(255);
  theXLM[XLMNUM]->addWrite32(FPGA_reset,0);  //Reset all XLM state machines
  theXLM[XLMNUM]->addDelay(255);

  theXLM[XLMNUM]->addWrite32(FPGA_enblA,0);  // turn off external Auto enable to prevent jamming XLM
  theXLM[XLMNUM]->addWrite32(FPGA_enblB,0);  // turn off external Auto enable to prevent jamming XLM

  /*  Clear the Veto  */

  theXLM[XLMNUM]->addWrite32(FPGA_clear_veto,1);  //Clear XLM veto latch

  // set timeouts and delays on XLM80M
  LoadDelays_inner(MBNO);

  int value = motherboard[MBNO].forceread;
  if(FBANK==BANKA)
    theXLM[XLMNUM]->addWrite32(FPGA_force_A,value);
  else
    theXLM[XLMNUM]->addWrite32(FPGA_force_B,value);
   
  LoadSerialDAC(MBNO);  // changing serial DACs wipes out rest of serial string, so that must be reloaded after
  LoadMBDAC(MBNO);      // same thing happens for motherboard DAC
  LoadMBMultSteering(MBNO);
  LoadMBOrSteering(MBNO);

  // now set motherboard slot select for current chip
  slot = chips[MBNO][SelectedChip].slot;
  pos =  chips[MBNO][SelectedChip].pos;
      //  only do this for OLD hinp chips
  pos--;   // flip even/odd of chip # to make 1,2 => 0,1
    //    printf("slot = %d SelectedChip = %d SelectedChannel = %d\n",slot,SelectedChip,SelectedChannel);
  l = 16;
  for (i=0;i<16;i++) 
    {
      if (slot == l) j=0x8000;
      else j=0;
      aword = aword >> 1;
      aword = aword | j;
      l--;
    }
  aword = aword & 0xffff;
  theXLM[XLMNUM]->addWrite32(serport, aword);
  theXLM[XLMNUM]->addDelay(serialDelay);   // delay between serial transmissions
    
  ReloadConfig(MBNO);
  theXLM[XLMNUM]->addWrite32(FBANK,ld_dacs);  // pulse CS to load multiplicity switches
  theXLM[XLMNUM]->addDelay(serialDelay);
  theXLM[XLMNUM]->addWrite32(FBANK,0);
  theXLM[XLMNUM]->addDelay(serialDelay);
    // the threshold DACs cannot be loaded until all the chips' chip_ID have been loaded
    // via the serial string by function ReloadConfig()
  for (i=1;i<NumChips[MBNO]+1;i++) 
    {
      //      printf("Loading Threshold DACs on Chip %d.\n",i);
      
      theXLM[XLMNUM]->addWrite32(FBANK,XLMout|selextbus);  // set for external control of bus
      // look up which slot this chip is in
      slot = chips[MBNO][i].slot;
      pos = i;      // for new chip, it is actual chip number
      //printf(" SLOT is %x,  POS is %x\n",slot,pos);

      for (j=0;j<=15;j++)    // loop through 16 channels/chip
	{
	  //  printf("  %d",chips[i].Threshold[j]);
	  if ((i & 1) == 1) thresh = chips[MBNO][(i+1)/2].Threshold[j]+1;
	  else thresh = chips[MBNO][(i+1)/2].Threshold[j+16]+1;
	  //	  printf("Setting thresh chip_id %d chan %d thresh %d\n",pos,j,thresh);
	  theXLM[XLMNUM]->addWrite32(FBANK,
				     XLMout | (pos<<5) | j| selextbus);  // Load chip+channel address on lowest 13 bits
	  theXLM[XLMNUM]->addWrite32(FBANK,
				    XLMout| (pos<<5| j|selextbus|dacstb));  // raise dac_strobe
	  
	  if (thresh < 0) 
	    {
	      theXLM[XLMNUM]->addWrite32(FBANK,
					XLMout|(pos<<5)|(abs(thresh))|selextbus|dacstb|
					dacsign);  //replace chip+channel with threshold
	      theXLM[XLMNUM]->addWrite32(FBANK,
					XLMout|(pos<<5)|(abs(thresh))|selextbus|
					dacsign);  //replace chip+channel with threshold
	    }
	  else 
	    {
	      theXLM[XLMNUM]->addWrite32(FBANK,
					XLMout|(pos<<5)|(abs(thresh))|selextbus|
					dacstb);  //replace chip+channel with threshold
	      theXLM[XLMNUM]->addWrite32(FBANK,
					XLMout|(pos<<5)|(abs(thresh))|
					selextbus);  //replace chip+channel with threshold
	      // but lower dacstb signal
	    }
	}
      //printf("\n");
    }


    //    theXLM[XLMNUM]->WriteFPGA(FBANK,
    //			      XLMout|
    //			      (pos<<5)|
    //			      SelectedChannel);             // place chip+channel on data bus
  if (SelectedChannel > 15) ChipAddr = (SelectedChip -1) *2 + 2;
  else ChipAddr = (SelectedChip -1) *2 + 1;
    theXLM[XLMNUM]->addWrite32(FBANK,
			      XLMout|(ChipAddr<<5)|(SelectedChannel & 15)); // place chip+channel on data bus
    theXLM[XLMNUM]->addWrite32(FBANK,
			      XLMout|(ChipAddr<<5)|(SelectedChannel & 15) | selextbus|
			      glbl_enable);   // enable drivers
    printf("selecting channel on bank %x, FPGA word %x\n",FBANK,
			      XLMout | (ChipAddr<<5)|(SelectedChannel & 15) | selextbus|
			      glbl_enable);   // enable drivers
    
    theXLM[XLMNUM]->addWrite32(AccBus,0);  // release control of memory busses
    status = theXLM[XLMNUM]->executeList(Presp, sizeof(response), &bytesRead);
}

// these SetChipxxx routines are called from the GUI (silstrip.tcl and motherboard1.tcl)
// to alter system settings.  The in-memory database is updated and the chip setting
// is also set.  ChipNo now refers to sequential Board Number.
void SetChipGain(int MBNO, int ChipNo, char* gain)
{
  //printf("setting Chip %d gain to %c\n",ChipNo,gain);
  chips[MBNO][ChipNo].gain = gain[0];
  reload(MBNO);
}

void SetChipPolarity(int MBNO, int ChipNo, char* polarity)
{
  //printf("setting polarity to %c\n",polarity);
  chips[MBNO][ChipNo].polarity = polarity[0];
  reload(MBNO);
}

void SetChipTVCRange(int MBNO, int ChipNo, char* TVCRange)
{
  //printf("setting TVCRange to %c\n",TVCRange);
  chips[MBNO][ChipNo].TVCRange = TVCRange[0];
  reload(MBNO);
}

void SetChipTest1(int MBNO, int ChipNo, char* Test1)
{
  printf("setting Test1 to %c\n",Test1[0]);
  switch (Test1[0]) {
  case 'y' : ShowSignals = 'y';
    break;
  case 'n' : ShowSignals = 'n';
    break;
  case 'e' : ShowSignals = 'e';
    break;
  default : ShowSignals = 'q';
    break;
  }
  //  printf("in SetChipTest1 mb %d chip %d Test1 = %c\n",MBNO,ChipNo,Test1);
  //  printf("ShowSignals = *%c*\n",ShowSignals);
  reload(MBNO);
}

void SetChipExtShaper(int MBNO, int ChipNo, char* ExtShaper)
{
  //printf("setting ExtShaper to %c\n",ExtShaper);
  chips[MBNO][ChipNo].ExtShaper = ExtShaper[0];
  reload(MBNO);
}

void SetChipDiscMode(int MBNO, int ChipNo, char* DiscMode)
{
  printf("setting DiscMode to %c\n",DiscMode[0]);
  chips[MBNO][ChipNo].DiscMode = DiscMode[0];
  printf("set the discmode on %d chip %d\n", MBNO, ChipNo);
  reload(MBNO);
  printf("reloaded\n");
}

void SetManyDiscMode(int MBNO, char* DiscMode, int setAll)
{
  int mbStart = MBNO;
  int mbEnd = MBNO;
  if (setAll == 1){
    mbStart = 1;
    mbEnd = num_mb;
  }

  for (int mb = mbStart; mb <= mbEnd; mb++) {
    for (int chip = 1; chip <= MaxChips; chip++) {

      //don't bother with undefined chips
      if(chips[mb][chip].slot == 0) break;

      chips[mb][chip].DiscMode = DiscMode[0];
      //printf("set %d %d to %c\n\n", mb, chip, chips[mb][chip].DiscMode);
    }    
    reload(mb);
  }
}

void SetChipDiscMask(int MBNO, int ChipNo, int Channel, short DiscMask)
{
  int temp;
  //printf("setting DiscMask for Chip %d channel %d to %d",ChipNo,Channel,DiscMask);
  temp = chips[MBNO][ChipNo].DiscMask;
  //printf(" temp %d",temp);
  temp = temp & (~ (1 << Channel) );
  //printf(" masked %d",temp);
  if (DiscMask == 1)
    chips[MBNO][ChipNo].DiscMask = temp | (1 << Channel);
  else chips[MBNO][ChipNo].DiscMask = temp;
  //printf("       now %4x\n",chips[MBNO][ChipNo].DiscMask);
  reload(MBNO);
}

void SetChipwideDiscMask(int MBNO, int ChipNo, int DiscMask)
{
  chips[MBNO][ChipNo].DiscMask = DiscMask;
  reload(MBNO);
}

void SetChipThreshold(int MBNO, int ChipNo, int Channel, char* polarity, int Threshold, int setAll)
{
  //set the thresholds of all chips with the given polarity
  if(setAll==3) {
    for (int mb = 1; mb <= num_mb; mb++) {
      for (int chip = 1; chip <= MaxChips; chip++) {	  
	if (chips[mb][chip].polarity == polarity[0]) {
	  for (int chan = 0; chan < NumChannels; chan++){
	    chips[mb][chip].Threshold[chan] = Threshold;
	  }
	}
      }
      reload(mb);
    }
  }

  //set the thresholds of all chips of given polarity on the given motherboard
  if(setAll==2) {
    for (int chip = 1; chip <= MaxChips; chip++) {	  
      if (chips[MBNO][chip].polarity == polarity[0]) {
	for (int chan = 0; chan < NumChannels; chan++){
	  chips[MBNO][chip].Threshold[chan] = Threshold;
	}
      }
    }
    reload(MBNO);
  }

  //set the thresholds of all channels on given chip
  if(setAll==1)
    {
      for(int i=0;i<NumChannels;i++)
	{
	  chips[MBNO][ChipNo].Threshold[i] = Threshold;
	}
      reload(MBNO);
    }

  //set only the theshhold of the given channel
  if(setAll==0){
    chips[MBNO][ChipNo].Threshold[Channel] = Threshold;
    reload(MBNO);
  }
}

void SetChipCSARef(int MBNO, int ChipNo, int CSARef)
{
  //printf("setting chip %d  CSARef to %d\n",ChipNo,CSARef);
  chips[MBNO][ChipNo].CSARef = CSARef;
  reload(MBNO);
}

void SetChipResCV(int MBNO, int ChipNo, int ResCV)
{
  //printf("setting chip %d  ResCV to %d\n",ChipNo,ResCV);
  chips[MBNO][ChipNo].ResetCV = ResCV;
  reload(MBNO);
}

void SetChipARef(int MBNO, int ChipNo, int ARef)
{
  //printf("setting chip %d  ARef to %d\n",ChipNo,ARef);
  chips[MBNO][ChipNo].ARef = ARef;
  reload(MBNO);
}

void SetChipZC2(int MBNO, int ChipNo, int ARef)
{
  //printf("setting chip %d  ZC2 to %d\n",ChipNo,ARef);
  chips[MBNO][ChipNo].ZC2 = ARef;
  reload(MBNO);
}

void SetChipDACRef(int MBNO, int ChipNo, int ARef)
{
  //printf("setting chip %d  DACRef to %d\n",ChipNo,ARef);
  chips[MBNO][ChipNo].DACRef = ARef;
  reload(MBNO);
}

void SetChipCFDRef(int MBNO, int ChipNo, int ARef)
{
  //printf("setting chip %d  CFDRef to %d\n",ChipNo,ARef);
  chips[MBNO][ChipNo].CFDRef = ARef;
  reload(MBNO);
}

void SetChipEOff(int MBNO, int ChipNo, int EOff)
{
  //printf("setting chip %d  EOff to %d\n",ChipNo,EOff);
  chips[MBNO][ChipNo].EOffset = EOff;
  if (chips[MBNO][ChipNo].pos == 2) chips[MBNO][ChipNo-1].EOffset = EOff;
  reload(MBNO);
}

void SetChipTOff(int MBNO, int ChipNo, int TOff)
{
  //printf("setting chip %d  TOff to %d\n",ChipNo,TOff);
  chips[MBNO][ChipNo].TOffset = TOff;
  if (chips[MBNO][ChipNo].pos == 2) chips[MBNO][ChipNo-1].TOffset = TOff;
  reload(MBNO);
}

void SetCFDCap(int MBNO, int ChipNo, int CFDCap)
{
  printf("setting chip %d  CFDCap to %d\n",ChipNo,CFDCap);
  chips[MBNO][ChipNo].CFDCap = CFDCap;
  reload(MBNO);
}

void SetChipShapOff(int MBNO, int ChipNo, int value)
{
  //printf("setting chip %d  ShapOff to %d\n",ChipNo,value);
  chips[MBNO][ChipNo].ShapOffset = value;
  reload(MBNO);
}

void SetChipCSAOff(int MBNO, int ChipNo, int value)
{
  //printf("setting chip %d  CSAOff to %d\n",ChipNo,value);
  chips[MBNO][ChipNo].CSAOffset = value;
  reload(MBNO);
}

// sets routing of OR and multiplicity for each motherboard slot
void SetSlotRouting(int MBNO, int slot, char route)
{
  //printf("setting slot %d routing to %c\n",slot,route);
  switch (route) {
  case 'O' : motherboard[MBNO].routing[slot] = 0;
    break;
  case 'A' : motherboard[MBNO].routing[slot] = 1;
    break;
  case 'B' : motherboard[MBNO].routing[slot] = 2;
    break;
  case 'C' : motherboard[MBNO].routing[slot] = 3;
    break;
   }

  reload(MBNO);
}

// sets multiplicity offsets
void SetSumOffA(int MBNO, int value)
{
  //printf("setting motherboard  SumOffA to %d\n",value);
  motherboard[MBNO].SumOffA = value;
  reload(MBNO);
}

void SetSumOffB(int MBNO, int value)
{
  //printf("setting motherboard  SumOffB to %d\n",value);
  motherboard[MBNO].SumOffB = value;
  reload(MBNO);
}

void SetSumOffC(int MBNO, int value)
{
  //printf("setting motherboard  SumOffC to %d\n",value);
  motherboard[MBNO].SumOffC = value;
  reload(MBNO);
}

void SetSISDelay(int MBNO, int value)
{
  //printf("setting motherboard  Start Delay to %d\n",value);
  motherboard[MBNO].SIS_delay = value;
  LoadDelays(MBNO);

  //reload(MBNO);
}

void SetAcqDelay(int MBNO, int value)
{
  //printf("setting motherboard  Acq Delay to %d\n",value);
  motherboard[MBNO].acq_delay = value;
  
  LoadDelays(MBNO);
  //reload(MBNO);
}

void SetPauseDelay(int MBNO, int value)
{
  motherboard[MBNO].pause_delay = value;  
  LoadDelays(MBNO);
}

void SetCycleTimeout(int MBNO, int value)
{
  motherboard[MBNO].cycle_timeout = value;  
  LoadDelays(MBNO);
}

void SetGlobalTimeout(int MBNO, int value)
{
  motherboard[MBNO].global_timeout = value;
  LoadDelays(MBNO);
}

void SetTriggerDelay(int MBNO, int value)
{
  motherboard[MBNO].trigger_delay = value;
  LoadDelays(MBNO);
}

void SetCoincWindow(int MBNO, int value)
{
  motherboard[MBNO].coinc_window = value;
  LoadDelays(MBNO);
}

void SetFTDelay(int MBNO, int value)
{
  motherboard[MBNO].force_track_delay = value;
  LoadDelays(MBNO);
}

void SetAADelay(int MBNO, int value)
{
  motherboard[MBNO].acq_all_delay = value;
  LoadDelays(MBNO);
}

void SetGDDelay(int MBNO, int value)
{
  motherboard[MBNO].glob_dis_delay = value;
  LoadDelays(MBNO);
}

void SetForceRead(int MBNO, int value)
{
  motherboard[MBNO].forceread = value;
  LoadForceRead(MBNO);
}

/* Select a MOTHERBOARD */
void SelectBoard(int MBNO)
{
  SelectedBoard = MBNO;
  //reload(MBNO);
  return;
}

// selects a chip BOARD
void SelectChip(int MBNO, int chipNo) 
{
  //  int g;
  //  g = 0;
  //printf("Selecting chipboard %d\n",chipNo);
  SelectedChip = chipNo;
  //reload(MBNO);
  return;
}

/* Select a channel */
void SelectChannel(int MBNO, int chan) 
{
  //  int g;
  //  g = 0;
  //printf("Selecting channel %d\n",chan);
  SelectedChannel = chan;
  //reload(MBNO);
  return;
}

// loads the serial DACs on each chip board
// the serial DACs come after the HINP chips on each chip board, so additional bits have to
// be sent out to move the DAC data to the right place in the string
//  both old and new chip boards have 112 total shift register bits 
//  boards with one HINP chip and one DAC would have 96 SR bits, these are no longer supported
//             by the database.
void LoadSerialDAC(int MBNO) 
{
  int i,j,k,m,m2,m3,n,ival;
  int regval,serport;
  unsigned int l;

  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;

  //  printf(" Load serial DAC, shift reg len = %d\n",ShiftRegLen[MBNO]);
  k=NumBoards[MBNO];  // number of chip boards needs to be determined by new motherboard
  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }

  for (m2=1; m2 <= k; m2++) 
    {
      //        printf("Loading Serial DAC on Chip Board %d.\n",m2);
      theXLM[XLMNUM]->addWrite32(FBANK,0);
      //cout<<"***********"<<m2<<" "<<chips[m2].pos<<endl;
      
      /*
	if (chips[m2].pos == 1) // associate chip boards with chip numbers
	m = m2;
	else
	m = m2+1;
      */
      m=2*m2-1;	
      m3 = m/2+1;  // convert from chip sequence to board sequence
      //printf(" m %d m2 %d m3 %d\n",m,m2,m3);
      for (n=1;n<9;n++) 
	{
	  /* put out one chip board's worth of zeroes to prevent old data from
	     altering contents of other DACs */
	  for (j=1;j<17;j++)              // uninit variable was here!
	    {
	      if (motherboard[MBNO].config[j] == 1) 
		{
		  //	    printf("padding ahead with 64 zeroes for slot %d\n",j);
		  for (i=0;i<4;i++) {
		    theXLM[XLMNUM]->addWrite32(serport, 0);  // roll bits thru one HINP16 chip & DAC
		    theXLM[XLMNUM]->addDelay(serialDelay);
		  }
		}
	      if (motherboard[MBNO].config[j] == 2) 
		{
		  //	  	  printf("padding ahead with 112 zeroes for slot %d\n",j);
		  for (i=0;i<7;i++) {
		    theXLM[XLMNUM]->addWrite32(serport, 0);  // roll bits thru 2 HINP16 chips & DAC
		    theXLM[XLMNUM]->addDelay(serialDelay);
		  }
		}
	    }
	  for (i=1;i<11;i++) {
	    theXLM[XLMNUM]->addWrite32(serport, 0);  // send zeros to MB
	    theXLM[XLMNUM]->addDelay(serialDelay);
	  }
	  if (chips[MBNO][m3].New == FALSE) {              // Old board DAC layout
	    //    printf("selected old DAC layout for board %d m= %d\n",m2,m3);
	    switch (n) 
	      {
	      case 1 : regval = chips[MBNO][m3].CSARef;
		break;
	      case 2 : regval = chips[MBNO][m3].ResetCV;
		break;
	      case 3 : regval = chips[MBNO][m3].ARef;
		break;
	      case 4 : regval = chips[MBNO][m3].EOffset;
		break;
	      case 5 : regval = chips[MBNO][m3].CSARef;
		break;
	      case 6 : regval = chips[MBNO][m3].ResetCV;
		break;
	      case 7 : regval = chips[MBNO][m3].ARef;
		break;
	      case 8 : regval = chips[MBNO][m3].TOffset;
	      }  
	  } else {  // new board DAC layout
	    //printf("selected new DAC layout for board %d m= %d\n",m2,m3);
	    switch (n) 
	      {
	      case 1 : regval = chips[MBNO][m3].CSARef;
		break;
	      case 2 : regval = chips[MBNO][m3].ResetCV;
		break;
	      case 3 : regval = chips[MBNO][m3].ARef;
		break;
	      case 4 : regval = chips[MBNO][m3].EOffset;
		break;
	      case 5 : regval = chips[MBNO][m3].ZC2;
		break;
	      case 6 : regval = chips[MBNO][m3].DACRef;
		break;
	      case 7 : regval = chips[MBNO][m3].CFDRef;
		break;
	      case 8 : regval = chips[MBNO][m3].TOffset;
	      }  
	  }	    
	  //  printf("     DAC # %d set to %d\n",n,regval);
	  l=8;       // send the channel address to the DAC as 4 serial bits, MSB first
	  ival = 0;
	  for (i=0;i<4;i++) 
	    {
	      if ((n & l) != 0) j=0x8000;
	      else j=0;
	      ival = ival >> 1;
	      ival = ival | j;
	      l = l >> 1;
	    }
	  
	  l=512;       // send the DAC value to the DAC as 10 serial bits, MSB first
	  for (i=0;i<10;i++) 
	    {
	      if ((regval & l) != 0) j=0x8000;
	      else j=0;
	      ival = ival >> 1;
	      ival = ival | j;
	      l = l >> 1;
	    }
	  ival = ival >> 2;  // send 2 zero bits to the DAC to supply the required 16 total bits
	  ival = ival & 0xffff;
	  theXLM[XLMNUM]->addWrite32(serport, ival);  // send serial bits toward DAC
	  theXLM[XLMNUM]->addDelay(serialDelay);
	  regval=chips[MBNO][m2].slot;           // find slot this board is in
	  //printf("board slot now is %d board %d\n",regval, m2);
	  j=regval;
	  //printf("checking for padding zeroes for slot %d\n",j);
	  //printf("config[%d] has been set to %d\n",j,motherboard[MBNO].config[j]);
	  if (motherboard[MBNO].config[j] == 1) 
	    {
	      //  	printf("padding behind with 48 zeroes for slot %d\n",j);
	      for (i=0;i<3;i++) {
		theXLM[XLMNUM]->addWrite32(serport, 0);  // roll bits thru 1 HINP16 to DAC
		theXLM[XLMNUM]->addDelay(serialDelay);
	      }
	    }
	  if (motherboard[MBNO].config[j] == 2) {
	    // 	printf("padding behind with 96 zeroes for slot %d\n",j);
	    for (i=0;i<6;i++) {
		theXLM[XLMNUM]->addWrite32(serport, 0);  // roll bits thru 2 HINP16 to DAC
		theXLM[XLMNUM]->addDelay(serialDelay);
	    }
	  }
	  for (j=regval-1;j>0;--j) 
	    {
	      //printf("checking for padding zeroes for slot %d\n",j);
	      //printf("config[%d] has been set to %d\n",j,motherboard[MBNO].config[j]);
	      if (motherboard[MBNO].config[j] == 1) 
		{
		  //  	  printf("padding behind with 64 zeroes for slot %d\n",j);
		  for (i=0;i<4;i++) {
		theXLM[XLMNUM]->addWrite32(serport, 0);  // roll bits thru 1 HINP16 and DAC
		theXLM[XLMNUM]->addDelay(serialDelay);
		  }
		}
	      if (motherboard[MBNO].config[j] == 2) 
		{
		  //  printf("padding behind with 112 zeroes for slot %d\n",j);
		  for (i=0;i<7;i++) {
		theXLM[XLMNUM]->addWrite32(serport, 0);  // roll bits thru 2 HINP16 and DAC
		theXLM[XLMNUM]->addDelay(serialDelay);
		  }
		}
	    }
	  // Raise dacsign, which is also ser DAC CS/LD, to load the actual DAC
	  //printf("loading the DAC\n");
	  theXLM[XLMNUM]->addWrite32(FBANK,dacsign);
	  theXLM[XLMNUM]->addWrite32(FBANK,0);
	}
    }
}

// loads the serial DACs on the motherboard
//This DAC is after the FPGA on the motherboard, but before the multiplicity switches.
void LoadMBDAC(int MBNO) 
{
  int i,j,k,m,n,ival,jval,serport;
  unsigned int l;
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;

  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }
  //printf(" Load MB DAC, shift reg len = %d\n",ShiftRegLen[MBNO]);
  k=NumBoards[MBNO];  // number of chip boards needs to be determined by new motherboard
  {
      //printf("Loading Serial DAC on MotherBoard %d.\n",m);
    theXLM[XLMNUM]->addWrite32(FBANK,0);  // clear dac sign which is the LD/Shift ctrl
    for (n=1;n<6;n++) 
      {
	
	switch (n) 
	  {
	  case 1 : ival = motherboard[MBNO].SumOffA;
	    break;
	  case 2 : ival = motherboard[MBNO].SumOffB;
	    break;
	  case 3 : ival = motherboard[MBNO].SumOffC;
	    break;
	  case 4 : ival = 1023 - chips[MBNO][SelectedChip].ShapOffset;
	    break;
	  case 5 : ival = 1023 - chips[MBNO][SelectedChip].CSAOffset;
	    break;
	  }
	
	//printf("     DAC # %d set to %d\n",n,ival);
	l=8;       // send the channel address to the DAC as 4 serial bits, MSB first
	
	for (i=0;i<4;i++) 
	  {
	    if ((n & l) != 0) j=0x8000;
	    else j=0;
	    jval = jval >> 1;
	    jval = jval | j;
	    //printf("%1d",j);
	    l = l >> 1;
	  }
	l=512;       // send the DAC value to the DAC as 10 serial bits, MSB first
	
	for (i=0;i<10;i++) 
	  {
	    if ((ival & l) != 0) j=0x8000;
	      else j=0;
	    jval = jval >> 1;
	    jval = jval | j;
	    //printf("%1d",j);
	    l = l >> 1;
	  }
	
	jval = jval >> 2;  // send 2 zero bits to the DAC to supply the required 16 total bits
	jval = jval & 0x3fff;
	theXLM[XLMNUM]->addWrite32(serport, jval);  // send serial bits toward DAC
	theXLM[XLMNUM]->addDelay(serialDelay);
	j = (ShiftRegLen[MBNO] - 16) / 16;  // compute number of bits to shift out to reach the DAC
	//      printf("padding in with %d zeroes for DAC.\n",j);
	for (i=0;i<j;i++) {
	  theXLM[XLMNUM]->addWrite32(serport, jval);  // roll bits thru to end of shift reg
	  theXLM[XLMNUM]->addDelay(serialDelay);
	}
	// Raise dacsign, which is also ser DAC CS/LD, to load the actual DAC
	theXLM[XLMNUM]->addWrite32(FBANK,ld_dacs);
	theXLM[XLMNUM]->addDelay(serialDelay);
	theXLM[XLMNUM]->addWrite32(FBANK,0);
	theXLM[XLMNUM]->addDelay(serialDelay);	
      }
  }
}

// loads the serial MOS Switches for Multiplicity steering on the motherboard
//These switches are at the very tail end of the serial string
void LoadMBMultSteering(int MBNO) 
{
  int i,j,k,m,n,ival,serport;
  unsigned int l;

  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;

  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }
  theXLM[XLMNUM]->addWrite32(FBANK,0); // make sure dac_sign and ld_dacs are low
  theXLM[XLMNUM]->addWrite32(serport, 0);  // prevent altering MB DAC setting
  theXLM[XLMNUM]->addDelay(serialDelay);
  //printf(" Load MB Mult Steering, shift reg len = %d\n",ShiftRegLen[MBNO]);
  for (m=1; m <= 1; m++) {
    //printf("Loading Serial Switches on MotherBoard %d.\n",m);
    ival=0;
    for (n=16;n>8;--n) 
      {
	ival = ival >> 1;
	if (motherboard[m].routing[n] == 3)     // SUM C output
	  ival = ival | 0x8000;
      }
    for (n=16;n>8;--n) 
      {
	ival = ival >> 1;
	if (motherboard[MBNO].routing[n] == 2)    // Sum B output
	  ival = ival | 0x8000;
      }
    ival = ival & 0xffff;
    theXLM[XLMNUM]->addWrite32(serport, ival);
    theXLM[XLMNUM]->addDelay(serialDelay);
    for (n=16;n>8;--n) 
      {
	ival = ival >> 1;
	if (motherboard[MBNO].routing[n] == 1)     // Sum A output
	  ival = ival | 0x8000;
      }
    for (n=8;n>0;--n) 
      {
	ival = ival >> 1;
	if (motherboard[MBNO].routing[n] == 3)    // Sum C output
	  ival = ival | 0x8000;
      }
    ival = ival & 0xffff;
    theXLM[XLMNUM]->addWrite32(serport, ival);
    theXLM[XLMNUM]->addDelay(serialDelay);
    for (n=8;n>0;--n) 
      {
	ival = ival >> 1;
	if (motherboard[MBNO].routing[n] == 2)    // Sum B output
	  ival = ival | 0x8000;
      }
    for (n=8;n>0;--n) 
      {
	ival = ival >> 1;
	if (motherboard[MBNO].routing[n] == 1)    // Sum A output
	  ival = ival | 0x8000;
      }
    ival = ival & 0xffff;
    theXLM[XLMNUM]->addWrite32(serport, ival);
    theXLM[XLMNUM]->addDelay(serialDelay);
  }
}

// loads the serial selectors for OR steering on the motherboard FPGA
void LoadMBOrSteering(int MBNO) 
{
  int i,j,k,m,n,ival,serport;
  unsigned int l;
  
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;

  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }
  //printf(" Load MB Or Steering, shift reg len = %d\n",ShiftRegLen[MBNO]);
  // skip past MB FPGA option reg (16 bits) and present reg (16 bits)
  for (m=0;m<2;m++) {
    theXLM[XLMNUM]->addWrite32(serport, 0);
    theXLM[XLMNUM]->addDelay(serialDelay);
  }
  //for (m=1; m <= 1; m++) 
    {
      //printf("Loading OR Routing on MotherBoard %d.\n",m);
      i = 0;
      for (n=16;n>0;--n) 
	{
	  for (j=3;j>0;--j) 
	    {
	      ival = ival >> 1;
	      if (motherboard[MBNO].routing[n] == j)     // OR (C,B,A) output
		ival = ival | 0x8000;
	      i++;
	      if (i > 15) {
		ival = ival & 0xffff;
		theXLM[XLMNUM]->addWrite32(serport, ival);
		theXLM[XLMNUM]->addDelay(serialDelay);
		i = 0;
	      }
	    }
	}
    }
}

/* Reload the HINP chip configuration registers
    Each HINP chip has a 48-bit configuration register
The boards have the two 48-bit strings wired in series, with the DAC coming after
  The bits are as follows :
0 - 15             1 = disable CFD for channel
16 - 31            unused
32                 1 = Negative pulses at CSA Input
33                 1 = Short TVC time range
34                 1 = CSA low-gain mode
35                 1 = linear signal test output On
36                 1 = External preamp
37                 1 = override peak sampling circuit
38 - 39            unused
  40 - 47            Chip ID number, bit 47 is MSB
*/
void ReloadConfig(int MBNO) 
{
  int Setup, ChannelMask;
  int i,j,k,m,n,ival,serport;
  unsigned int l;
  
  int XLMNUM = motherboard[MBNO].XLM;
  int FBANK = motherboard[MBNO].bank;
  if (FBANK == BANKA) {
    serport = FAST_SERA;
      } else {
    serport = FAST_SERB;
  }

  k=NumChips[MBNO];  // figure out how many chips are actually present

  printf("determined %d chip(s) are in Shift Reg String.\n",k);
  // step 2. load config register with calculated value

  for (m=k; m > 0 ; --m) 
    {
      if ((m & 1) == 0) // detect first chip on each board to be loaded
	{  
	  //	  printf("output 16 zeroes for DAC\n");
	  theXLM[XLMNUM]->addWrite32(serport, 0);  // send 15 zeros so DAC won't be altered
	  theXLM[XLMNUM]->addDelay(serialDelay);
	}
      //      printf("For chip %d ShowSignals =%c\n",m,ShowSignals);
      if (ShowSignals=='e') {
	ChannelMask = 0x0000;
	if ((MBNO == SelectedBoard) && (m == SelectedChip*2) && SelectedChannel > 15 ) {
	  ChannelMask = 1 << SelectedChannel - 16;
	  //	printf("for secondchip %d ChannelMask = %x\n",m, ChannelMask);
	}
	if ((MBNO == SelectedBoard) && (m == SelectedChip*2-1) && SelectedChannel < 16 ) {
	  ChannelMask = 1 << SelectedChannel;
	  //	printf("for first chip %d ChannelMask = %x\n",m, ChannelMask);
	}
	//	printf("for chip %d ChannelMask = %x\n",m, ChannelMask);
      }
      else {
	//	printf("DiscMode for chip %d is %c\n",m,chips[MBNO][(m+1)/2].DiscMode);
	switch (chips[MBNO][(m+1)/2].DiscMode) 
	  {
	  case 'a' : ChannelMask = 0xffff;
	    break;
	  case 'n' : ChannelMask = 0x0000;
	    break;
	  case 's' :
	    if ((m & 1) == 1) {
	      if (SelectedChannel <= 15) {
		ChannelMask = 1 << SelectedChannel;
	      }
	      else {
		ChannelMask = 0;
	      }
	      //	      printf("selected odd chip %d ChannelMask %x\n",m,ChannelMask);
	    }
	    else {
	      if (SelectedChannel >= 16) {
		ChannelMask = 1 << SelectedChannel-16;
	      }
	      else {
		ChannelMask = 0;
	      }
	      //	      printf("selected even chip %d ChannelMask %x\n",m,ChannelMask);
	    }
	    break;
	  case 'o' : ChannelMask = 0xaaaa;
	    break;
	  case 'e' : ChannelMask = 0x5555;
	    break;
	  case 'm' : if ((m & 1) == 1) {
	      ChannelMask = chips[MBNO][(m+1)/2].DiscMask;
	    } else
	      {
		ChannelMask = chips[MBNO][(m+1)/2].DiscMask >> 16;
	      }
	    break;
	  }
      }
      ival = 0;    // fix uninitialized variable !!
      l=128;                 // send chip ID, msb first      
      for (i=0;i<8;i++) 
	{
	  ival = ival >> 1;
	  if ((m & l) != 0) j= 0x8000;  // m counts down from n..1 through chip addresses
	  else j=0;
	  ival = ival | j;
	  l = l >> 1;
	}
      //      printf("chip %d ID = 0x%x\n",m,ival);
      Setup = 0;               // build misc setup bits
      printf("chips[MBNO][(m+1)/2].New = %d when m = %d\n",chips[MBNO][(m+1)/2].New,m);
      if (chips[MBNO][(m+1)/2].polarity  == 'n') Setup = Setup | 1;
      if (chips[MBNO][(m+1)/2].TVCRange  == 's') Setup = Setup | 2;
      if (chips[MBNO][(m+1)/2].gain      == 'l') Setup = Setup | 4;
      if (((m & 1) == 1 && (SelectedChannel < 16)) || ((m & 1) == 0 && (SelectedChannel > 15))) {
	//	printf("m = %d SelectedChip %d\n",m,SelectedChip);
	if ((MBNO == SelectedBoard) && ((m+1)/2 == SelectedChip) && (ShowSignals == 'y' || ShowSignals == 'e' && chips[MBNO][(m+1)/2].New == TRUE)) Setup = Setup | 64; // test mode 4 for shaper output, new chips ONLY
	if ((MBNO == SelectedBoard) && ((m+1)/2 == SelectedChip) && (ShowSignals == 'y' || ShowSignals == 'e')) Setup = Setup | 8;
      }
      if (chips[MBNO][(m+1)/2].gain      == 'e') Setup = Setup | 16;
      if (chips[MBNO][(m+1)/2].ExtShaper == 'e') Setup = Setup | 32;
      if (chips[MBNO][(m+1)/2].CFDCap  != 5 && chips[MBNO][(m+1)/2].New == TRUE) Setup = Setup | 128;     // TURN ON 12 pF cap in fast shaper, NEW CHIP ONLY
      printf("chip %d misc setup byte = 0x%x\n",m,Setup);
      l=128;                 // send misc setup bits
      for (i=0;i<8;i++) 
	{
	  ival = ival >> 1;
	  if ((Setup & l) != 0) j= 0x8000;
	  else j=0;
	  ival = ival | j;
	  l = l >> 1;
	}
      ival = ival & 0xffff;
      //      printf("chip %d ID and Setup Byte = 0x%x\n",m,ival);
      theXLM[XLMNUM]->addWrite32(serport, ival);  // send misc setup bits
      theXLM[XLMNUM]->addDelay(serialDelay);
      theXLM[XLMNUM]->addWrite32(serport, 0xffff);  // always mask unused channels
      theXLM[XLMNUM]->addDelay(serialDelay);
      ival = 0;    // fix uninitialized variable !!
      l=32768;
      for (i=0;i<16;i++) 
	{
	  ival = ival >> 1;
	  if ((ChannelMask & l) != 0) j= 0;  // note polarity gets inverted here!
	  else j=0x8000;
	  ival = ival | j;
	  l = l >> 1;
	}
      //      printf("chip %d Channel Mask = 0x%x\n",m,ival);
      theXLM[XLMNUM]->addWrite32(serport, ival);  // send real channel mask
      theXLM[XLMNUM]->addDelay(serialDelay);
    }
  // Raise ld_dacs, for MAX335  CS/LD, to load the shift reg into the switches
  // now that the bits are moved to the right place
  theXLM[XLMNUM]->addWrite32(FBANK,ld_dacs);
  theXLM[XLMNUM]->addWrite32(FBANK,0);
  
  theXLM[XLMNUM]->addWrite32(FBANK,XLMout);
  theXLM[XLMNUM]->addWrite32(FBANK,XLMout|
			    (SelectedChip<<5)| SelectedChannel| selextbus| glbl_enable);
  return;
}

// loads database from file
// note that motherboard settings now come first, so that if less boards are in system
// than file has, the MB settings will take effect.
void LoadFile(char* filename) {
  int i,j,k,l,m,n;
  char abuf[64],bbuf[64],cbuf[64];
  char* a = &abuf[0];
  char* b = &bbuf[0];
  char* c = &cbuf[0];
  dbfile = fopen(filename,"r");
  if (dbfile == NULL)
    printf("file could not be opened for reading.\n");
  else 
    {
      int charsRead = charsRead = fscanf(dbfile," ");    // gobble something at beginning ?
      for (m=1;m<num_mb+1;m++)
	{
	  if (feof(dbfile)) break;
	  for (j=1;j<=16;j++) 
	    {
	      charsRead = charsRead = fscanf(dbfile," %s %d %s %d %s %d\n",a,&k,b,&l,c,&motherboard[m].routing[j]);
	      printf("MB %d slot %d routing is %d\n",m,j,motherboard[m].routing[j]);
	    }
	  if (feof(dbfile)) break;
	  charsRead = charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].SumOffA);
	  printf("MB %d SumOffA is %d\n",m,motherboard[m].SumOffA);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].SumOffB);
	  printf("MB %d SumOffB is %d\n",m,motherboard[m].SumOffB);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].SumOffC);
	  printf("MB %d SumOffC is %d\n",m,motherboard[m].SumOffC);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].SIS_delay);
	  printf("MB %d Start Delay is %d\n",m,motherboard[m].SIS_delay);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].acq_delay);
	  printf("MB %d Acq Delay is %d\n",m,motherboard[m].acq_delay);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].pause_delay);
	  printf("MB %d Hold Delay is %d\n",m,motherboard[m].pause_delay);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].cycle_timeout);
	  printf("MB %d Cycle Timeout is %d\n",m,motherboard[m].cycle_timeout);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].global_timeout);
	  printf("MB %d Global Timeout is %d\n",m,motherboard[m].global_timeout);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].trigger_delay);
	  printf("MB %d Trigger Delay is %d\n",m,motherboard[m].trigger_delay);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].coinc_window);
	  printf("MB %d Coincidence Window is %d\n",m,motherboard[m].coinc_window);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].force_track_delay);
	  printf("MB %d Force Track Delay is %d\n",m,motherboard[m].force_track_delay);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].acq_all_delay);
	  printf("MB %d Acquisition All Delay is %d\n",m,motherboard[m].acq_all_delay);
	  charsRead = fscanf(dbfile," %s %d %s %d\n",a,&k,b,&motherboard[m].glob_dis_delay);
	  printf("MB %d Global Disable Delay is %d\n",m,motherboard[m].glob_dis_delay);
	  motherboard[m].forceread = 0;
	  //	  charsRead = fscanf(dbfile," ");    // header
	  for (i=1;i<NumChips[m]+1;i++) 
	    {
	      printf("Reading Motherboard %d Chip %d",m,i);
	      charsRead = fscanf(dbfile,"%s %d\n",a,&k);
	      printf("  chip number is %s %d\n",a,k);
	      charsRead = fscanf(dbfile," %s %c\n",a,&chips[m][i].gain);
	      printf("gain is %s %c\n",a,chips[m][i].gain);
	      charsRead = fscanf(dbfile," %s %c\n",a,&chips[m][i].polarity);
	      printf("polarity is %s %c\n",a,chips[m][i].polarity);
	      charsRead = fscanf(dbfile," %s %c\n",a,&chips[m][i].TVCRange);
	      printf("TVCRange is %c\n",chips[m][i].TVCRange);
	      charsRead = fscanf(dbfile," %s %c\n",a,b);
	      //	      printf("Test1 is %c\n",&chips[m][i].Test1);
	      charsRead = fscanf(dbfile," %s %c\n",a,&chips[m][i].ExtShaper);
	      printf("Shaper is %c\n",chips[m][i].ExtShaper);
	      charsRead = fscanf(dbfile," %s %c\n",a,&chips[m][i].DiscMode);
	      printf("Disc Mode is %c\n",chips[m][i].DiscMode);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].DiscMask);
	      printf("DiskMask is %d\n",chips[m][i].DiscMask);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].CSARef);
	      printf("CSARef is %d\n",chips[m][i].CSARef);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].ResetCV);
	      printf("ResetCV is %d\n",chips[m][i].ResetCV);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].ARef);
	      printf("ARef is %d\n",chips[m][i].ARef);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].ZC2);
	      printf("ZC2 is %d\n",chips[m][i].ZC2);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].DACRef);
	      printf("DACRef is %d\n",chips[m][i].DACRef);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].CFDRef);
	      printf("CFDRef is %d\n",chips[m][i].CFDRef);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].EOffset);
	      printf("EOffset is %d\n",chips[m][i].EOffset);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].TOffset);
	      printf("TOffset is %d\n",chips[m][i].TOffset);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].CFDCap);
	      printf("CFDCap  is %d\n",chips[m][i].CFDCap);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].CSAOffset);
	      printf("TOffset is %d\n",chips[m][i].CSAOffset);
	      charsRead = fscanf(dbfile," %s %d\n",a,&chips[m][i].ShapOffset);
	      printf("TOffset is %d\n",chips[m][i].ShapOffset);
	      for (j=0;j<NumChannels;j++) 
		{
		  charsRead = fscanf(dbfile," %s %d %d\n",a,&k,&chips[m][i].Threshold[j]);
		  //		  printf("Threshold %d is %d\n",j,chips[m][i].Threshold[j]);
		}
	    }
	}
      fclose(dbfile);
    }
  for(i=1;i<num_mb+1;i++) {
    reload(i);
    LoadDelays(i);
  }
}

// saves database to file
void SaveFile(char* filename) 
{
  int i,j,m;
  dbfile = fopen(filename,"w");
  if (dbfile != NULL) 
    {
      rewind(dbfile);
    }
  if (dbfile == NULL)
    printf("file could not be created\n");
  else 
    {
      for(m=1;m<num_mb+1;m++)
	{
	  // now save the motherboard data
	  for (j=1;j<=16;j++)
	    fprintf(dbfile,"Routing_MB %d Slot %d is %d\n",m,j,motherboard[m].routing[j]);
	  fprintf(dbfile,"MB %d SumOffA %d\n",m,motherboard[m].SumOffA);
	  fprintf(dbfile,"MB %d SumOffB %d\n",m,motherboard[m].SumOffB);
	  fprintf(dbfile,"MB %d SumOffC %d\n",m,motherboard[m].SumOffC);
	  fprintf(dbfile,"MB %d SISDelay %d\n",m,motherboard[m].SIS_delay);
	  fprintf(dbfile,"MB %d AcqDelay %d\n",m,motherboard[m].acq_delay);
	  fprintf(dbfile,"MB %d PauseDelay %d\n",m,motherboard[m].pause_delay);
	  fprintf(dbfile,"MB %d CycleTimeout %d\n",m,motherboard[m].cycle_timeout);
	  fprintf(dbfile,"MB %d GlobalTimeout %d\n",m,motherboard[m].global_timeout);
	  fprintf(dbfile,"MB %d TriggerDelay %d\n",m,motherboard[m].trigger_delay);
	  fprintf(dbfile,"MB %d CoincidenceWindow %d\n",m,motherboard[m].coinc_window);
	  fprintf(dbfile,"MB %d ForceTrackDelay %d\n",m,motherboard[m].force_track_delay);
	  fprintf(dbfile,"MB %d AcqAllDelay %d\n",m,motherboard[m].acq_all_delay);
	  fprintf(dbfile,"MB %d GlobDisDelay %d\n",m,motherboard[m].glob_dis_delay);
	  for (i=1;i<NumChips[m]+1;i++) 
	    {
	      fprintf(dbfile,"Chip %d\n",i);
	      fprintf(dbfile,"    Gain %c\n",chips[m][i].gain);
	      fprintf(dbfile,"    Polarity %c\n",chips[m][i].polarity);
	      fprintf(dbfile,"    TVCRange %c\n",chips[m][i].TVCRange);
	      fprintf(dbfile,"    Test1 %c\n",'n');
	      fprintf(dbfile,"    Shaper %c\n",chips[m][i].ExtShaper);
	      fprintf(dbfile,"    DiscMode %c\n",chips[m][i].DiscMode);
	      fprintf(dbfile,"    DiscMask %d\n",chips[m][i].DiscMask);
	      fprintf(dbfile,"    CSARef %d\n",chips[m][i].CSARef);
	      fprintf(dbfile,"    ResetCV %d\n",chips[m][i].ResetCV);      
	      fprintf(dbfile,"    ARef %d\n",chips[m][i].ARef);
	      fprintf(dbfile,"    ZC2 %d\n",chips[m][i].ZC2);
	      fprintf(dbfile,"    DACRef %d\n",chips[m][i].DACRef);
	      fprintf(dbfile,"    CFDRef %d\n",chips[m][i].CFDRef);
	      fprintf(dbfile,"    EOffset %d\n",chips[m][i].EOffset);
	      fprintf(dbfile,"    TOffset %d\n",chips[m][i].TOffset);
	      fprintf(dbfile,"    CFDCap %d\n",chips[m][i].CFDCap);
	      fprintf(dbfile,"    CSAOffset %d\n",chips[m][i].CSAOffset);
	      fprintf(dbfile,"    ShapOffset %d\n",chips[m][i].ShapOffset);
	      for (j=0;j<NumChannels;j++) 
		{
		  fprintf(dbfile,"    Threshold %d %d\n",j,chips[m][i].Threshold[j]);
		}
	    }
	}
    }
  fclose(dbfile);
}


/*  change the threshold of specified channels by a certain amount, regardless
    of original threshold
*/

void LoadIncr(char* filename) {
  
  //Load an increment data file of format:
  //
  // mb slot chip strip increment
  // $mbnum $slotnum $chipnum $stripnum $incrementval
  // more of the same
  //
  // Increment values are relative to the polarity of the chip: 
  // with a postive increment, positive thresholds get more positive and
  // negative get more negative

  char line[100];
  int mb, slot, chip, strip, increment;

  ifstream file(filename,ios::in);
  if ( !file.good() ) {
    cout<<"Could not open increment file for reading\n";
  }
  else {

    file.getline(line,100);  //gobble header
    
    while (!file.eof()){

      //get data and test for trailing newline
      int mbtest=-1;
      file>>mbtest>>slot>>chip>>strip>>increment;
      if (mbtest == -1) break;
      mb = mbtest;

      //have chip information in terms of location, search for index
      int chipIndex = -1;
      for (int j=1; j<=NumChips[mb]; j++){
	if ( chips[mb][j].slot == slot && chips[mb][j].pos == chip ){
	  chipIndex = j;
	  break;
	}
      }
      if (chipIndex == -1) {
	printf("Motherboard %d slot %d chip %d not found to increment\n", 
	       mb, slot, chip);
      }
      else {
	int tsign = 1;
	if (chips[mb][chipIndex].polarity == 'p') tsign = -1;
	increment *= tsign;
	
	int oldThres = chips[mb][chipIndex].Threshold[strip];
	int newThres = oldThres + increment;
	if (newThres*tsign > 32) newThres = tsign*32;
	if (newThres*tsign < -31) newThres = -31;
	chips[mb][chipIndex].Threshold[strip] = newThres;
      }
    }
  }
  
  file.close();

  for(int i=1;i<num_mb+1;i++)
    reload(i); 

}

void LoadFPGA (char* pFilename, int XLMCrate, int XLMSlot, int XLMType)
{
  int XLMNUM = motherboard[1].XLM;
  printf("LoadFPGA filename is '%s' crate %d slot %d type %d\n",
	 pFilename,XLMCrate,XLMSlot,XLMType);
  printf("XLMNUM = %d\n",XLMNUM);
  theXLM[XLMNUM]->loadFirmware(pFilename);
}
