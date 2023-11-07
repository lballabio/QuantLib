#ifndef quantlib_global_fixture_hpp
#define quantlib_global_fixture_hpp

#include <ql/types.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <ql/version.hpp>

#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#include "paralleltestrunner.hpp"
#define BOOST_TEST_ALTERNATIVE_INIT_API
#else
#include <boost/test/unit_test.hpp>
#endif

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/

#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif

#include "speedlevel.hpp"
#include <chrono>

class QuantLibGlobalFixture {
  public:
    QuantLibGlobalFixture();
    ~QuantLibGlobalFixture();
    static SpeedLevel get_speed ();
    SpeedLevel speed_level (int argc, char **argv);

  private:
    static SpeedLevel speed;
    decltype(std::chrono::steady_clock::now()) start;
    decltype(std::chrono::steady_clock::now()) stop;
};

#endif // quantlib_global_fixture_hpp