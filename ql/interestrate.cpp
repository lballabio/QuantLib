
/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#include <ql/interestrate.hpp>
#include <ql/null.hpp>
#include <ql/basicdataformatters.hpp>
#include <sstream>

namespace QuantLib {

    // constructors

    InterestRate::InterestRate()
    : r_(Null<Real>()) {}

    InterestRate::InterestRate(Rate r, const DayCounter& dc,
                               Compounding comp, Frequency freq)
    : r_(r), dc_(dc), comp_(comp), freqMakesSense_(false) {

        if (comp_==Compounded || comp_==SimpleThenCompounded) {
            freqMakesSense_ = true;
            QL_REQUIRE(freq!=Once && freq!=NoFrequency,
                       "frequency not allowed for InterestRate");
            freq_ = Real(freq);
        }
    }

    Real InterestRate::compoundFactor(Time t) const {

        QL_REQUIRE(t>=0.0, "negative time not allowed");
        QL_REQUIRE(r_ != Null<Rate>(), "null interest rate");
        switch (comp_) {
          case Simple:
            return 1.0 + r_*t;
          case Compounded:
            return QL_POW(1.0+r_/freq_, freq_*t);
          case Continuous:
            return QL_EXP(r_*t);
          case SimpleThenCompounded:
            if (t<=1.0/Real(freq_))
                return 1.0 + r_*t;
            else
                return QL_POW(1.0+r_/freq_, freq_*t);
          default:
            QL_FAIL("unknown compounding convention");
        }
    }

    Rate InterestRate::impliedRate(Real compound, Time t,
                                   Compounding comp, Frequency freq) {

        QL_REQUIRE(compound>0.0, "positive compound factor required");
        QL_REQUIRE(t>0.0, "positive time required");

        switch (comp) {
          case Simple:
            return (compound - 1.0)/t;
          case Compounded:
            return (QL_POW(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
          case Continuous:
            return QL_LOG(compound)/t;
          case SimpleThenCompounded:
            if (t<=1.0/Real(freq))
                return (compound - 1.0)/t;
            else
                return (QL_POW(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
          default:
            QL_FAIL("unknown compounding convention ("+
                IntegerFormatter::toString(comp)+")");
        }
    }

    std::string InterestRateFormatter::toString(InterestRate ir,
                                                Integer precision) {
        static std::ostringstream out;
        out << RateFormatter::toString(ir.rate(),precision);
        out << " ";
        out << ir.dayCounter().name();
        out << " ";
        out << CompoundingRuleFormatter::toString(ir.compounding(),
                                                  ir.frequency());
        return out.str();
    }

    std::string CompoundingRuleFormatter::toString(Compounding comp,
                                                   Frequency freq) {
        switch (comp) {
          case Simple:
            return std::string("simple compounding");
          case Compounded:
            switch (freq) {
              case NoFrequency:
              case Once:
                QL_FAIL(FrequencyFormatter::toString(freq) +
                        " frequency not allowed for interest rate");
              default:
                return std::string(FrequencyFormatter::toString(freq)
                                   + " compounding");
            }
          case Continuous:
            return std::string("continuous compounding");
          case SimpleThenCompounded:
            switch (freq) {
              case NoFrequency:
              case Once:
                QL_FAIL(FrequencyFormatter::toString(freq) +
                        " frequency not allowed for interest rate");
              default:
                return std::string("simple compounding up to " +
                                   IntegerFormatter::toString(Integer(12/freq))
                                   + " months, then "
                                   + FrequencyFormatter::toString(freq) +
                                   " compounding");
            }
          default:
            QL_FAIL("unknown compounding convention (" +
                    IntegerFormatter::toString(comp) + ")");
        }
    }

}
