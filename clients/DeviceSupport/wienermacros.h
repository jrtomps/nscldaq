/*
 * macros_vc32.h  - Macro definitions for data acquisition user programming
 *                  assumed variables: INT16 *buffer -> buffer.  Modifications
 *                  to macros.h for using Wiener vc32.
 */

    /* CAMAC and buffer manipulation. */

#ifndef _WIENERMACROS_H  
#define _WIENERMACROS_H


#ifndef __CRT_STDIO_H
#include <stdio.h>
#define __CRT_STDIO_H
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#define __CRT_UNISTD_H
#endif


#ifndef __CAMAC_H
#include <camac.h>
#endif

#define putbufw(word)   (*bufpt = (INT16)(word)); ++bufpt
#define putbufl(l)      {                                   \
                            UINT32 tmp = (l);               \
                            putbufw((tmp&0xffff));          \
                            putbufw((tmp >> 16));           \
                        }

#define savebufpt(var)  var = (INT32)bufpt
#define rstbufpt(var)   bufpt = ((INT16 *)var)


/*All definitions referring to CBDPTR are changed from macros.h--ddc */
#define camread16(b,c,n,a,f)     (CAMRD16(CBDPTR((b), (c),(n),(a),(f), CAM16)))
#define camread24(b, c, n, a, f) (CAMRD24(CBDPTR((b), (c),(n),(a),(f), CAM24)))
#define camread32(b, c, n, a, f) (CAMRD32(CBDPTR((b), (c),(n),(a),(f), CAM24)))

#define camwrite16(b,c,n,a,f,d)  (CAMWR16(CBDPTR((b), (c),(n),(a),(f), CAM16),d))
#define camwrite24(b,c,n,a,f,d)  (CAMWR24(CBDPTR((b), (c),(n),(a),(f), CAM24),d))
/* 
camctl...
unfortunately, the prior version of camctl assumed that a READ was
was always being done.  Even though there is (or should be!) NO data for 
commands that use "camctl", the wiener VC32-CC32 depends on the type of 
access to determine the actual function that it executes (reading for 
F<16, write for F>=16). So, we replace the camctl macro. --ddc
*/ 

/* #define camctl(b,c,n,a,f)        CAMCTL(CBDPTR((b), (c), (n), (a), (f), CAM16)) */
#define camctl(b,c,n,a,f)  {  \
            if(f<16){ \
                if( *(volatile INT16 *)CBDPTR((b), (c),(n),(a),(f), CAM16) ){}; \
            } else *(volatile INT16 *)CBDPTR((b), (c),(n),(a),(f), CAM16)=0;    \
	}

#ifdef LONGBRANCH


#define rdtobuf16(b, c, n, a, f) { putbufw(camread16((b),(c),(n),(a),(f)))
#define rdtobuf24(b, c, n, a, f) { putbufl(camread24((b),(c),(n),(a),(f)))
#else
#define rdtobuf16(b, c, n, a, f) putbufw(camread16((b),(c),(n),(a),(f)))
#define rdtobuf24(b, c, n, a, f) putbufl(camread24((b),(c),(n),(a),(f)))

#endif

   /*Wiener cc32 uses NAF of 0,0,0 to get status word --ddc */
  /*NOTE! BRANCH is not used any longer,  specify crate */
#define qtst(c)       ((camread16(0,(c),0,0,0) & 0x08) != 0)
#define xtst(c)       ((camread16(0,(c),0,0,0) & 0X04) != 0)

/*Until my next comment... there are no changes from macros.h --ddc*/
    
   /* Bitwise operators. */

#define IAND(a,b)       ((a) & (b))
#define IOR(a, b)       ((a) | (b))
#define ISHIFT(a, b)       ((a) << (b))

    /* Boolean operators. Same --ddc */

#define EQ       ==
#define NE       !=
#define GT       >
#define GE       >=
#define LT       <
#define LE       <=
#define AND       &&
#define OR       ||
#define NOT       !
#ifndef TRUE
#define TRUE       1
#endif
#ifndef FALSE
#define FALSE       0
#endif


    /* Flow of control */

