
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
//#include <iostream>
#include <sstream>

namespace QuantLib {

    // constructors
    InterestRate::InterestRate()
    : r_(Null<Real>()) {}

    InterestRate::InterestRate(Rate r, DayCounter dc,
                               Compounding comp, Frequency freq)
    : r_(r), dc_(dc), comp_(comp) {

        if (comp_==Compounded) {
            QL_REQUIRE(freq!=Once && freq!=NoFrequency,
                       "frequency not allowed for InterestRate");
            freq_ = Real(freq);
        }
    }

    Real InterestRate::compoundFactor(Time t) const {
        QL_REQUIRE(r_, "null InterestRate");
        switch (comp_) {
          case Simple:
            return 1.0 + r_*t;
          case Compounded:
            /*
              std::cout << "accrualFactor Compounded, ";
              std::cout << timeMultiplier_ << ", ";
              std::cout << t << ", ";
              std::cout << r_ << ", ";
              std::cout << QL_POW(1.0+r_, timeMultiplier_*t) << std::endl;
            */
            return QL_POW(1.0+r_/freq_, freq_*t);
          case Continuous:
            return QL_EXP(r_*t);
          default:
            QL_FAIL("unknown compounding convention");
        }
    }

    Rate InterestRate::equivalentRate(Time t, DayCounter dc,
                                      Compounding comp, Frequency freq) const {

        QL_REQUIRE(r_, "null InterestRate");

        Real compounded = compoundFactor(t);
        switch (comp) {
          case Simple:
            return (compounded - 1.0)/t;
          case Compounded:
            return (QL_POW(compounded, 1.0/(Real(freq)*t))-1.0)*Real(freq);
          case Continuous:
            return QL_LOG(compounded)/t;
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
          default:
            QL_FAIL("unknown compounding convention (" +
                    IntegerFormatter::toString(comp) + ")");
        }
    }

}
