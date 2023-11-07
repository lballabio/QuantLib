#include "speedlevel.hpp"
#include "quantlibglobalfixture.hpp"

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

if_speed::if_speed(SpeedLevel speed) : speed(speed) {}

tt::assertion_result if_speed::operator()(utf::test_unit_id) {
    tt::assertion_result level (QuantLibGlobalFixture::get_speed() <= speed);
    level.message() << "precondition failed";
    return level;
}