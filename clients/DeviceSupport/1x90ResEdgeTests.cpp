// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CCAENV1x90.h"
#include "DesignByContract.h"


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DesignByContract;

extern long ModuleBase;

class ResEdgeTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ResEdgeTests);
  CPPUNIT_TEST(LSBTest);
  CPPUNIT_TEST(EdgeTest);
  CPPUNIT_TEST(PairTest);
  CPPUNIT_TEST(DeadTimeTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CCAENV1x90* m_pModule;

  void CheckResolution(CCAENV1x90::Resolution res);
  void CheckEdge(CCAENV1x90::EdgeMode nmode);
  void CheckPair(CCAENV1x90::LEResolution leRes,
		 CCAENV1x90::PWResolution pwRes);
  void CheckDead(CCAENV1x90::DeadTime dt);

public:
  // Construct a module in powered up condition:

  void setUp() {
    m_pModule = new CCAENV1x90(1, 0, ModuleBase);
    m_pModule->Reset();
  }
  // Delete the module to prevent resource leaks.

  void tearDown() {
    delete m_pModule;
  }
protected:
  void LSBTest();
  void EdgeTest();
  void PairTest();
  void DeadTimeTest();

};

CPPUNIT_TEST_SUITE_REGISTRATION(ResEdgeTests);


CCAENV1x90::LEResolution les[] = {CCAENV1x90::LE_100ps,
                                  CCAENV1x90::LE_200ps,
				  CCAENV1x90::LE_400ps,
				  CCAENV1x90::LE_800ps,
				  CCAENV1x90::LE_1600ps,
				  CCAENV1x90::LE_3120ps,
				  CCAENV1x90::LE_6250ps,
				  CCAENV1x90::LE_12500ps
};
unsigned const int nLEResolutions=sizeof(les)/
                                  sizeof(CCAENV1x90::LEResolution);
			      
CCAENV1x90::PWResolution pws[]  = { CCAENV1x90::PW_100ps, 
                                  CCAENV1x90::PW_200ps,
				  CCAENV1x90::PW_400ps,
				  CCAENV1x90::PW_800ps,
				  CCAENV1x90::PW_1600ps,
				  CCAENV1x90::PW_3200ps,
				  CCAENV1x90::PW_6250ps,
				  CCAENV1x90::PW_12500ps,
				  CCAENV1x90::PW_25ns,
				  CCAENV1x90::PW_50ns,
				  CCAENV1x90::PW_100ns,
				  CCAENV1x90::PW_200ns,
				  CCAENV1x90::PW_400ns,
				  CCAENV1x90::PW_800ns };
unsigned const int nPWResolutions = sizeof(pws) /
                                    sizeof(CCAENV1x90::PWResolution);

void 
ResEdgeTests::CheckResolution(CCAENV1x90::Resolution res)
{
    m_pModule->SetIndividualLSB(res);
    unsigned short nRes = m_pModule->GetResolution();
    EQ(res, m_pModule->InterpretEdgeResolution(nRes));
}

void
ResEdgeTests::CheckEdge(CCAENV1x90::EdgeMode nmode)
{
  m_pModule->SetEdgeDetectMode(nmode);
  EQ(nmode, m_pModule->GetEdgeDetectMode()); 

}

void
ResEdgeTests::CheckPair(CCAENV1x90::LEResolution leres,
			CCAENV1x90::PWResolution pwres)
{
  m_pModule->SetPairResolutions(leres, pwres);
  unsigned short res = m_pModule->GetResolution();


  EQ(leres, m_pModule->InterpretLEResolution(res));
  EQ(pwres, m_pModule->InterpretWidthResolution(res));

}

void
ResEdgeTests::CheckDead(CCAENV1x90::DeadTime dead)
{
  m_pModule->SetDoubleHitResolution(dead);
  EQ(dead, m_pModule->GetDoubleHitResolution());

}

// Test ability to set/get resolution of the LSB.

void
ResEdgeTests::LSBTest()
{

  // Check 25 ps:

  if(m_pModule->getModel() == 1190) { // 1190 doesn't support this...
    EXCEPTION(m_pModule->SetIndividualLSB(CCAENV1x90::Res_25ps),
	      Require);
  } 
  else {			// 1290 does support 25ps.

    CheckResolution(CCAENV1x90::Res_25ps);

  }

  // Check 100ps, 200ps, 800ps:

  CheckResolution(CCAENV1x90::Res_100ps);
  CheckResolution(CCAENV1x90::Res_200ps);
  CheckResolution(CCAENV1x90::Res_800ps);


}
// Check edge detection modes.

void
ResEdgeTests::EdgeTest()
{
  m_pModule->SetIndividualLSB(CCAENV1x90::Res_100ps); // Pair mode good.

  CheckEdge(CCAENV1x90::EdgeMode_Pair);
  CheckEdge(CCAENV1x90::EdgeMode_Trailing);
  CheckEdge(CCAENV1x90::EdgeMode_Leading);
  CheckEdge(CCAENV1x90::EdgeMode_Both);

  // If a 1290, setting to 25ps resolution should cause]
  // pair mode to throw:

  if(m_pModule->getModel() == 1290) {
    m_pModule->SetIndividualLSB(CCAENV1x90::Res_25ps);
    EXCEPTION(m_pModule->SetEdgeDetectMode(CCAENV1x90::EdgeMode_Pair),
	      Require);
  }
  
}

// Check pulse width resolution modes:

void
ResEdgeTests::PairTest()
{
  // Set up the module in pair mode (must be 100ps or worse resolution)

  m_pModule->SetIndividualLSB(CCAENV1x90::Res_100ps);
  m_pModule->SetEdgeDetectMode(CCAENV1x90::EdgeMode_Pair);

  for (int le  = 0; le < nLEResolutions; le++) {
    for (int pw = 0; pw < nPWResolutions; pw++) {
      CheckPair(les[le],
		pws[pw]);
    }
  }
}
//  Check dead times:

void
ResEdgeTests::DeadTimeTest()
{
  CheckDead(CCAENV1x90::DT_5ns);
  CheckDead(CCAENV1x90::DT_10ns);
  CheckDead(CCAENV1x90::DT_30ns);
  CheckDead(CCAENV1x90::DT_100ns);


}
