/*!
  \file VMEBusy.cpp
   Implements the CVMEBusy class.
   */
// Author:
//        Ron Fox
//        NSCL
//        Michigan State University
//        East Lansing, MI 48824-1321
//        fox@nscl.msu.edu
//
// Version information:
//    $Header$
//
/* Change log:
      $Log$
      Revision 3.1  2003/03/22 04:03:45  ron-fox
      Added SBS/Bit3 device driver.

      Revision 2.1  2003/02/11 16:44:57  ron-fox
      Retag to version 2.1 to remove the weird branch I accidently made.

      Revision 1.1.1.1  2003/02/05 14:04:53  ron-fox
      Initial import of the NSCL Daq clients version 7.0-000 to sourceforge.


      Revision 2.6  2002/10/23 12:00:42  fox
      Use output 3 for module clears (prior to busy release).

// Revision 2.5  2002/10/14  15:49:24  fox
// *** empty log message ***
//
// Revision 2.2  2002/07/02  15:12:21  fox
// Go to 2.xx based releases (recover from client crash too).
//
// Revision 2.1  2002/07/02  15:05:28  fox
// Transition to 2.1 releases
//
// Revision 1.1  2002/06/27  15:55:09  fox
// - Debug tight packed buffer Readout (note still problems with Spectrodaq)
// - Support SBS/Bit3 device driver in vmetcl et seq.
//

*/
#include "VMEBusy.h"

/*!
  Construct an object;  The NIMOUT and CAENIO module are saved
  for later use by the object.
  
  \parameter pClears - CNimout* [in] Pointer to the clears module.
  \parameter pBusy   - CCaenIO* [in] Pointer to the CAEN I/O trigger/busy
                       management module.
 */
CVMEBusy::CVMEBusy(CNimout* pClears, CCaenIO* pBusy) :
  m_rClearModule(*pClears),
  m_rBusyModule(*pBusy)
{
  
}
/*!
   Initialize the busy system.

   */
void
CVMEBusy::Initialize()
{

  m_rBusyModule.ClearAll();

}
/*!
  Set the busy.  This involves pulsing output 0 of the 
  IO register (GoingBusy bit).,

  */
void
CVMEBusy::Set()
{
  m_rBusyModule.PulseOutput(0);
}
/*!
  Clear the busy.  This involves pulsing output 1 of the IO register
  (GoingFree bit).
  */
void
CVMEBusy::Clear()
{
  m_rBusyModule.PulseOutput(1);
}
/*!
  Pulse the nimouts to provide front panel clears for the
  digitizers.
  */
void
CVMEBusy::ModuleClear()
{
  m_rBusyModule.PulseOutput(3);
}
/*!
   Set the busy indicating scaler readout has started.
   */
void
CVMEBusy::ScalerSet()
{
  Set();			// Go busy.
  m_rBusyModule.SetLevel(0); // For a scaler.
  m_rBusyModule.ClearLevel(0);

}
/*!
   Clear scaler busy.
   */
void
CVMEBusy::ScalerClear()
{
  m_rBusyModule.SetLevel(1);
  m_rBusyModule.ClearLevel(1);
  //  ModuleClear();
  //  Clear();
}
