#include <boost/test/unit_test.hpp>

#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API

#include <iostream>
#include <iomanip>
#include <chrono>

#include "quantlibglobalfixture.hpp"
#include "speedlevel.hpp"

using namespace boost::unit_test;

QuantLibGlobalFixture::QuantLibGlobalFixture() {
    start = std::chrono::steady_clock::now();
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    speed = speed_level(argc, argv);
}

QuantLibGlobalFixture::~QuantLibGlobalFixture(){
    stop = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(start - stop).count() * 1e-3;
    int hours = int (seconds/3600);
    seconds -= hours * 3600;
    int minutes = int(seconds/60);
    seconds -= minutes * 60;

    std::cout << "\nTests completed in ";
    if (hours > 0)
        std::cout << hours << " h ";
    if (hours > 0 || minutes > 0)
        std::cout << minutes << " m ";
    std::cout << std::fixed << std::setprecision(0)
              << seconds << " s\n" << std::endl;
}

SpeedLevel QuantLibGlobalFixture::get_speed() {
    return speed;
}

SpeedLevel QuantLibGlobalFixture::speed_level(int argc, char** argv) {
    /*! Again, dead simple parser:
        - passing --slow causes all tests to be run;
        - passing --fast causes most tests to be run, except the slowest;
        - passing --faster causes only the faster tests to be run;
        - passing nothing is the same as --slow
    */

    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--slow")
            return Slow;
        else if (arg == "--fast")
            return Fast;
        else if (arg == "--faster")
            return Faster;
    }
    return Slow;
}

SpeedLevel QuantLibGlobalFixture::speed = Slow;

BOOST_TEST_GLOBAL_FIXTURE(QuantLibGlobalFixture);