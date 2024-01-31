/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_utilities_hpp
#define quantlib_test_utilities_hpp

#include <ql/indexes/indexmanager.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/quote.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/functional.hpp>
#include <boost/test/unit_test.hpp>
#if BOOST_VERSION < 105900
#include <boost/test/floating_point_comparison.hpp>
#else
#include <boost/test/tools/floating_point_comparison.hpp>
#endif
#include <cmath>
#include <iomanip>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// This adapts the BOOST_CHECK_SMALL and BOOST_CHECK_CLOSE macros to
// support a struct as Real for arguments, while fully transparant to regular doubles.
// Unfortunately boost does not provide a portable way to customize these macros' behaviour,
// so we need to define wrapper macros QL_CHECK_SMALL etc.
//
// It is required to have a function `value` defined that returns the double-value
// of the Real type (or a value function in the Real type's namespace for ADT).

namespace QuantLib {
    // overload this function in case Real is something different - it should alway return double
    inline double value(double x) {
        return x;
    }
}

using QuantLib::value;

#define QL_CHECK_SMALL(FPV, T)  BOOST_CHECK_SMALL(value(FPV), value(T))
#define QL_CHECK_CLOSE(L, R, T) BOOST_CHECK_CLOSE(value(L), value(R), value(T))
#define QL_CHECK_CLOSE_FRACTION(L, R, T) BOOST_CHECK_CLOSE_FRACTION(value(L), value(R), value(T))


// This makes it easier to use array literals (for new code, use std::vector though)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

#define QUANTLIB_TEST_CASE(f) BOOST_TEST_CASE(QuantLib::detail::quantlib_test_case(f))

namespace QuantLib {

    namespace detail {

        // used to avoid no-assertion messages in Boost 1.35
        class quantlib_test_case {
            ext::function<void()> test_;
          public:
            template <class F>
            explicit quantlib_test_case(F test) : test_(test) {}
            void operator()() const {
                // Restore settings after each test.
                SavedSettings restore;
                // Clear all fixings before running a test to avoid interference.
                IndexManager::instance().clearHistories();
                BOOST_CHECK(true);
                test_();
            }
            #if BOOST_VERSION <= 105300
            // defined to avoid unused-variable warnings. It doesn't
            // work after Boost 1.53 because the functions were
            // overloaded and the address can't be resolved.
            void _use_check(
                    const void* = &boost::test_tools::check_is_close,
                    const void* = &boost::test_tools::check_is_small) const {}
            #endif
        };

    }

    std::string payoffTypeToString(const ext::shared_ptr<Payoff>&);
    std::string exerciseTypeToString(const ext::shared_ptr<Exercise>&);


    ext::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             const ext::shared_ptr<Quote>& forward,
             const DayCounter& dc);

    ext::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             Rate forward,
             const DayCounter& dc);

    ext::shared_ptr<YieldTermStructure>
    flatRate(const ext::shared_ptr<Quote>& forward,
             const DayCounter& dc);

    ext::shared_ptr<YieldTermStructure>
    flatRate(Rate forward,
             const DayCounter& dc);


    ext::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today,
            const ext::shared_ptr<Quote>& volatility,
            const DayCounter& dc);

    ext::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today,
            Volatility volatility,
            const DayCounter& dc);

    ext::shared_ptr<BlackVolTermStructure>
    flatVol(const ext::shared_ptr<Quote>& volatility,
            const DayCounter& dc);

    ext::shared_ptr<BlackVolTermStructure>
    flatVol(Volatility volatility,
            const DayCounter& dc);


    Real relativeError(Real x1, Real x2, Real reference);

    //bool checkAbsError(Real x1, Real x2, Real tolerance){
    //    return std::fabs(x1 - x2) < tolerance;
    //};

    class Flag : public QuantLib::Observer {
      private:
        bool up_ = false;

      public:
        Flag() = default;
        void raise() { up_ = true; }
        void lower() { up_ = false; }
        bool isUp() const { return up_; }
        void update() override { raise(); }
    };

    template<class Iterator>
    Real norm(const Iterator& begin, const Iterator& end, Real h) {
        // squared values
        std::vector<Real> f2(end-begin);
        std::transform(begin, end, begin, f2.begin(), std::multiplies<>());
        // numeric integral of f^2
        Real I = h * (std::accumulate(f2.begin(),f2.end(),Real(0.0))
                      - 0.5*f2.front() - 0.5*f2.back());
        return std::sqrt(I);
    }


    inline Integer timeToDays(Time t, Integer daysPerYear = 360) {
        return Integer(std::lround(t * daysPerYear));
    }


    // Used to check that an exception message contains the expected message string
    struct ExpectedErrorMessage {

        explicit ExpectedErrorMessage(std::string msg) : expected(std::move(msg)) {}

        bool operator()(const Error& ex) const {
            std::string actual(ex.what());
            if (actual.find(expected) == std::string::npos) {
                BOOST_TEST_MESSAGE("Error expected to contain: '" << expected << "'.");
                BOOST_TEST_MESSAGE("Actual error is: '" << actual << "'.");
                return false;
            } else {
                return true;
            }
        }

        std::string expected;
    };


    // Allow streaming vectors to error messages.

    // The standard forbids defining new overloads in the std
    // namespace, so we have to use a wrapper instead of overloading
    // operator<< to send a vector to the stream directly.
    // Defining the overload outside the std namespace wouldn't work
    // with Boost streams because of ADT name lookup rules.

    template <class T>
    struct vector_streamer {
        explicit vector_streamer(std::vector<T> v) : v(std::move(v)) {}
        std::vector<T> v;
    };

    template <class T>
    vector_streamer<T> to_stream(const std::vector<T>& v) {
        return vector_streamer<T>(v);
    }

    template <class T>
    std::ostream& operator<<(std::ostream& out, const vector_streamer<T>& s) {
        out << "{ ";
        if (!s.v.empty()) {
            for (size_t n=0; n<s.v.size()-1; ++n)
                out << s.v[n] << ", ";
            out << s.v.back();
        }
        out << " }";
        return out;
    }


}


#endif
