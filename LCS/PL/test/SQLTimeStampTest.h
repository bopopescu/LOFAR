#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <PL/SQLTimeStamp.h>
#include <Common/Debug.h>

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

class SQLTimeStampTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(SQLTimeStampTest);
  CPPUNIT_TEST_EXCEPTION(testConstructorThrows, LCS::AssertError);
  CPPUNIT_TEST(testLessThanComparison);
  CPPUNIT_TEST_FAIL(testLessThanIsNotEqual);
  CPPUNIT_TEST_FAIL(testShouldOverflow);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void tearDown();
  void testConstructorThrows();
  void testLessThanComparison();
  void testLessThanIsNotEqual();
  void testShouldOverflow();
private:
  SQLTimeStamp sts1;
  SQLTimeStamp sts2;
  SQLTimeStamp sts3;
  SQLTimeStamp sts4;
  SQLTimeStamp stsf;
};