#define IF       if
#define THEN       {
#define ELSE       } else {
#define ENDIF       }
#define DO
#define WHILE(expr) while(expr) {
#define ENDDO       }
#define CALL       
#ifdef __unix__
#else
#define sleep(sec)              pause_p((int)((sec)*second))
#endif
    /* Declarations */

#define LOGICAL       UINT32
#define WORD       INT16
#define INTEGER       INT32
#define REAL       float

    /* code fragment generators: */


#define qstoptobuf(b,c,n,a,f) do {                            \
                             putbufw(camread16(b,c,n,a,f));   \
                           } while(qtst(b));                  \
                           --bufpt;


/*
 * Ok... obviously branchinit and crateinit are largely different.
 * The "branchinit" we use to initialize the vc32 interface(s).  The
 * branch number is irrelevent, but each vc32 has a "number" same as
 * crate number. --ddc
 */

 extern void WIENERBranchAccess(int branch);

#define AR 0x1000             /* turns on autoread ... NORMALLY OFF! */
#define ROAK 0x0800           /* ROAK mode, RORA mode if zero */
#define VME_LAM_INT 0x0000    /* If vme int generated for LAMS (bits8-11) */
#define VME_INT_VEC 0x0000    /* If vme int vector for LAMS (bits0-7) */
#define branchinit(b)  WIENERBranchAccess(b);  \
                      camwrite16(b,b+1,0,0,3,VME_LAM_INT|VME_INT_VEC);     \
                       camctl(b,b+1,31,0,16);                              \
                     sleep(2)

/* NAF (0,3,16) Z+I, (0,2,16) C+Uninhibit, (28,1,16,0) set lam mask */
 
#define crateinit(b, c) while((camread16(b,c,0,0,3) & 0x4000) == 0)    \
                      {                                                \
                        fprintf(stderr, "Crate %d is offline\n",c);    \
                        sleep(5);                                      \
                      }                                                \
                     camwrite24(b,c,28,1,16,0);                        \
                     camctl(b,c,0,3,16);                               \
                     camctl(b,c,0,2,16)


#ifdef LONGBRANCH
#define READBIT(b, c, n, a, f, d) d = camread16(b,c,n,a,f);               \
                              putbufw(d)
#else
#define READBIT(b, c, n, a, f, d) d = camread16(b,c,n,a,f);               \
                              putbufw(d)
#endif  
#define BEGINLAM(br,cr) {                                                 \
                          UINT16 _b,_c;                                   \
                          INT16 _timout;                                  \
                          UINT32 _lammsk[br][cr];                         \
                          UINT16 MAXBR = br;                              \
                          UINT16 MAXCR = cr;                              \
                                                                          \
                          for(_b = 0; _b<br; _b++)                        \
                          for(_c = 0; _c < cr; _c++) _lammsk[_b][_c] = 0;

#define ENDLAM       }

#define NEEDLAM(b, c, n)       _lammsk[b][c-1] |= (1 << (n-1))

#define IFTIMEOUT(maxloop) _timout = maxloop;     /* maxloop in ~10us */      \
                           _b = 0; _c = 0;                                    \
                           while(_timout >= 0)                                \
                           {                                                  \
                             if(_lammsk[_b][_c] == 0)                         \
                             {                                                \
                               _c++;                                          \
                               if(_c >= MAXCR)                                \
                               {                                              \
                                 _c = 0;                                      \
                                 _b++;                                        \
                                 if(_b >= MAXBR) break;                       \
                               }                                              \
                             } else                                           \
                               {                                              \
                                  _timout--;                                  \
                                  _lammsk[_b][_c] &= ~(_lammsk[_b][_c] &      \
                                                   camread24(_b,_c+1,28,4,0));\
                                }                                             \
                           }                                                  \
                           if(_timout < 0)

#define LAMREAD(b,c)   camread24((b), (c), 28, 4, 0)

/* LAM read commands in macros changed to NAF 28,4,0 --ddc */


/* Device specific macros */

       /* ORTEC AD811's */

#define INIT811(b, c, n)       camctl((b),(c),(n),12,11); \
                            camctl((b),(c),(n),12,26)
#define CLR811(b, c, n)              camctl((b),(c),(n),12,11)
#define READ811(b, c, n, a)       rdtobuf16((b),(c),(n),(a),0)

       /* LRS 22xx series devices */

