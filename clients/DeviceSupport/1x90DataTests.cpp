// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CCAENV1x90Data.h"

#ifdef HAVE_STD_NAMESPACE
  using namespace std;
#endif

using namespace CCAENV1x90Data;

class DataTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DataTests);
  CPPUNIT_TEST(DataTypes);
  CPPUNIT_TEST(GlobalHeaderFields);
  CPPUNIT_TEST(TDCHeaderFields);
  CPPUNIT_TEST(TDCChipNumber);
  CPPUNIT_TEST(EventIdTest);
  CPPUNIT_TEST(TDCWordCountTest);
  CPPUNIT_TEST(TestMeasurement);
  CPPUNIT_TEST(ErrorBitsTest);
  CPPUNIT_TEST(TriggerTimeTest);
  CPPUNIT_TEST(GlobalTrailerTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void DataTypes();
  void GlobalHeaderFields();
  void TDCHeaderFields();
  void TDCChipNumber();
  void EventIdTest();
  void TDCWordCountTest();
  void TestMeasurement();
  void ErrorBitsTest();
  void TriggerTimeTest();
  void GlobalTrailerTest();
};



CPPUNIT_TEST_SUITE_REGISTRATION(DataTests);


// Various test data:


static const int GlobalHeader(0x40000aac); // Board 12(10), Event 0x55.
static const int TDCHeader(0x0e123567);	// TDC header:
                                        // chip 2, event 0x123, bunch 0x567.
static const int Measurement(0x06281234); // TDC data:
                                          // Trailing, Channel 69, value 0x1234
static const int TDCTrailer(0x1a321666);  // TDC Trailer word:
                                          // Chip 2, Event id 0x321, word count
                                          // 0x666
static const int TDCError(0x25ff5a5a);	  // TDC chip error...
                                          // Chip 1 (extra bits),
                                          // Every other error bit (evens)
static const int TTimeTag(0x88789abc);	  // Extended trigger time tag:
                                          // Time 0x789abc.
static const int GlobalTrailer(0x87f033e5); // Global trailer:
                                            // overflow, tdc error, trigger lost.
                                            // Word count = 0x819f, Geo = 5.
static const int Filler(0xc7ffffff);	    // Filler with bunches of extra bits set.


// Check that all the test data can be identified, and none of them wrongly so.
//
void
DataTests::DataTypes()
{
  // Global header:

  ASSERT(isGlobalHeader(GlobalHeader));
  ASSERT(!isGlobalTrailer(GlobalHeader));
  ASSERT(!isTDCHeader(GlobalHeader));
  ASSERT(!isTDCTrailer(GlobalHeader));
  ASSERT(!isTDCError(GlobalHeader));
  ASSERT(!isMeasurement(GlobalHeader));
  ASSERT(!isTriggerTimeTag(GlobalHeader));
  ASSERT(!isFiller(GlobalHeader));

  // TDC Header:

  ASSERT(!isGlobalHeader(TDCHeader));
  ASSERT(!isGlobalTrailer(TDCHeader));
  ASSERT(isTDCHeader(TDCHeader));
  ASSERT(!isTDCTrailer(TDCHeader));
  ASSERT(!isTDCError(TDCHeader));
  ASSERT(!isMeasurement(TDCHeader));
  ASSERT(!isTriggerTimeTag(TDCHeader));
  ASSERT(!isFiller(TDCHeader));


  // Measurement:

  ASSERT(!isGlobalHeader(Measurement));
  ASSERT(!isGlobalTrailer(Measurement));
  ASSERT(!isTDCHeader(Measurement));
  ASSERT(!isTDCTrailer(Measurement));
  ASSERT(!isTDCError(Measurement));
  ASSERT(isMeasurement(Measurement));
  ASSERT(!isTriggerTimeTag(Measurement));
  ASSERT(!isFiller(Measurement));

  // TDC Trailer:

  ASSERT(!isGlobalHeader(TDCTrailer));
  ASSERT(!isGlobalTrailer(TDCTrailer));
  ASSERT(!isTDCHeader(TDCTrailer));
  ASSERT(isTDCTrailer(TDCTrailer));
  ASSERT(!isTDCError(TDCTrailer));
  ASSERT(!isMeasurement(TDCTrailer));
  ASSERT(!isTriggerTimeTag(TDCTrailer));
  ASSERT(!isFiller(TDCTrailer));

  // TDCError:

  ASSERT(!isGlobalHeader(TDCError));
  ASSERT(!isGlobalTrailer(TDCError));
  ASSERT(!isTDCHeader(TDCError));
  ASSERT(!isTDCTrailer(TDCError));
  ASSERT(isTDCError(TDCError));
  ASSERT(!isMeasurement(TDCError));
  ASSERT(!isTriggerTimeTag(TDCError));
  ASSERT(!isFiller(TDCError));

  // Extended time tag:

  ASSERT(!isGlobalHeader(TTimeTag));
  ASSERT(!isGlobalTrailer(TTimeTag));
  ASSERT(!isTDCHeader(TTimeTag));
  ASSERT(!isTDCTrailer(TTimeTag));
  ASSERT(!isTDCError(TTimeTag));
  ASSERT(!isMeasurement(TTimeTag));
  ASSERT(isTriggerTimeTag(TTimeTag));
  ASSERT(!isFiller(TTimeTag));

  // Global trailer:

  ASSERT(!isGlobalHeader(GlobalTrailer));
  ASSERT(isGlobalTrailer(GlobalTrailer));
  ASSERT(!isTDCHeader(GlobalTrailer));
  ASSERT(!isTDCTrailer(GlobalTrailer));
  ASSERT(!isTDCError(GlobalTrailer));
  ASSERT(!isMeasurement(GlobalTrailer));
  ASSERT(!isTriggerTimeTag(GlobalTrailer));
  ASSERT(!isFiller(GlobalTrailer));

  // Filler:

  ASSERT(!isGlobalHeader(Filler));
  ASSERT(!isGlobalTrailer(Filler));
  ASSERT(!isTDCHeader(Filler));
  ASSERT(!isTDCTrailer(Filler));
  ASSERT(!isTDCError(Filler));
  ASSERT(!isMeasurement(Filler));
  ASSERT(!isTriggerTimeTag(Filler));
  ASSERT(isFiller(Filler));
}

