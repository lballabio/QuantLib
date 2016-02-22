#ifdef BOOST_TEST_DYN_LINK
/* This file needs to be linked with the test suite in case the
   shared-library version of the Boost unit-test framework is used. */
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>
#endif