#define INIT22XX(b, c, n)       camctl((b),(c),(n),0,9); \
                            camctl((b),(c),(n),0,26)
#define CLR22XX(b, c, n)       camctl((b),(c),(n),0,9)
#define READ22XX(b, c, n, a)       rdtobuf16((b),(c),(n),(a),0)

       /* LRS feras. */

#define FERA_OFS       0x8000
#define FERA_CLE       0x4000
#define FERA_CSR       0x2000
#define FERA_CCE       0x1000
#define FERA_CPS       0x0800
#define FERA_EEN       0x0400
#define FERA_ECE       0x0200
#define FERA_EPS       0x0100
#define busy(br, cr, sl) {                                             \
                     fprintf(stderr,                                   \
                           "Module busy during programming: \n");      \
                     fprintf(stderr,                                   \
                              "Br = %d Cr = %d Slot = %d \n",          \
                              (br), (cr), (sl));                       \
                     sleep(2);                                         \
                     }
#define INITFERA(b, c, n, cmd, ped) { INT16 _sub;                       \
                                     INT32 *_p;                         \
                                     do                                 \
                                     {                                  \
                                      camctl((b),(c),(n),0,9);          \
                                      camwrite16((b),(c),(n),0,16,cmd); \
                                      if(!qtst(b))                      \
                                         busy((b),(c),(n));             \
                                     }while (!qtst((b)));               \
                                     _p = &userints[ped];               \
                                     for(_sub=0;_sub<=15; _p++,_sub++)  \
                                       do                               \
                                       {                                \
                                         camctl((b),(c),(n),0,9);       \
                                         camwrite16((b),(c),(n),_sub,   \
                                                     17,*_p);           \
                                         if(!qtst(b))                   \
                                         busy((b),(c),(n));             \
                                       } while(!qtst(b));               \
                                    }
#define CLRFERA(b, c, n)       camctl((b),(c),(n),0, 9)
#define READFERA(b, c, n, a)       rdtobuf16((b),(c),(n),(a),2)
#define READFERAALL(b, c, n)       qstoptobuf((b),(c),(n),0,2)

#define INIT812F(b,c,n, cmd, ped)   { INT16 _sub;                        \
                                      INT32 *_p;                         \
                                      do                                 \
                                      {                                  \
                                      camctl((b),(c),(n),0,9);           \
                                      camwrite16((b),(c),(n),0,16,cmd);  \
                                      if(!qtst(b))                       \
                                         busy((b),(c),(n));              \
                                      }while (!qtst((b)));               \
                                      _p = &userints[ped];               \
                                      for(_sub=0;_sub<=7; _p++,_sub++)   \
                                        do                               \
                                        {                                \
                                          camctl((b),(c),(n),0,9);       \
                                          camwrite16((b),(c),(n),_sub,   \
                                                      17,*_p);           \
                                          if(!qtst(b))                   \
                                          busy((b),(c),(n));             \
                                        } while(!qtst(b));               \
                                    }


    /* The definitions below handle Silena ADC's */

/* initialize Silena 4418 ADC
** Arguments:
**       int b,c,n       - Branch, crate, slot
**       int cmd              - status register
**       int thrsh       - number of first of 25 users integers 
**                         common threshold x1
**                         lower level discriminator x8
**                         offset x8
**                         upper level discriminator x8
*/

#define INIT4418(b,c,n,cmd,thrsh) {                                   \
    INT16 i;                                                          \
    INT32 *_p;                                                        \
    do{                                                               \
       camctl(b,c,n,0,9);  /* clear */                                \
       camwrite16(b,c,n,14,20,cmd); /* write status word */           \
       if(!qtst(b))busy(b,c,n);                                       \
    }while(!qtst(b));                                                 \
    _p =  &userints[thrsh];                                           \
    do{                                                               \
       camwrite16(b,c,n,9,20,*_p); /* write common threashold */      \
       if(!qtst(b))busy(b,c,n);                                       \
    }while(!qtst(b));                                                 \
    ++_p;                                                             \
                 /* write LLD value */                                \
    for(i=8;i<16;_p++,i++)  do{                                       \
       camwrite16(b,c,n,i,17,*_p);                                    \
       if(!qtst(b))busy(b,c,n);                                       \
    }while(!qtst(b));                                                 \
              /* write offset value */                                \
    for(i=0;i<8;_p++,i++) do{                                         \
       camwrite16(b,c,n,i,20,*_p);                                    \
       if(!qtst(b))busy(b,c,n);                                       \
    }while(!qtst(b));                                                 \
                 /* write LLD value */                                \
    for(i=0;i<8;_p++,i++)do{                                          \
       camwrite16(b,c,n,i,17,*_p);                                    \
       if(!qtst(b))busy(b,c,n);                                       \
    }while(!qtst(b));                                                 \
 }

