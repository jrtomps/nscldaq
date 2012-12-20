#define HIRA  0x3100
#define VMEADC  0xADC0
#define VMETDC  0xADC1
#define VMEQDC  0xADC2
#define VMEIGQDC  0xADC3
#define VMEFADC  0xFADC
#define VMEXLM  0xEFEF
#define TOWER_1  0xA001
#define TOWER_2  0xA002
#define TOWER_3  0xA003
#define TOWER_4  0xA004
#define TOWER_5  0xA005

//#define FALSE 0

/* Modifications for XLM */
#define acqall       0x10000

#define glbl_enable  0x40000
#define serin        0x80000
#define serclk       0x100000
#define tokenin      0x200000            // active low, inverted in FPGA
#define forcereset   0x400000
#define dacstb       0x800000
#define dacsign      0x1000000
#define selextbus    0x2000000
#define ld_dacs      0x4000000
#define vetoreset    0x8000000
#define force_track  0x10000000
#define XLMout       0x80000000

/************* Inputs  ***************/

#define ser_busy      0x80000000  // for XLM XXV or dual-port XLM 80M
// #define ser_busy      0x2000  // for old XLM 80M
#define token_out   0x4000
#define serout      0x8000
#define acqack      0x10000
#define orout       0x20000

/********* addresses in XLM *********/
#define SRAMA         0x0
#define SRAMB         0x200000
#define FPGA          0x400000
#define AccBus        0x800000 // don't cause name collision

/********* register addresses in XLM FPGA *********/
#define FPGA             0x400000          // base address of FPGA range
#define FPGA_reset       FPGA+0x00*4       // write to this reg resets XLM
#define FPGA_acq_a       FPGA+0x01*4      // write starts acq cycle on A bus
#define FPGA_acq_b       FPGA+0x02*4       // write starts acq cycle on B bus
#define FPGA_set_delay   FPGA+0x03*4       // set delay timings
#define FPGA_set_timeout FPGA+0x04*4       // set timeout counters
#define FPGA_ABus        FPGA+0x05*4       // read or write to A bus bits
#define FPGA_BBus        FPGA+0x06*4       // read or write to B bus bits
#define FPGA_enblA       FPGA+0x07*4       // External Enable for Bus A
#define FPGA_enblB       FPGA+0x08*4       // External Enable for Bus B
#define FPGA_clear_veto  FPGA+0x09*4       // Strobes glbl_enbl veto clear
#define FPGA_trig_delay  FPGA+0x0a*4       // Sets the trigger delay
#define FPGA_coin_window FPGA+0x0b*4       // Sets width of coincidence window
#define FPGA_force_A     FPGA+0x0c*4       // Force Readout for Bank A
#define FPGA_force_B     FPGA+0x0d*4        // Force Read for Bank B
#define FT_DELAY         FPGA+0x0e*4        // Force Track Delay Register
#define AA_DELAY         FPGA+0x0f*4        // ACQ_ALL DELAY REGISTER
#define GD_DELAY         FPGA+0x10*4        // GLOBAL DISABLE DELAY REGISTER
#define FAST_SERA        FPGA+0x11*4        // fast serial A (16 bits at a time)
#define FAST_SERB        FPGA+0x12*4        // fast serial B (16 bits at a time)