/*!
  Pick apart the global header fields:
  Our global header test data is for board 12, event 0x55
 */
void
DataTests::GlobalHeaderFields()
{
  // Using a real global header we should be able to pick out the header
  // fields correctly:

  EQMSG("Global header Trigger Number", 0x55, TriggerNumber(GlobalHeader));
  EQMSG("Global header Board Number",   (unsigned int)12,   BoardNumber(GlobalHeader));

  // Using a bad global header, we should get string exceptions.

  EXCEPTION(TriggerNumber(Filler), string);
  EXCEPTION(BoardNumber(Filler),   string);

}
/*!
   Extract the fields of the TDC HEADER.
   If I attempt to extract these fields from a global header, e.g., 
   exceptions should be thrown.
*/
void
DataTests::TDCHeaderFields()
{
  EQMSG("TDC Chip", 2, (int)TDCChip(TDCHeader));
  EQMSG("TDC Event", 0x123, (int)EventId(TDCHeader));
  EQMSG("TDC Bunchid", 0x567, (int)BunchId(TDCHeader));

  // Those should all fail throwing string exceptions if
  // I hand them a global header:


  EXCEPTION(TDCChip(GlobalHeader), string);
  EXCEPTION(EventId(GlobalHeader), string);
  EXCEPTION(BunchId(GlobalHeader), string);

}
/*!
  Check TDCChip's ability to work for trailers and errors:
*/
void
DataTests::TDCChipNumber()
{
  EQMSG("Trailer ", 2, (int) TDCChip(TDCTrailer));
  EQMSG("Error   ", 1, (int) TDCChip(TDCError));
}
/*!
   Check EventId's ability to decode the event identification.
*/
void
DataTests::EventIdTest()
{
  EQMSG("Trailer: ", 0x321, (int)EventId(TDCTrailer));
}
/*!
   Test ability to pull word count from TDC Trailers:
*/
void
DataTests::TDCWordCountTest()
{
  EQMSG("TDC trailer word count ",
	0x666, (int)TDCWordCount(TDCTrailer));

  EXCEPTION(TDCWordCount(Filler), string);

}
/*!
  Test functions that decode a measurement.
*/
void
DataTests::TestMeasurement()
{

  // Extract the fields from a valid datum:

  EQMSG("isTrailing:    ", true, isTrailing(Measurement));
  EQMSG("ChannelNumber: ", 69, ChannelNumber(Measurement));
  EQMSG("Channel value: ", 0x1234L, ChannelValue(Measurement));

  // ensure exceptions get thrown for invalid datum:

  EXCEPTION(isTrailing(Filler),    string);
  EXCEPTION(ChannelNumber(Filler), string);
  EXCEPTION(ChannelValue(Filler),  string);

}

/*!
   Test ability to pull error mask from error word in event stream.
*/
void
DataTests::ErrorBitsTest()
{
  EQMSG(" Error Bits", 0x5a5a, (int)TDCErrorBits(TDCError));

  EXCEPTION((TDCErrorBits(Filler)), string)


}
/*!
   Test ability to pull trigger time test from event stream.
*/
void 
DataTests::TriggerTimeTest()
{
  EQMSG(" TriggerTime ", 0x789abcUL , ExtendedTriggerTime(TTimeTag));

  EXCEPTION(ExtendedTriggerTime(Filler), string);
}
/*!
   Test ability to get stuff from global trailers:
*/
void
DataTests::GlobalTrailerTest()
{
  // The real global trailer should work:

  EQMSG("Overflow   ", true, Overflow(GlobalTrailer));
  EQMSG("Error      ", true, Error(GlobalTrailer));
  EQMSG("Lost       ", true, Lost(GlobalTrailer));
  EQMSG("GEO        ", 5,    (int)BoardNumber(GlobalTrailer));
  EQMSG("Event Size ", 0x819f, (int)EventSize(GlobalTrailer));

  // The fake should vomit strings:

  EXCEPTION(Overflow(Filler),    string);
  EXCEPTION(Error(Filler),       string);
  EXCEPTION(Lost(Filler),        string);
  EXCEPTION(BoardNumber(Filler), string);
  EXCEPTION(EventSize(Filler),   string);

}