/*
** zero suppressed read.
*/
#define READ4418(b,c,n,a) rdtobuf16((b),(c),(n),(a), 2)
#define RDHDR4418(b,c,n)  camread16((b),(c),(n),14, 2)
#define RDPAT4418(b,c,n)  camread16((b),(c),(n),15, 2)
#define BLKREAD4418(b,c,n) qstoptobuf((b),(c),(n),0, 2)


       /* Generic bit registers
                                              */

#define INITBIT(b, c, n)    camctl((b),(c),(n),0,11);                     \
                            camctl((b),(c),(n),0,26)
#define CLRBIT(b, c, n)     camctl((b),(c),(n),0,11)
#define RDBIT(b, c, n, a)   rdtobuf16((b),(c),(n),(a),0)

       /* NIM out pattern registers. */

#define NIMOUT(b, c, n, pattern) camwrite16((b),(c),(n),0,16,pattern)

       /* LRS 4448 scalers. */

#define ECLSCL_TEST     0x8000
#define ECLSCL_BD       0x2000
#define ECLSCL_RD       0x0080
#define ECLSCL_CL       0x0040
#define ECLSCL_LD       0X0020
#define ECLSCL_NPOS     8

/* GSI scalers. */
#define INITGSIS(b, c, n)   { camwrite16((b),(c),(n),0,16,0);   \
                              camctl((b),(c),(n),0,9);          \
                              camctl((b),(c),(n),0,1); }
#define CLRGSIS(b, c, n)    INITGSIS((b),(c),(n))


#define READALLGSIS(b, c, n)  { INT16 _a;                        \
                                INITGSIS((b),(c),(n));           \
                                for(_a = 0; _a <=47; _a++) {     \
                                  long l =                       \
                                  camread24((b),(c),(n),0,0));   \
                                  putbufl(l);                    \
                                }                                \
                              }                               


#define INIT4434(b, c, n)     camwrite16((b),(c),(n),0,16, ECLSCL_CL |      \
                                                      ECLSCL_BD)
#define CLR4434(b, c, n)      INIT4434((b),(c),(n))
#define READ4434(b, c, n, a)  camwrite16((b),(c),(n),0,16,          \
                                                   (ECLSCL_LD) |    \
                                                      (a));         \
                              putbufl(                              \
                                      camread24((b),(c),(n),0,0)) 
#define READALL4434(b, c, n) { INT16 _a;                            \
                               camwrite16((b),(c),(n),0,16,         \
                                                    (ECLSCL_LD)  |  \
                                            (31 << ECLSCL_NPOS));   \
                             for(_a = 31; _a != -1; _a--)           \
                             {                                      \
                                 putbufl(                           \
                                       camread24((b),(c),(n),0,2)); \
                             }                                      \
                             }

       /* LRS 2551 scalers. */

#define INIT2551(b, c, n)      camctl((b),(c),(n),0,9)
#define CLR2551(b, c, n)       INIT2551((b),(c),(n))
#define READ2551(b, c, n, a)   putbufl(                                     \
                               camread24((b),(c),(n),(a),0))

#define READALL2551(b, c, n)  { INT16 _a;                                   \
                                for(_a = 0; _a < 12; _a++)                  \
                                   putbufl(                                 \
                                   camread24((b),(c),(n),(_a), 0));         \
                              }

