
/*
 Copyright (C) 2003, 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_utilities_hpp
#define quantlib_test_utilities_hpp

#include <ql/Instruments/payoffs.hpp>
#include <ql/exercise.hpp>
#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>
#include <ql/Patterns/observable.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <vector>
#include <string>
#include <numeric>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))


/* the following somewhat support setup() and teardown() functions,
   the latter being called even in presence of exceptions.  When used,
   QL_TEST_BEGIN must begin the test, and either QL_TEST_TEARDOWN or
   QL_TEST_END must close the test.  When needed, QL_TEST_SETUP must
   be put in between. */
#define QL_TEST_BEGIN     try {
#define QL_TEST_SETUP     setup();
#define QL_TEST_TEARDOWN  teardown(); } catch (...) { teardown(); throw; }
#define QL_TEST_END       } catch (...) { throw; }

/* the following displays the elapsed time for the test if
   QL_DISPLAY_TEST_TIME is defined. */
#if defined(QL_DISPLAY_TEST_TIME)
#define QL_TEST_START_TIMING boost::progress_timer t;
#else
#define QL_TEST_START_TIMING
#endif

namespace QuantLib {

    std::string payoffTypeToString(const boost::shared_ptr<Payoff>&);
    std::string exerciseTypeToString(const boost::shared_ptr<Exercise>&);


    boost::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             const boost::shared_ptr<Quote>& forward,
             const DayCounter& dc = Actual365Fixed());

    boost::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             Rate forward,
             const DayCounter& dc = Actual365Fixed());

    boost::shared_ptr<YieldTermStructure>
    flatRate(const boost::shared_ptr<Quote>& forward,
             const DayCounter& dc = Actual365Fixed());

    boost::shared_ptr<YieldTermStructure>
    flatRate(Rate forward,
             const DayCounter& dc = Actual365Fixed());


    boost::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today,
            const boost::shared_ptr<Quote>& volatility,
            const DayCounter& dc = Actual365Fixed());

    boost::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today,
            Volatility volatility,
            const DayCounter& dc = Actual365Fixed());

    boost::shared_ptr<BlackVolTermStructure>
    flatVol(const boost::shared_ptr<Quote>& volatility,
            const DayCounter& dc = Actual365Fixed());

    boost::shared_ptr<BlackVolTermStructure>
    flatVol(Volatility volatility,
            const DayCounter& dc = Actual365Fixed());


    Real relativeError(Real x1, Real x2, Real reference);

    class Flag : public QuantLib::Observer {
      private:
        bool up_;
      public:
        Flag() : up_(false) {}
        void raise() { up_ = true; }
        void lower() { up_ = false; }
        bool isUp() const { return up_; }
        void update() { raise(); }
    };

    template<class Iterator>
    Real norm(const Iterator& begin, const Iterator& end, Real h) {
        // squared values
        std::vector<Real> f2(end-begin);
        std::transform(begin,end,begin,f2.begin(),
                       std::multiplies<Real>());
        // numeric integral of f^2
        Real I = h * (std::accumulate(f2.begin(),f2.end(),0.0)
                      - 0.5*f2.front() - 0.5*f2.back());
        return QL_SQRT(I);
    }

}


#endif
