
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
#include <ql/Utilities/null.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <sstream>
#include <iomanip>

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
                       "frequency not allowed for this interest rate");
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
            return std::pow(1.0+r_/freq_, freq_*t);
          case Continuous:
            return std::exp(r_*t);
          case SimpleThenCompounded:
            if (t<=1.0/Real(freq_))
                return 1.0 + r_*t;
            else
                return std::pow(1.0+r_/freq_, freq_*t);
          default:
            QL_FAIL("unknown compounding convention");
        }
    }

    InterestRate InterestRate::impliedRate(Real compound, Time t,
                                           const DayCounter& resultDC,
                                           Compounding comp, Frequency freq) {

        QL_REQUIRE(compound>0.0, "positive compound factor required");
        QL_REQUIRE(t>0.0, "positive time required");

        Rate r;
        switch (comp) {
          case Simple:
            r = (compound - 1.0)/t;
            break;
          case Compounded:
            r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
            break;
          case Continuous:
            r = std::log(compound)/t;
            break;
          case SimpleThenCompounded:
            if (t<=1.0/Real(freq))
                r = (compound - 1.0)/t;
            else
                r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(comp) << ")");
        }
        return InterestRate(r, resultDC, comp, freq);
    }


    std::ostream& operator<<(std::ostream& out, const InterestRate& ir) {
        if (ir.rate() == Null<Rate>())
            return out << "null interest rate";

        out << io::rate(ir.rate()) << " " << ir.dayCounter().name() << " ";
        switch (ir.compounding()) {
          case Simple:
            out << "simple compounding";
            break;
          case Compounded:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << ir.frequency() <<" compounding";
            }
            break;
          case Continuous:
            out << "continuous compounding";
            break;
          case SimpleThenCompounded:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << "simple compounding up to "
                    << Integer(12/ir.frequency()) << " months, then "
                    << ir.frequency() << " compounding";
            }
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(ir.compounding()) << ")");
        }
        return out;
    }


    #ifndef QL_DISABLE_DEPRECATED
    std::string InterestRateFormatter::toString(InterestRate ir,
                                                Integer precision) {
        static std::ostringstream out;
        out << std::setprecision(precision) << ir;
        return out.str();
    }

    std::string CompoundingRuleFormatter::toString(Compounding comp,
                                                   Frequency freq) {
        std::ostringstream out;
        switch (comp) {
          case Simple:
            out << "simple compounding";
            break;
          case Compounded:
            switch (freq) {
              case NoFrequency:
              case Once:
                QL_FAIL(freq << " frequency not allowed "
                        "for this interest rate");
              default:
                out << freq <<" compounding";
            }
            break;
          case Continuous:
            out << "continuous compounding";
            break;
          case SimpleThenCompounded:
            switch (freq) {
              case NoFrequency:
              case Once:
                QL_FAIL(freq << " frequency not allowed "
                        "for this interest rate");
              default:
                out << "simple compounding up to "
                    << Integer(12/freq) << " months, then "
                    << freq << " compounding";
            }
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(comp) << ")");
        }
        return out.str();
    }
    #endif

}