/*
** Handle phillips adc/qdc/tdc:
*/
/*   For the 7164 ADCs:
*     cmd      is the control register
*              bits 1,2,3 are ped, lower thrsh, upper thrsh enable
*              bits 9-16 are conversion delay
*     ped      is the first of 16 integers for pedestals */
#define INIT7164(b,c,n,cmd,ped) {                        \
    INT16 i;                                             \
    INT32 *_p;                                           \
    camctl(b,c,n,0,9);                   /* clear */     \
    do{                                                  \
          camwrite16(b,c,n,0,19,cmd);                    \
          if(!qtst(b))busy(b,c,n);                       \
    }while(!qtst(b));                                    \
         _p = &userints[ped];                            \
         for(i=0;i<16;_p++,i++)do{                       \
            camwrite16(b,c,n,0,17,i); /* ped to follow*/ \
            camwrite16(b,c,n,i,20,*_p); /* here it is */ \
            if(!qtst(b))busy(b,c,n);                     \
         }while(!qtst(b));                               \
         camctl(b,c,n,0,26);                             \
}

#define INIT7186(b,c,n,ip,ilow,iup,ped){                 \
        INT16 i,ibit,icmd=0;                             \
        INT32 *_p;                                       \
        camctl(b,c,n,0,9);              /* clear */      \
        if(ip)icmd |= 1;     /* set pedestals */         \
        if(ilow)icmd |= 2;   /* set lower thresh */      \
        if(iup)icmd |= 4;    /* set upper thresh */      \
        camwrite16(b,c,n,0,19,icmd);                     \
        _p = &userints[ped];                             \
        for(ibit=0;ibit<3; ibit++){/* ped, low, upper */ \
          if((1 < ibit) & icmd){                         \
            camwrite16(b,c,n,ibit,17,0);                 \
            for(i=0; i<16; _p++,i++)do{                  \
              camwrite16(b,c,n,i,20,*_p);                \
              if(!qtst(b))busy(b,c,n);                   \
            }while(!qtst(b));                            \
          }                                              \
        }                                                \
        camctl(b,c,n,0,26);    /* enable LAM */          \
}       
/*   Reads and clears are straightforward 
 * bits 12-16 contain the address, which we don't want
 * I don't know what a LONGBRANCH is, so I'll
 * follow what precedes this */
#ifdef LONGBRANCH
#define READ7164(b,c,n,a) putbufw(camread16((b),(c),(n),(a),0) & 0x0fff)
#define READ7186(b,c,n,a) putbufw(camread16((b),(c),(n),(a),0))
#else
#define READ7164(b,c,n,a) putbufw(camread16((b),(c),(n),(a),0) & 0x0fff)
#define READ7186(b,c,n,a) putbufw(camread16((b),(c),(n),(a),0))
#endif
#define CLR7164(b,c,n)    camctl((b),(c),(n),0,9)


       /* Initialize the triggers; */
       /* User trigger 1 is done by a */

#define STARTUSR1TRIG(frequency)       trig1init((frequency))
#define STOPUSR1TRIG                   trig1kill

       /* I/O macros */

#define msg(txt)       fprintf (stderr, txt)
#define hexout(val)    fprintf (stderr, "%x", val)
#define decout(val)    fprintf (stderr, "%d", val)
#define newline        fprintf (stderr, "\n")


/* Data packetizing macros:


/*
** Macros for building subevent structures:
*/

              /* Fixed size sub event packet. */

#define Packet(size, type) { putbufw(size); putbufw(type);
#define EndPacket          }

              /* Variable sized sub event packet */

#ifdef __unix__
#define VPacket(type)   {                                           \
                            DAQWordBufferPtr _pktstart = bufpt;     \
                            ++bufpt;                                \
                            putbufw(type);
#else
#define VPacket(type)   {                                            \
                          INT16 *_pktstart = bufpt; ++bufpt;         \
                            putbufw(type);
#endif
#ifdef __unix__
#define EndVPacket        *_pktstart = (INT16)(bufpt.GetIndex() -     \
                                               _pktstart.GetIndex()); \
                        }
#else
#define EndVPacket        *_pktstart = (INT16)(bufpt - _pktstart);    \

                        }
#endif
/*
**   Reserve a fixed length chunk of buffer which will be filled in
**   later.  ptr will point to the start of this buffer. 
*/
                     
#define Reserve(ptr, n)   ptr = bufpt; bufpt += (n);


/*#endif  * endif for MACROS_H defined */
#endif         /*  _WIENERMACROS_H */
